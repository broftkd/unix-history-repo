/*
 * ng_btsocket_l2cap_raw.c
 *
 * Copyright (c) 2001-2002 Maksim Yevmenkin <m_evmenkin@yahoo.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: ng_btsocket_l2cap_raw.c,v 1.1 2002/09/04 21:44:00 max Exp $
 * $FreeBSD$
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/domain.h>
#include <sys/errno.h>
#include <sys/filedesc.h>
#include <sys/ioccom.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/mutex.h>
#include <sys/protosw.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <sys/taskqueue.h>
#include <netgraph/ng_message.h>
#include <netgraph/netgraph.h>
#include <bitstring.h>
#include "ng_bluetooth.h"
#include "ng_hci.h"
#include "ng_l2cap.h"
#include "ng_btsocket.h"
#include "ng_btsocket_l2cap.h"

/* MALLOC define */
#ifdef NG_SEPARATE_MALLOC
MALLOC_DEFINE(M_NETGRAPH_BTSOCKET_L2CAP_RAW, "netgraph_btsocks_l2cap_raw",
		"Netgraph Bluetooth raw L2CAP sockets");
#else
#define M_NETGRAPH_BTSOCKET_L2CAP_RAW M_NETGRAPH
#endif /* NG_SEPARATE_MALLOC */

/* Netgraph node methods */
static ng_constructor_t	ng_btsocket_l2cap_raw_node_constructor;
static ng_rcvmsg_t	ng_btsocket_l2cap_raw_node_rcvmsg;
static ng_shutdown_t	ng_btsocket_l2cap_raw_node_shutdown;
static ng_newhook_t	ng_btsocket_l2cap_raw_node_newhook;
static ng_connect_t	ng_btsocket_l2cap_raw_node_connect;
static ng_rcvdata_t	ng_btsocket_l2cap_raw_node_rcvdata;
static ng_disconnect_t	ng_btsocket_l2cap_raw_node_disconnect;

static void		ng_btsocket_l2cap_raw_input     (void *, int);
static void		ng_btsocket_l2cap_raw_rtclean   (void *, int);
static void		ng_btsocket_l2cap_raw_get_token (u_int32_t *);

/* Netgraph type descriptor */
static struct ng_type	typestruct = {
	NG_ABI_VERSION,
	NG_BTSOCKET_L2CAP_RAW_NODE_TYPE,	/* typename */
	NULL,					/* modevent */
	ng_btsocket_l2cap_raw_node_constructor,	/* constructor */
	ng_btsocket_l2cap_raw_node_rcvmsg,	/* control message */
	ng_btsocket_l2cap_raw_node_shutdown,	/* destructor */
	ng_btsocket_l2cap_raw_node_newhook,	/* new hook */
	NULL,					/* find hook */
	ng_btsocket_l2cap_raw_node_connect,	/* connect hook */
	ng_btsocket_l2cap_raw_node_rcvdata,	/* data */
	ng_btsocket_l2cap_raw_node_disconnect,	/* disconnect hook */
	NULL					/* node command list */
};

/* Globals */
extern int					ifqmaxlen;
static u_int32_t				ng_btsocket_l2cap_raw_debug_level;
static u_int32_t				ng_btsocket_l2cap_raw_ioctl_timeout;
static node_p					ng_btsocket_l2cap_raw_node;
static struct ng_bt_itemq			ng_btsocket_l2cap_raw_queue;
static struct mtx				ng_btsocket_l2cap_raw_queue_mtx;
static struct task				ng_btsocket_l2cap_raw_queue_task;
static LIST_HEAD(, ng_btsocket_l2cap_raw_pcb)	ng_btsocket_l2cap_raw_sockets;
static struct mtx				ng_btsocket_l2cap_raw_sockets_mtx;
static u_int32_t				ng_btsocket_l2cap_raw_token;
static struct mtx				ng_btsocket_l2cap_raw_token_mtx;
static LIST_HEAD(, ng_btsocket_l2cap_rtentry)	ng_btsocket_l2cap_raw_rt;
static struct mtx				ng_btsocket_l2cap_raw_rt_mtx;
static struct task				ng_btsocket_l2cap_raw_rt_task;

/* Sysctl tree */
SYSCTL_DECL(_net_bluetooth_l2cap_sockets);
SYSCTL_NODE(_net_bluetooth_l2cap_sockets, OID_AUTO, raw, CTLFLAG_RW,
	0, "Bluetooth raw L2CAP sockets family");
SYSCTL_INT(_net_bluetooth_l2cap_sockets_raw, OID_AUTO, debug_level,
	CTLFLAG_RW,
	&ng_btsocket_l2cap_raw_debug_level, NG_BTSOCKET_WARN_LEVEL,
	"Bluetooth raw L2CAP sockets debug level");
SYSCTL_INT(_net_bluetooth_l2cap_sockets_raw, OID_AUTO, ioctl_timeout,
	CTLFLAG_RW,
	&ng_btsocket_l2cap_raw_ioctl_timeout, 5,
	"Bluetooth raw L2CAP sockets ioctl timeout");
SYSCTL_INT(_net_bluetooth_l2cap_sockets_raw, OID_AUTO, queue_len, 
	CTLFLAG_RD,
	&ng_btsocket_l2cap_raw_queue.len, 0,
	"Bluetooth raw L2CAP sockets input queue length");
SYSCTL_INT(_net_bluetooth_l2cap_sockets_raw, OID_AUTO, queue_maxlen, 
	CTLFLAG_RD,
	&ng_btsocket_l2cap_raw_queue.maxlen, 0,
	"Bluetooth raw L2CAP sockets input queue max. length");
