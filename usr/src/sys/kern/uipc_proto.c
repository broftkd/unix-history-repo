/*	uipc_proto.c	4.26	82/10/17	*/

#include "../h/param.h"
#include "../h/socket.h"
#include "../h/protosw.h"
#include "../h/mbuf.h"
#include <time.h>
#include "../h/kernel.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"

/*
 * Protocol configuration table and routines to search it.
 *
 * SHOULD INCLUDE A HEADER FILE GIVING DESIRED PROTOCOLS
 */

/*
 * Local protocol handler.
 */
int	piusrreq();

/*
 * TCP/IP protocol family: IP, ICMP, UDP, TCP.
 */
int	ip_output();
int	ip_init(),ip_slowtimo(),ip_drain();
int	icmp_input();
int	udp_input(),udp_ctlinput();
int	udp_usrreq();
int	udp_init();
int	tcp_input(),tcp_ctlinput();
int	tcp_usrreq();
int	tcp_init(),tcp_fasttimo(),tcp_slowtimo(),tcp_drain();
int	rip_input(),rip_output();

/*
 * IMP protocol family: raw interface.
 * Using the raw interface entry to get the timer routine
 * in is a kludge.
 */
#include "imp.h"
#if NIMP > 0
int	rimp_output(), hostslowtimo();
#endif

/*
 * PUP-I protocol family: raw interface
 */
#include "pup.h"
#if NPUP > 0
int	rpup_output();
#endif

/*
 * Sundries.
*/
int	raw_init(),raw_usrreq(),raw_input(),raw_ctlinput();

struct protosw protosw[] = {
{ SOCK_STREAM,	PF_UNIX,	0,		PR_CONNREQUIRED,
  0,		0,		0,		0,
  piusrreq,
  0,		0,		0,		0,
},
{ SOCK_DGRAM,	PF_UNIX,	0,		PR_ATOMIC|PR_ADDR,
  0,		0,		0,		0,
  piusrreq,
  0,		0,		0,		0,
},
{ SOCK_RDM,	PF_UNIX,	0,		PR_ATOMIC|PR_ADDR,
  0,		0,		0,		0,
  piusrreq,
  0,		0,		0,		0,
},
{ SOCK_RAW,	PF_UNIX,	0,		PR_ATOMIC|PR_ADDR,
  0,		0,		0,		0,
  piusrreq,
  0,		0,		0,		0,
},
{ 0,		0,		0,		0,
  0,		ip_output,	0,		0,
  0,
  ip_init,	0,		ip_slowtimo,	ip_drain,
},
{ 0,		PF_INET,	IPPROTO_ICMP,	0,
  icmp_input,	0,		0,		0,
  0,
  0,		0,		0,		0,
},
{ SOCK_DGRAM,	PF_INET,	IPPROTO_UDP,	PR_ATOMIC|PR_ADDR,
  udp_input,	0,		udp_ctlinput,	0,
  udp_usrreq,
  udp_init,	0,		0,		0,
},
{ SOCK_STREAM,	PF_INET,	IPPROTO_TCP,	PR_CONNREQUIRED|PR_WANTRCVD,
  tcp_input,	0,		tcp_ctlinput,	0,
  tcp_usrreq,
  tcp_init,	tcp_fasttimo,	tcp_slowtimo,	tcp_drain,
},
{ 0,		0,		0,		0,
  raw_input,	0,		raw_ctlinput,	0,
  raw_usrreq,
  raw_init,	0,		0,		0,
},
{ SOCK_RAW,	PF_INET,	IPPROTO_RAW,	PR_ATOMIC|PR_ADDR,
  rip_input,	rip_output,	0,	0,
  raw_usrreq,
  0,		0,		0,		0,
}
#if NIMP > 0
,
{ SOCK_RAW,	PF_IMPLINK,	0,		PR_ATOMIC|PR_ADDR,
  0,		rimp_output,	0,		0,
  raw_usrreq,
  0,		0,		hostslowtimo,	0,
}
#endif
#if NPUP > 0
,
{ SOCK_RAW,	PF_PUP,		0,		PR_ATOMIC|PR_ADDR,
  0,		rpup_output,	0,		0,
  raw_usrreq,
  0,		0,		0,		0,
}
#endif
};

#define	NPROTOSW	(sizeof(protosw) / sizeof(protosw[0]))

struct	protosw *protoswLAST = &protosw[NPROTOSW-1];

/*
 * Operations on protocol table and protocol families.
 */

/*
 * Initialize all protocols.
 */
pfinit()
{
	register struct protosw *pr;

	for (pr = protoswLAST; pr >= protosw; pr--)
		if (pr->pr_init)
			(*pr->pr_init)();
	pffasttimo();
	pfslowtimo();
}

/*
 * Find a standard protocol in a protocol family
 * of a specific type.
 */
struct protosw *
pffindtype(family, type)
	int family, type;
{
	register struct protosw *pr;

	if (family == 0)
		return (0);
	for (pr = protosw; pr <= protoswLAST; pr++)
		if (pr->pr_family == family && pr->pr_type == type)
			return (pr);
	return (0);
}

/*
 * Find a specified protocol in a specified protocol family.
 */
struct protosw *
pffindproto(family, protocol)
	int family, protocol;
{
	register struct protosw *pr;

	if (family == 0)
		return (0);
	for (pr = protosw; pr <= protoswLAST; pr++)
		if (pr->pr_family == family && pr->pr_protocol == protocol)
			return (pr);
	return (0);
}

pfctlinput(cmd, arg)
	int cmd;
	caddr_t arg;
{
	register struct protosw *pr;

	for (pr = protosw; pr <= protoswLAST; pr++)
		if (pr->pr_ctlinput)
			(*pr->pr_ctlinput)(cmd, arg);
}

/*
 * Slow timeout on all protocols.
 */
pfslowtimo()
{
	register struct protosw *pr;

	for (pr = protoswLAST; pr >= protosw; pr--)
		if (pr->pr_slowtimo)
			(*pr->pr_slowtimo)();
	timeout(pfslowtimo, (caddr_t)0, hz / PR_SLOWHZ);
}

pffasttimo()
{
	register struct protosw *pr;

	for (pr = protoswLAST; pr >= protosw; pr--)
		if (pr->pr_fasttimo)
			(*pr->pr_fasttimo)();
	timeout(pffasttimo, (caddr_t)0, hz / PR_FASTHZ);
}
