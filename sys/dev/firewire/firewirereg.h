/*
 * Copyright (c) 2003 Hidetoshi Shimokawa
 * Copyright (c) 1998-2002 Katsushi Kobayashi and Hidetoshi Shimokawa
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the acknowledgement as bellow:
 *
 *    This product includes software developed by K. Kobayashi and H. Shimokawa
 *
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * $FreeBSD$
 *
 */

#if __FreeBSD_version >= 500000
typedef	struct thread fw_proc;
#include <sys/selinfo.h>
#else
typedef	struct proc fw_proc;
#include <sys/select.h>
#endif

#include <sys/uio.h>

#define	splfw splimp

struct fw_device{
	u_int16_t dst;
	struct fw_eui64 eui;
	u_int8_t speed;
	u_int8_t maxrec;
	u_int8_t nport;
	u_int8_t power;
#define CSRROMOFF 0x400
#define CSRROMSIZE 0x400
	int rommax;	/* offset from 0xffff f000 0000 */
	u_int32_t csrrom[CSRROMSIZE/4];
	int rcnt;
	struct firewire_comm *fc;
	u_int32_t status;
#define FWDEVINIT	1
#define FWDEVATTACHED	2
#define FWDEVINVAL	3
	STAILQ_ENTRY(fw_device) link;
};

struct firewire_softc {
#if __FreeBSD_version >= 500000
	dev_t dev;
#else
	dev_t dev[FWMAXNDMA+1];
#endif
	struct firewire_comm *fc;
};

#define FW_MAX_DMACH 0x20
#define FW_MAX_DEVCH FW_MAX_DMACH
#define FW_XFERTIMEOUT 1

struct firewire_dev_comm {
	device_t dev;
	struct firewire_comm *fc;
	void (*post_busreset) __P((void *));
	void (*post_explore) __P((void *));
};

struct tcode_info {
	u_char hdr_len;	/* IEEE1394 header length */
	u_char flag;
#define FWTI_REQ	(1 << 0)
#define FWTI_RES	(1 << 1)
#define FWTI_TLABEL	(1 << 2)
#define FWTI_BLOCK_STR	(1 << 3)
#define FWTI_BLOCK_ASY	(1 << 4)
};

struct firewire_comm{
	device_t dev;
	device_t bdev;
	u_int16_t busid:10,
		  nodeid:6;
	u_int mode;
	u_int nport;
	u_int speed;
	u_int maxrec;
	u_int irm;
	u_int max_node;
	u_int max_hop;
	u_int max_asyretry;
#define FWPHYASYST (1 << 0)
	u_int retry_count;
	u_int32_t ongobus:10,
		  ongonode:6,
		  ongoaddr:16;
	struct fw_device *ongodev;
	struct fw_eui64 ongoeui;
#define	FWMAXCSRDIR     16
	SLIST_HEAD(, csrdir) ongocsr;
	SLIST_HEAD(, csrdir) csrfree;
	u_int32_t status;
#define	FWBUSNOTREADY	(-1)
#define	FWBUSRESET	0
#define	FWBUSINIT	1
#define	FWBUSCYMELECT	2
#define	FWBUSMGRELECT	3
#define	FWBUSMGRDONE	4
#define	FWBUSEXPLORE	5
#define	FWBUSPHYCONF	6
#define	FWBUSEXPDONE	7
#define	FWBUSCOMPLETION	10
	int nisodma;
	struct fw_eui64 eui;
	struct fw_xferq
		*arq, *atq, *ars, *ats, *it[FW_MAX_DMACH],*ir[FW_MAX_DMACH];
	STAILQ_HEAD(, tlabel) tlabels[0x40];
	STAILQ_HEAD(, fw_bind) binds;
	STAILQ_HEAD(, fw_device) devices;
	STAILQ_HEAD(, fw_xfer)	pending;
	u_int  sid_cnt;
#define CSRSIZE 0x4000
	u_int32_t csr_arc[CSRSIZE/4];
#define CROMSIZE 0x400
	u_int32_t *config_rom;
	struct crom_src_buf *crom_src_buf;
	struct crom_src *crom_src;
	struct crom_chunk *crom_root;
	struct fw_topology_map *topology_map;
	struct fw_speed_map *speed_map;
	struct callout busprobe_callout;
	struct callout bmr_callout;
	struct callout timeout_callout;
	struct callout retry_probe_callout;
	u_int32_t (*cyctimer) __P((struct  firewire_comm *));
	void (*ibr) __P((struct firewire_comm *));
	u_int32_t (*set_bmr) __P((struct firewire_comm *, u_int32_t));
	int (*ioctl) __P((dev_t, u_long, caddr_t, int, fw_proc *));
	int (*irx_enable) __P((struct firewire_comm *, int));
	int (*irx_disable) __P((struct firewire_comm *, int));
	int (*itx_enable) __P((struct firewire_comm *, int));
	int (*itx_disable) __P((struct firewire_comm *, int));
	void (*timeout) __P((void *));
	void (*poll) __P((struct firewire_comm *, int, int));
	void (*set_intr) __P((struct firewire_comm *, int));
	void (*irx_post) __P((struct firewire_comm *, u_int32_t *));
	void (*itx_post) __P((struct firewire_comm *, u_int32_t *));
	struct tcode_info *tcode;
	bus_dma_tag_t dmat;
};
#define CSRARC(sc, offset) ((sc)->csr_arc[(offset)/4])