SYSCTL_INT(_net_bluetooth_l2cap_sockets_raw, OID_AUTO, queue_drops, 
	CTLFLAG_RD,
	&ng_btsocket_l2cap_raw_queue.drops, 0,
	"Bluetooth raw L2CAP sockets input queue drops");

/* Debug */
#define NG_BTSOCKET_L2CAP_RAW_INFO \
	if (ng_btsocket_l2cap_raw_debug_level >= NG_BTSOCKET_INFO_LEVEL) \
		printf

#define NG_BTSOCKET_L2CAP_RAW_WARN \
	if (ng_btsocket_l2cap_raw_debug_level >= NG_BTSOCKET_WARN_LEVEL) \
		printf

#define NG_BTSOCKET_L2CAP_RAW_ERR \
	if (ng_btsocket_l2cap_raw_debug_level >= NG_BTSOCKET_ERR_LEVEL) \
		printf

#define NG_BTSOCKET_L2CAP_RAW_ALERT \
	if (ng_btsocket_l2cap_raw_debug_level >= NG_BTSOCKET_ALERT_LEVEL) \
		printf

/*****************************************************************************
 *****************************************************************************
 **                        Netgraph node interface
 *****************************************************************************
 *****************************************************************************/

/*
 * Netgraph node constructor. Do not allow to create node of this type.
 */

static int
ng_btsocket_l2cap_raw_node_constructor(node_p node)
{
	return (EINVAL);
} /* ng_btsocket_l2cap_raw_node_constructor */

/*
 * Do local shutdown processing. Let old node go and create new fresh one.
 */

static int
ng_btsocket_l2cap_raw_node_shutdown(node_p node)
{
	int	error = 0;

	NG_NODE_UNREF(node);

	/* Create new node */
	error = ng_make_node_common(&typestruct, &ng_btsocket_l2cap_raw_node);
	if (error != 0) {
		NG_BTSOCKET_L2CAP_RAW_ALERT(
"%s: Could not create Netgraph node, error=%d\n", __func__, error);

		ng_btsocket_l2cap_raw_node = NULL;

		return (error);
	}

	error = ng_name_node(ng_btsocket_l2cap_raw_node,
				NG_BTSOCKET_L2CAP_RAW_NODE_TYPE);
	if (error != NULL) {
		NG_BTSOCKET_L2CAP_RAW_ALERT(
"%s: Could not name Netgraph node, error=%d\n", __func__, error);

		NG_NODE_UNREF(ng_btsocket_l2cap_raw_node);
		ng_btsocket_l2cap_raw_node = NULL;

		return (error);
	}
		
	return (0);
} /* ng_btsocket_l2cap_raw_node_shutdown */

/*
 * We allow any hook to be connected to the node.
 */

static int
ng_btsocket_l2cap_raw_node_newhook(node_p node, hook_p hook, char const *name)
{
	return (0);
} /* ng_btsocket_l2cap_raw_node_newhook */

/* 
 * Just say "YEP, that's OK by me!"
 */

static int
ng_btsocket_l2cap_raw_node_connect(hook_p hook)
{
	NG_HOOK_SET_PRIVATE(hook, NULL);
	NG_HOOK_REF(hook); /* Keep extra reference to the hook */

	return (0);
} /* ng_btsocket_l2cap_raw_node_connect */

/*
 * Hook disconnection. Schedule route cleanup task
 */

static int
ng_btsocket_l2cap_raw_node_disconnect(hook_p hook)
{
	/*
	 * If hook has private information than we must have this hook in
	 * the routing table and must schedule cleaning for the routing table.
	 * Otherwise hook was connected but we never got "hook_info" message,
	 * so we have never added this hook to the routing table and it save
	 * to just delete it.
	 */

	if (NG_HOOK_PRIVATE(hook) != NULL)
		return (taskqueue_enqueue(taskqueue_swi, 
				&ng_btsocket_l2cap_raw_rt_task));

	NG_HOOK_UNREF(hook); /* Remove extra reference */

	return (0);
} /* ng_btsocket_l2cap_raw_node_disconnect */

/*
 * Process incoming messages 
 */

static int
ng_btsocket_l2cap_raw_node_rcvmsg(node_p node, item_p item, hook_p hook)
{
	struct ng_mesg	*msg = NGI_MSG(item); /* item still has message */
	int		 error = 0;

	if (msg != NULL && msg->header.typecookie == NGM_L2CAP_COOKIE) {
		mtx_lock(&ng_btsocket_l2cap_raw_queue_mtx);
		if (NG_BT_ITEMQ_FULL(&ng_btsocket_l2cap_raw_queue)) {
			NG_BTSOCKET_L2CAP_RAW_ERR(
"%s: Input queue is full\n", __func__);

			NG_BT_ITEMQ_DROP(&ng_btsocket_l2cap_raw_queue);
			NG_FREE_ITEM(item);
			error = ENOBUFS;
		} else {
			if (hook != NULL) {
				NG_HOOK_REF(hook);
				NGI_SET_HOOK(item, hook);
			}

			NG_BT_ITEMQ_ENQUEUE(&ng_btsocket_l2cap_raw_queue, item);
			error = taskqueue_enqueue(taskqueue_swi,
					&ng_btsocket_l2cap_raw_queue_task);
		}
		mtx_unlock(&ng_btsocket_l2cap_raw_queue_mtx);
	} else {
		NG_FREE_ITEM(item);
		error = EINVAL;
	}

	return (error);
} /* ng_btsocket_l2cap_raw_node_rcvmsg */

/*
 * Receive data on a hook
 */

static int
ng_btsocket_l2cap_raw_node_rcvdata(hook_p hook, item_p item)
{
	NG_FREE_ITEM(item);

	return (EINVAL);
} /* ng_btsocket_l2cap_raw_node_rcvdata */

/*****************************************************************************
 *****************************************************************************
 **                              Socket interface
 *****************************************************************************
 *****************************************************************************/

/*
 * L2CAP sockets input routine
 */

static void
ng_btsocket_l2cap_raw_input(void *context, int pending)
{
	item_p		 item = NULL;
	hook_p		 hook = NULL;
	struct ng_mesg  *msg = NULL;

	for (;;) {
		mtx_lock(&ng_btsocket_l2cap_raw_queue_mtx);
		NG_BT_ITEMQ_DEQUEUE(&ng_btsocket_l2cap_raw_queue, item);
		mtx_unlock(&ng_btsocket_l2cap_raw_queue_mtx);

		if (item == NULL)
			break;

		KASSERT((item->el_flags & NGQF_TYPE) == NGQF_MESG,
("%s: invalid item type=%ld\n", __func__, (item->el_flags & NGQF_TYPE)));

		NGI_GET_MSG(item, msg);
		NGI_GET_HOOK(item, hook);
		NG_FREE_ITEM(item);

		switch (msg->header.cmd) {
		case NGM_L2CAP_NODE_HOOK_INFO: {
			ng_btsocket_l2cap_rtentry_t	*rt = NULL;

			if (hook == NULL || NG_HOOK_NOT_VALID(hook) ||
			    msg->header.arglen != sizeof(bdaddr_t))
				break;

			if (bcmp(msg->data, NG_HCI_BDADDR_ANY,
					sizeof(bdaddr_t)) == 0)
				break;

			rt = (ng_btsocket_l2cap_rtentry_t *) 
				NG_HOOK_PRIVATE(hook);
			if (rt == NULL) {
				MALLOC(rt, ng_btsocket_l2cap_rtentry_p, 
					sizeof(*rt),
					M_NETGRAPH_BTSOCKET_L2CAP_RAW,
					M_NOWAIT|M_ZERO);
				if (rt == NULL)
					break;

				mtx_lock(&ng_btsocket_l2cap_raw_rt_mtx);
				LIST_INSERT_HEAD(&ng_btsocket_l2cap_raw_rt, 
					rt, next);
				mtx_unlock(&ng_btsocket_l2cap_raw_rt_mtx);

				NG_HOOK_SET_PRIVATE(hook, rt);
			}
		
			bcopy(msg->data, &rt->src, sizeof(rt->src));
			rt->hook = hook;

			NG_BTSOCKET_L2CAP_RAW_INFO(
"%s: Updating hook \"%s\", src bdaddr=%x:%x:%x:%x:%x:%x\n",
				__func__, NG_HOOK_NAME(hook), 
				rt->src.b[5], rt->src.b[4], rt->src.b[3],
				rt->src.b[2], rt->src.b[1], rt->src.b[0]);
			} break;

		case NGM_L2CAP_NODE_GET_FLAGS:
		case NGM_L2CAP_NODE_GET_DEBUG:
		case NGM_L2CAP_NODE_GET_CON_LIST:
		case NGM_L2CAP_NODE_GET_CHAN_LIST:
		case NGM_L2CAP_L2CA_PING:
		case NGM_L2CAP_L2CA_GET_INFO: {
			ng_btsocket_l2cap_raw_pcb_p	pcb = NULL;

			if (msg->header.token == 0 ||
			    !(msg->header.flags & NGF_RESP))
				break;

			mtx_lock(&ng_btsocket_l2cap_raw_sockets_mtx);

			LIST_FOREACH(pcb, &ng_btsocket_l2cap_raw_sockets, next)
				if (pcb->token == msg->header.token) {
					pcb->msg = msg;
					msg = NULL;
					wakeup(&pcb->msg);
					break;
				}

			mtx_unlock(&ng_btsocket_l2cap_raw_sockets_mtx);
			} break;

		default:
			NG_BTSOCKET_L2CAP_RAW_WARN(
"%s: Unknown message, cmd=%d\n", __func__, msg->header.cmd);
			break;
		}

		if (hook != NULL)
			NG_HOOK_UNREF(hook); /* remove extra reference */

		NG_FREE_MSG(msg); /* Checks for msg != NULL */
	}
} /* ng_btsocket_l2cap_raw_default_msg_input */

/*
 * Route cleanup task. Gets scheduled when hook is disconnected. Here we 
 * will find all sockets that use "invalid" hook and disconnect them.
 */