struct csrdir{
	u_int32_t ongoaddr;
	u_int32_t off;
	SLIST_ENTRY(csrdir) link;
};

struct fw_xferq {
	int flag;
#define FWXFERQ_CHTAGMASK 0xff
#define FWXFERQ_RUNNING (1 << 8)
#define FWXFERQ_STREAM (1 << 9)

#define FWXFERQ_BULK (1 << 11)
#define FWXFERQ_MODEMASK (7 << 10)

#define FWXFERQ_EXTBUF (1 << 13)
#define FWXFERQ_OPEN (1 << 14)

#define FWXFERQ_HANDLER (1 << 16)
#define FWXFERQ_WAKEUP (1 << 17)
	void (*start) __P((struct firewire_comm*));
	int dmach;
	STAILQ_HEAD(, fw_xfer) q;
	u_int queued;
	u_int maxq;
	u_int psize;
	STAILQ_HEAD(, fw_bind) binds;
	struct fwdma_alloc_multi *buf;
	u_int bnchunk;
	u_int bnpacket;
	struct fw_bulkxfer *bulkxfer;
	STAILQ_HEAD(, fw_bulkxfer) stvalid;
	STAILQ_HEAD(, fw_bulkxfer) stfree;
	STAILQ_HEAD(, fw_bulkxfer) stdma;
	struct fw_bulkxfer *stproc;
	struct selinfo rsel;
	caddr_t sc;
	void (*hand) __P((struct fw_xferq *));
};

struct fw_bulkxfer{
	int poffset;
	struct mbuf *mbuf;
	STAILQ_ENTRY(fw_bulkxfer) link;
	caddr_t start;
	caddr_t end;
	int resp;
};

struct tlabel{
	struct fw_xfer  *xfer;
	STAILQ_ENTRY(tlabel) link;
};

struct fw_bind{
	u_int32_t start_hi, start_lo, addrlen;
	STAILQ_HEAD(, fw_xfer) xferlist;
	STAILQ_ENTRY(fw_bind) fclist;
	STAILQ_ENTRY(fw_bind) chlist;
#define FWACT_NULL	0
#define FWACT_XFER	2
#define FWACT_CH	3
	u_int8_t act_type;
	u_int8_t sub;
};

struct fw_xfer{
	caddr_t sc;
	struct firewire_comm *fc;
	struct fw_xferq *q;
	struct timeval tv;
	/* XXX should be removed */
	u_int32_t dst; /* XXX for if_fwe */
	u_int8_t spd;
	int8_t resp;
#define FWXF_INIT 0
#define FWXF_INQ 1
#define FWXF_START 2
#define FWXF_SENT 3
#define FWXF_SENTERR 4
#define FWXF_BUSY 8
#define FWXF_RCVD 10
	u_int8_t state;
	u_int8_t retry;
	u_int8_t tl;
	void (*retry_req) __P((struct fw_xfer *));
	union{
		void (*hand) __P((struct fw_xfer *));
	} act;
	struct {
		int len;
		caddr_t buf;
	} send, recv;
	struct mbuf *mbuf;
	STAILQ_ENTRY(fw_xfer) link;
	struct malloc_type *malloc;
};
void fw_sidrcv __P((struct firewire_comm *, u_int32_t *, u_int));
void fw_rcv __P((struct firewire_comm *, struct iovec *, int, u_int, u_int));
void fw_xfer_unload __P(( struct fw_xfer*));
void fw_xfer_free __P(( struct fw_xfer*));
struct fw_xfer *fw_xfer_alloc __P((struct malloc_type *));
struct fw_xfer *fw_xfer_alloc_buf __P((struct malloc_type *, int, int));
void fw_init __P((struct firewire_comm *));
int fw_tbuf_update __P((struct firewire_comm *, int, int));
int fw_rbuf_update __P((struct firewire_comm *, int, int));
void fw_asybusy __P((struct fw_xfer *));
int fw_bindadd __P((struct firewire_comm *, struct fw_bind *));
int fw_bindremove __P((struct firewire_comm *, struct fw_bind *));
int fw_asyreq __P((struct firewire_comm *, int, struct fw_xfer*));
void fw_busreset __P((struct firewire_comm *));
u_int16_t fw_crc16 __P((u_int32_t *, u_int32_t));
void fw_xfer_timeout __P((void *));
void fw_xfer_done __P((struct fw_xfer *));
void fw_asy_callback __P((struct fw_xfer *));
struct fw_device *fw_noderesolve_nodeid __P((struct firewire_comm *, int));
struct fw_device *fw_noderesolve_eui64 __P((struct firewire_comm *, struct fw_eui64 *));
struct fw_bind *fw_bindlookup __P((struct firewire_comm *, u_int32_t, u_int32_t));
void fw_drain_txq __P((struct firewire_comm *));


extern int firewire_debug;
extern devclass_t firewire_devclass;

#define		FWPRI		((PZERO+8)|PCATCH)

#if __FreeBSD_version >= 500000
#define CALLOUT_INIT(x) callout_init(x, 0 /* mpsafe */)
#else
#define CALLOUT_INIT(x) callout_init(x)
#endif

MALLOC_DECLARE(M_FW);
MALLOC_DECLARE(M_FWXFER);