static void
ng_btsocket_l2cap_raw_rtclean(void *context, int pending)
{
	ng_btsocket_l2cap_raw_pcb_p	pcb = NULL;
	ng_btsocket_l2cap_rtentry_p	rt = NULL;

	mtx_lock(&ng_btsocket_l2cap_raw_rt_mtx);
	mtx_lock(&ng_btsocket_l2cap_raw_sockets_mtx);

	/*
	 * First disconnect all sockets that use "invalid" hook
	 */

	LIST_FOREACH(pcb, &ng_btsocket_l2cap_raw_sockets, next)
		if (pcb->rt != NULL &&
		    pcb->rt->hook != NULL && NG_HOOK_NOT_VALID(pcb->rt->hook)) {
			if (pcb->so != NULL &&
			    pcb->so->so_state & SS_ISCONNECTED)
				soisdisconnected(pcb->so);

			pcb->rt = NULL;
		}

	/*
	 * Now cleanup routing table
	 */

	rt = LIST_FIRST(&ng_btsocket_l2cap_raw_rt);
	while (rt != NULL) {
		ng_btsocket_l2cap_rtentry_p	rt_next = LIST_NEXT(rt, next);

		if (rt->hook != NULL && NG_HOOK_NOT_VALID(rt->hook)) {
			LIST_REMOVE(rt, next);

			NG_HOOK_SET_PRIVATE(rt->hook, NULL);
			NG_HOOK_UNREF(rt->hook); /* Remove extra reference */

			bzero(rt, sizeof(*rt));
			FREE(rt, M_NETGRAPH_BTSOCKET_L2CAP_RAW);
		}

		rt = rt_next;
	}

	mtx_unlock(&ng_btsocket_l2cap_raw_sockets_mtx);
	mtx_unlock(&ng_btsocket_l2cap_raw_rt_mtx);
} /* ng_btsocket_l2cap_raw_rtclean */

/*
 * Initialize everything
 */

void
ng_btsocket_l2cap_raw_init(void)
{
	int	error = 0;

	ng_btsocket_l2cap_raw_node = NULL;
	ng_btsocket_l2cap_raw_debug_level = NG_BTSOCKET_WARN_LEVEL;
	ng_btsocket_l2cap_raw_ioctl_timeout = 5;

	/* Register Netgraph node type */
	error = ng_newtype(&typestruct);
	if (error != 0) {
		NG_BTSOCKET_L2CAP_RAW_ALERT(
"%s: Could not register Netgraph node type, error=%d\n", __func__, error);

                return;
	}

	/* Create Netgrapg node */
	error = ng_make_node_common(&typestruct, &ng_btsocket_l2cap_raw_node);
	if (error != 0) {
		NG_BTSOCKET_L2CAP_RAW_ALERT(
"%s: Could not create Netgraph node, error=%d\n", __func__, error);

		ng_btsocket_l2cap_raw_node = NULL;

		return;
	}

	error = ng_name_node(ng_btsocket_l2cap_raw_node,
				NG_BTSOCKET_L2CAP_RAW_NODE_TYPE);
	if (error != 0) {
		NG_BTSOCKET_L2CAP_RAW_ALERT(
"%s: Could not name Netgraph node, error=%d\n", __func__, error);

		NG_NODE_UNREF(ng_btsocket_l2cap_raw_node);
		ng_btsocket_l2cap_raw_node = NULL;

		return;
	}

	/* Create input queue */
	NG_BT_ITEMQ_INIT(&ng_btsocket_l2cap_raw_queue, ifqmaxlen);
	mtx_init(&ng_btsocket_l2cap_raw_queue_mtx,
		"btsocks_l2cap_queue_mtx", NULL, MTX_DEF);
	TASK_INIT(&ng_btsocket_l2cap_raw_queue_task, 0,
		ng_btsocket_l2cap_raw_input, NULL);

	/* Create list of sockets */
	LIST_INIT(&ng_btsocket_l2cap_raw_sockets);
	mtx_init(&ng_btsocket_l2cap_raw_sockets_mtx,
		"btsocks_l2cap_sockets_mtx", NULL, MTX_DEF);

	/* Tokens */
	ng_btsocket_l2cap_raw_token = 0;
	mtx_init(&ng_btsocket_l2cap_raw_token_mtx,
		"btsocks_l2cap_token_mtx", NULL, MTX_DEF);

	/* Routing table */
	LIST_INIT(&ng_btsocket_l2cap_raw_rt);
	mtx_init(&ng_btsocket_l2cap_raw_rt_mtx,
		"btsocks_l2cap_rt_mtx", NULL, MTX_DEF);
	TASK_INIT(&ng_btsocket_l2cap_raw_rt_task, 0,
		ng_btsocket_l2cap_raw_rtclean, NULL);
} /* ng_btsocket_l2cap_raw_init */

/*
 * Abort connection on socket
 */

int
ng_btsocket_l2cap_raw_abort(struct socket *so)
{
	return (ng_btsocket_l2cap_raw_detach(so));
} /* ng_btsocket_l2cap_raw_abort */

/*
 * Create and attach new socket
 */

int
ng_btsocket_l2cap_raw_attach(struct socket *so, int proto, struct thread *td)
{
	ng_btsocket_l2cap_raw_pcb_p	pcb = so2l2cap_raw_pcb(so);
	int				error;

	if (pcb != NULL)
		return (EISCONN);

	if (ng_btsocket_l2cap_raw_node == NULL) 
		return (EPROTONOSUPPORT);
	if (so->so_type != SOCK_RAW)
		return (ESOCKTNOSUPPORT);
	if ((error = suser(td)) != 0)
		return (error);

	/* Reserve send and receive space if it is not reserved yet */
	error = soreserve(so, NG_BTSOCKET_L2CAP_RAW_SENDSPACE,
			NG_BTSOCKET_L2CAP_RAW_RECVSPACE);
	if (error != 0)
		return (error);

	/* Allocate the PCB */
        MALLOC(pcb, ng_btsocket_l2cap_raw_pcb_p, sizeof(*pcb),
		M_NETGRAPH_BTSOCKET_L2CAP_RAW, M_ZERO);
        if (pcb == NULL)
                return (ENOMEM);

	/* Link the PCB and the socket */
	so->so_pcb = (caddr_t) pcb;
	pcb->so = so;

        /* Add the PCB to the list */
	mtx_lock(&ng_btsocket_l2cap_raw_sockets_mtx);
	LIST_INSERT_HEAD(&ng_btsocket_l2cap_raw_sockets, pcb, next);
	mtx_unlock(&ng_btsocket_l2cap_raw_sockets_mtx);

        return (0);
} /* ng_btsocket_l2cap_raw_attach */

/*
 * Bind socket
 */

int
ng_btsocket_l2cap_raw_bind(struct socket *so, struct sockaddr *nam, 
		struct thread *td)
{
	ng_btsocket_l2cap_raw_pcb_t	*pcb = so2l2cap_raw_pcb(so);
	struct sockaddr_l2cap		*sa = (struct sockaddr_l2cap *) nam;

	if (pcb == NULL)
		return (EINVAL);
	if (ng_btsocket_l2cap_raw_node == NULL) 
		return (EINVAL);

	if (sa == NULL)
		return (EINVAL);
	if (sa->l2cap_family != AF_BLUETOOTH)
		return (EAFNOSUPPORT);
	if (sa->l2cap_len != sizeof(*sa))
		return (EINVAL);
	if (bcmp(&sa->l2cap_bdaddr, NG_HCI_BDADDR_ANY, sizeof(bdaddr_t)) == 0)
		return (EINVAL);

	bcopy(&sa->l2cap_bdaddr, &pcb->src, sizeof(pcb->src));

	return (0);
} /* ng_btsocket_l2cap_raw_bind */

/*
 * Connect socket
 */

int
ng_btsocket_l2cap_raw_connect(struct socket *so, struct sockaddr *nam, 
		struct thread *td)
{
	ng_btsocket_l2cap_raw_pcb_t	*pcb = so2l2cap_raw_pcb(so);
	struct sockaddr_l2cap		*sa = (struct sockaddr_l2cap *) nam;
	ng_btsocket_l2cap_rtentry_t	*rt = NULL;

	if (pcb == NULL)
		return (EINVAL);
	if (ng_btsocket_l2cap_raw_node == NULL) 
		return (EINVAL);

	if (sa == NULL)
		return (EINVAL);
	if (sa->l2cap_family != AF_BLUETOOTH)
		return (EAFNOSUPPORT);
	if (sa->l2cap_len != sizeof(*sa))
		return (EINVAL);
	if (bcmp(&sa->l2cap_bdaddr, NG_HCI_BDADDR_ANY, sizeof(bdaddr_t)) == 0)
		return (EINVAL);
	if (bcmp(&sa->l2cap_bdaddr, &pcb->src, sizeof(pcb->src)) != 0)
		return (EADDRNOTAVAIL);

	/*
	 * Find hook with specified source address
	 */

	pcb->rt = NULL;

	mtx_lock(&ng_btsocket_l2cap_raw_rt_mtx);

	LIST_FOREACH(rt, &ng_btsocket_l2cap_raw_rt, next) {
		if (rt->hook == NULL || NG_HOOK_NOT_VALID(rt->hook))
			continue;

		if (bcmp(&pcb->src, &rt->src, sizeof(rt->src)) == 0)
			break;
	}

	if (rt != NULL) {
		pcb->rt = rt;
		soisconnected(so);
	}

	mtx_unlock(&ng_btsocket_l2cap_raw_rt_mtx);

	return  ((pcb->rt != NULL)? 0 : ENETDOWN);
} /* ng_btsocket_l2cap_raw_connect */

/*
 * Find hook that matches source address
 */

static ng_btsocket_l2cap_rtentry_p
ng_btsocket_l2cap_raw_find_src_route(bdaddr_p src)
{
	ng_btsocket_l2cap_rtentry_p	rt = NULL;

	if (bcmp(src, NG_HCI_BDADDR_ANY, sizeof(*src)) != 0) {
		mtx_lock(&ng_btsocket_l2cap_raw_rt_mtx);

		LIST_FOREACH(rt, &ng_btsocket_l2cap_raw_rt, next)
			if (rt->hook != NULL && NG_HOOK_IS_VALID(rt->hook) &&
			    bcmp(src, &rt->src, sizeof(*src)) == 0)
				break;

		mtx_unlock(&ng_btsocket_l2cap_raw_rt_mtx);
	}

	return (rt);
} /* ng_btsocket_l2cap_raw_find_src_route */

/*
 * Find first hook does does not match destination address
 */

static ng_btsocket_l2cap_rtentry_p
ng_btsocket_l2cap_raw_find_dst_route(bdaddr_p dst)
{
	ng_btsocket_l2cap_rtentry_p	rt = NULL;

	if (bcmp(dst, NG_HCI_BDADDR_ANY, sizeof(*dst)) != 0) {
		mtx_lock(&ng_btsocket_l2cap_raw_rt_mtx);

		LIST_FOREACH(rt, &ng_btsocket_l2cap_raw_rt, next)
			if (rt->hook != NULL && NG_HOOK_IS_VALID(rt->hook) &&
			    bcmp(dst, &rt->src, sizeof(*dst)) != 0)
				break;

		mtx_unlock(&ng_btsocket_l2cap_raw_rt_mtx);
	}

	return (rt);
} /* ng_btsocket_l2cap_raw_find_dst_route */

/*
 * Process ioctl's calls on socket
 */

int
ng_btsocket_l2cap_raw_control(struct socket *so, u_long cmd, caddr_t data,
		struct ifnet *ifp, struct thread *td)
{
	ng_btsocket_l2cap_raw_pcb_p	 pcb = so2l2cap_raw_pcb(so);
	bdaddr_t			*src = (bdaddr_t *) data;
	ng_btsocket_l2cap_rtentry_p	 rt = pcb->rt;
	struct ng_mesg			*msg = NULL;
	int				 error = 0;

	if (pcb == NULL)
		return (EINVAL);
	if (ng_btsocket_l2cap_raw_node == NULL)
		return (EINVAL);

	if (rt == NULL || rt->hook == NULL || NG_HOOK_NOT_VALID(rt->hook)) {
		if (cmd == SIOC_L2CAP_L2CA_PING ||
		    cmd == SIOC_L2CAP_L2CA_GET_INFO)
			rt = ng_btsocket_l2cap_raw_find_dst_route(src + 1);
		else
			rt = ng_btsocket_l2cap_raw_find_src_route(src);

		if (rt == NULL) 
			return (EHOSTUNREACH);
	}

	bcopy(&rt->src, src, sizeof(*src));

	switch (cmd) {
	case SIOC_L2CAP_NODE_GET_FLAGS: {
		struct ng_btsocket_l2cap_raw_node_flags	*p =
			(struct ng_btsocket_l2cap_raw_node_flags *) data;

		ng_btsocket_l2cap_raw_get_token(&pcb->token);
		pcb->msg = NULL;

		NG_MKMESSAGE(msg, NGM_L2CAP_COOKIE, NGM_L2CAP_NODE_GET_FLAGS,
			0, 0);
		if (msg == NULL) {
			pcb->token = 0;
			error = ENOMEM;
			break;
		}
		msg->header.token = pcb->token;

		NG_SEND_MSG_HOOK(error, ng_btsocket_l2cap_raw_node, msg,
			rt->hook, NULL);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		error = tsleep(&pcb->msg, PZERO|PCATCH, "l2ctl",
				ng_btsocket_l2cap_raw_ioctl_timeout * hz);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		if (pcb->msg != NULL &&
		    pcb->msg->header.cmd == NGM_L2CAP_NODE_GET_FLAGS)
			p->flags = *((ng_l2cap_node_flags_ep *)
							(pcb->msg->data));
		else
			error = EINVAL;

		NG_FREE_MSG(pcb->msg); /* checks for != NULL */
		pcb->token = 0;
		} break;

	case SIOC_L2CAP_NODE_GET_DEBUG: {
		struct ng_btsocket_l2cap_raw_node_debug	*p =
			(struct ng_btsocket_l2cap_raw_node_debug *) data;

		ng_btsocket_l2cap_raw_get_token(&pcb->token);
		pcb->msg = NULL;

		NG_MKMESSAGE(msg, NGM_L2CAP_COOKIE, NGM_L2CAP_NODE_GET_DEBUG,
			0, 0);
		if (msg == NULL) {
			pcb->token = 0;
			error = ENOMEM;
			break;
		}
		msg->header.token = pcb->token;

		NG_SEND_MSG_HOOK(error, ng_btsocket_l2cap_raw_node, msg,
			rt->hook, NULL);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		error = tsleep(&pcb->msg, PZERO|PCATCH, "l2ctl",
				ng_btsocket_l2cap_raw_ioctl_timeout * hz);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		if (pcb->msg != NULL &&
		    pcb->msg->header.cmd == NGM_L2CAP_NODE_GET_DEBUG)
			p->debug = *((ng_l2cap_node_debug_ep *)
							(pcb->msg->data));
		else
			error = EINVAL;

		NG_FREE_MSG(pcb->msg); /* checks for != NULL */
		pcb->token = 0;
		} break;

	case SIOC_L2CAP_NODE_SET_DEBUG: {
		struct ng_btsocket_l2cap_raw_node_debug	*p = 
			(struct ng_btsocket_l2cap_raw_node_debug *) data;

		NG_MKMESSAGE(msg, NGM_L2CAP_COOKIE, NGM_L2CAP_NODE_SET_DEBUG,
			sizeof(ng_l2cap_node_debug_ep), 0);
		if (msg == NULL) {
			error = ENOMEM;
			break;
		}

		*((ng_l2cap_node_debug_ep *)(msg->data)) = p->debug;
		NG_SEND_MSG_HOOK(error, ng_btsocket_l2cap_raw_node,
			msg, rt->hook, NULL);
		} break;

	case SIOC_L2CAP_NODE_GET_CON_LIST: {
		struct ng_btsocket_l2cap_raw_con_list	*p =
			(struct ng_btsocket_l2cap_raw_con_list *) data;
		ng_l2cap_node_con_list_ep		*p1 = NULL;
                ng_l2cap_node_con_ep			*p2 = NULL;
 
		if (p->num_connections == 0 ||
		    p->num_connections > NG_L2CAP_MAX_CON_NUM ||
		    p->connections == NULL) {
			error = EINVAL;
			break;
		}
 
		ng_btsocket_l2cap_raw_get_token(&pcb->token);
		pcb->msg = NULL;

		NG_MKMESSAGE(msg, NGM_L2CAP_COOKIE, NGM_L2CAP_NODE_GET_CON_LIST,
			0, 0);
		if (msg == NULL) {
			pcb->token = 0;
			error = ENOMEM;
			break;
		}
		msg->header.token = pcb->token;

		NG_SEND_MSG_HOOK(error, ng_btsocket_l2cap_raw_node, msg,
			rt->hook, NULL);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		error = tsleep(&pcb->msg, PZERO|PCATCH, "l2ctl",
				ng_btsocket_l2cap_raw_ioctl_timeout * hz);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		if (pcb->msg != NULL &&
		    pcb->msg->header.cmd == NGM_L2CAP_NODE_GET_CON_LIST) {
			/* Return data back to user space */
			p1 = (ng_l2cap_node_con_list_ep *)(pcb->msg->data);
			p2 = (ng_l2cap_node_con_ep *)(p1 + 1);

			p->num_connections = min(p->num_connections,
						p1->num_connections);
			if (p->num_connections > 0)
				error = copyout((caddr_t) p2, 
					(caddr_t) p->connections,
					p->num_connections * sizeof(*p2));
		} else
			error = EINVAL;

		NG_FREE_MSG(pcb->msg); /* checks for != NULL */
		pcb->token = 0;
		} break;

	case SIOC_L2CAP_NODE_GET_CHAN_LIST: {
		struct ng_btsocket_l2cap_raw_chan_list	*p =
			(struct ng_btsocket_l2cap_raw_chan_list *) data;
		ng_l2cap_node_chan_list_ep		*p1 = NULL;
                ng_l2cap_node_chan_ep			*p2 = NULL;
 
		if (p->num_channels == 0 ||
		    p->num_channels > NG_L2CAP_MAX_CHAN_NUM ||
		    p->channels == NULL) {
			error = EINVAL;
			break;
		}
 
		ng_btsocket_l2cap_raw_get_token(&pcb->token);
		pcb->msg = NULL;

		NG_MKMESSAGE(msg, NGM_L2CAP_COOKIE,
			NGM_L2CAP_NODE_GET_CHAN_LIST, 0, 0);
		if (msg == NULL) {
			pcb->token = 0;
			error = ENOMEM;
			break;
		}
		msg->header.token = pcb->token;

		NG_SEND_MSG_HOOK(error, ng_btsocket_l2cap_raw_node, msg,
			rt->hook, NULL);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		error = tsleep(&pcb->msg, PZERO|PCATCH, "l2ctl",
				ng_btsocket_l2cap_raw_ioctl_timeout * hz);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		if (pcb->msg != NULL &&
		    pcb->msg->header.cmd == NGM_L2CAP_NODE_GET_CHAN_LIST) {
			/* Return data back to user space */
			p1 = (ng_l2cap_node_chan_list_ep *)(pcb->msg->data);
			p2 = (ng_l2cap_node_chan_ep *)(p1 + 1);

			p->num_channels = min(p->num_channels, 
						p1->num_channels);
			if (p->num_channels > 0)
				error = copyout((caddr_t) p2, 
						(caddr_t) p->channels,
						p->num_channels * sizeof(*p2));
		} else
			error = EINVAL;

		NG_FREE_MSG(pcb->msg); /* checks for != NULL */
		pcb->token = 0;
		} break;

	case SIOC_L2CAP_L2CA_PING: {
		struct ng_btsocket_l2cap_raw_ping	*p = 
			(struct ng_btsocket_l2cap_raw_ping *) data;
		ng_l2cap_l2ca_ping_ip			*ip = NULL;
		ng_l2cap_l2ca_ping_op			*op = NULL;

		if ((p->echo_size != 0 && p->echo_data == NULL) ||
		     p->echo_size > NG_L2CAP_MAX_ECHO_SIZE) {
			error = EINVAL;
			break;
		}

		/* Loop back local ping */
		if (bcmp(&p->echo_dst, &rt->src, sizeof(rt->src)) == 0) {
			p->result = 0;
			break;
		}

		ng_btsocket_l2cap_raw_get_token(&pcb->token);
		pcb->msg = NULL;

		NG_MKMESSAGE(msg, NGM_L2CAP_COOKIE,
			NGM_L2CAP_L2CA_PING, sizeof(*ip) + p->echo_size,
			0);
		if (msg == NULL) {
			pcb->token = 0;
			error = ENOMEM;
			break;
		}
		msg->header.token = pcb->token;

		ip = (ng_l2cap_l2ca_ping_ip *)(msg->data);
		bcopy(&p->echo_dst, &ip->bdaddr, sizeof(ip->bdaddr));
		ip->echo_size = p->echo_size;

		if (ip->echo_size > 0) {
			error = copyin(p->echo_data, ip + 1, p->echo_size);
			if (error != 0) {
				NG_FREE_MSG(msg);
				pcb->token = 0;
				break;
			}
		}

		NG_SEND_MSG_HOOK(error, ng_btsocket_l2cap_raw_node, msg,
			rt->hook, NULL);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		error = tsleep(&pcb->msg, PZERO|PCATCH, "l2ctl",
				bluetooth_l2cap_rtx_timeout());
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		if (pcb->msg != NULL &&
		    pcb->msg->header.cmd == NGM_L2CAP_L2CA_PING) {
			/* Return data back to the user space */
			op = (ng_l2cap_l2ca_ping_op *)(pcb->msg->data);
			p->result = op->result;
			p->echo_size = min(p->echo_size, op->echo_size);

			if (p->echo_size > 0)
				error = copyout(op + 1, p->echo_data, 
						p->echo_size);
		} else
			error = EINVAL;

		NG_FREE_MSG(pcb->msg); /* checks for != NULL */
		pcb->token = 0;
		} break;

	case SIOC_L2CAP_L2CA_GET_INFO: {
		struct ng_btsocket_l2cap_raw_get_info	*p = 
			(struct ng_btsocket_l2cap_raw_get_info *) data;
		ng_l2cap_l2ca_get_info_ip		*ip = NULL;
		ng_l2cap_l2ca_get_info_op		*op = NULL;

		if ((p->info_size != 0 && p->info_data == NULL) ||
		    bcmp(&p->info_dst, &rt->src, sizeof(rt->src)) == 0) {
			error = EINVAL;
			break;
		}

		ng_btsocket_l2cap_raw_get_token(&pcb->token);
		pcb->msg = NULL;

		NG_MKMESSAGE(msg, NGM_L2CAP_COOKIE,
			NGM_L2CAP_L2CA_GET_INFO, sizeof(*ip) + p->info_size,
			0);
		if (msg == NULL) {
			pcb->token = 0;
			error = ENOMEM;
			break;
		}
		msg->header.token = pcb->token;

		ip = (ng_l2cap_l2ca_get_info_ip *)(msg->data);
		bcopy(&p->info_dst, &ip->bdaddr, sizeof(ip->bdaddr));
		ip->info_type = p->info_type;

		NG_SEND_MSG_HOOK(error, ng_btsocket_l2cap_raw_node, msg,
			rt->hook, NULL);
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		error = tsleep(&pcb->msg, PZERO|PCATCH, "l2ctl",
				bluetooth_l2cap_rtx_timeout());
		if (error != 0) {
			pcb->token = 0;
			break;
		}

		if (pcb->msg != NULL &&
		    pcb->msg->header.cmd == NGM_L2CAP_L2CA_GET_INFO) {
			/* Return data back to the user space */
			op = (ng_l2cap_l2ca_get_info_op *)(pcb->msg->data);
			p->result = op->result;
			p->info_size = min(p->info_size, op->info_size);

			if (p->info_size > 0)
				error = copyout(op + 1, p->info_data, 
						p->info_size);
		} else
			error = EINVAL;

		NG_FREE_MSG(pcb->msg); /* checks for != NULL */
		pcb->token = 0;
		} break;

	default:
		error = EINVAL;
		break;
	}

	return (error);
} /* ng_btsocket_l2cap_raw_control */

/*
 * Detach and destroy socket
 */

int
ng_btsocket_l2cap_raw_detach(struct socket *so)
{
	ng_btsocket_l2cap_raw_pcb_p	pcb = so2l2cap_raw_pcb(so);

	if (pcb == NULL)
		return (EINVAL);
	if (ng_btsocket_l2cap_raw_node == NULL) 
		return (EINVAL);

	so->so_pcb = NULL;
	sotryfree(so);

	mtx_lock(&ng_btsocket_l2cap_raw_sockets_mtx);
	LIST_REMOVE(pcb, next);
	mtx_unlock(&ng_btsocket_l2cap_raw_sockets_mtx);

	bzero(pcb, sizeof(*pcb));
	FREE(pcb, M_NETGRAPH_BTSOCKET_L2CAP_RAW);

	return (0);
} /* ng_btsocket_l2cap_raw_detach */

/*
 * Disconnect socket
 */

int
ng_btsocket_l2cap_raw_disconnect(struct socket *so)
{
	ng_btsocket_l2cap_raw_pcb_p	pcb = so2l2cap_raw_pcb(so);

	if (pcb == NULL)
		return (EINVAL);
	if (ng_btsocket_l2cap_raw_node == NULL) 
		return (EINVAL);

	pcb->rt = NULL;
	soisdisconnected(so);

	return (0);
} /* ng_btsocket_l2cap_raw_disconnect */

/*
 * Get peer address
 */

int
ng_btsocket_l2cap_raw_peeraddr(struct socket *so, struct sockaddr **nam)
{
	return (EOPNOTSUPP);
} /* ng_btsocket_l2cap_raw_peeraddr */

/*
 * Send data to socket
 */

int
ng_btsocket_l2cap_raw_send(struct socket *so, int flags, struct mbuf *m,
		struct sockaddr *nam, struct mbuf *control, struct thread *td)
{
	NG_FREE_M(m); /* Checks for m != NULL */
	NG_FREE_M(control);

	return (EOPNOTSUPP);
} /* ng_btsocket_l2cap_raw_send */

/*
 * Get socket address
 */

int
ng_btsocket_l2cap_raw_sockaddr(struct socket *so, struct sockaddr **nam)
{
	ng_btsocket_l2cap_raw_pcb_p	pcb = so2l2cap_raw_pcb(so);
	struct sockaddr_l2cap		sa;

	if (pcb == NULL)
		return (EINVAL);
	if (ng_btsocket_l2cap_raw_node == NULL) 
		return (EINVAL);

	bcopy(&pcb->src, &sa.l2cap_bdaddr, sizeof(sa.l2cap_bdaddr));
	sa.l2cap_psm = 0;
	sa.l2cap_len = sizeof(sa);
	sa.l2cap_family = AF_BLUETOOTH;

	*nam = dup_sockaddr((struct sockaddr *) &sa, 0);

	return ((*nam == NULL)? ENOMEM : 0);
} /* ng_btsocket_l2cap_raw_sockaddr */

/*
 * Get next token
 */

static void
ng_btsocket_l2cap_raw_get_token(u_int32_t *token)
{
	mtx_lock(&ng_btsocket_l2cap_raw_token_mtx);
  
	if (++ ng_btsocket_l2cap_raw_token == 0)
		ng_btsocket_l2cap_raw_token = 1;
 
	*token = ng_btsocket_l2cap_raw_token;
 
	mtx_unlock(&ng_btsocket_l2cap_raw_token_mtx);
} /* ng_btsocket_l2cap_raw_get_token */

