/*
 * Copyright (c) 1982, 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)user.h	8.2 (Berkeley) %G%
 */

#include <machine/pcb.h>
#ifndef KERNEL
/* stuff that *used* to be included by user.h, or is now needed */
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ucred.h>
#include <sys/uio.h>
#endif
#include <sys/resourcevar.h>
#include <sys/signalvar.h>
#include <vm/vm.h>		/* XXX */
#include <sys/sysctl.h>


/*
 * Per process structure containing data that isn't needed in core
 * when the process isn't running (esp. when swapped out).
 * This structure may or may not be at the same kernel address
 * in all processes.
 */
 
struct	user {
	struct	pcb u_pcb;

	struct	sigacts u_sigacts;	/* p_sigacts points here (use it!) */
	struct	pstats u_stats;		/* p_stats points here (use it!) */

	/*
	 * Remaining fields only for core dump and/or ptrace--
	 * not valid at other times!
	 */
	struct	kinfo_proc u_kproc;	/* proc + eproc */
	struct	md_coredump u_md;	/* machine dependent glop */
};

/*
 * Redefinitions to make the debuggers happy for now...  This subterfuge
 * brought to you by coredump() and trace_req().  These fields are *only*
 * valid at those times!
 */
#define	U_ar0	u_kproc.kp_proc.p_md.md_regs /* copy of curproc->p_md.md_regs */
#define	U_tsize	u_kproc.kp_eproc.e_vm.vm_tsize
#define	U_dsize	u_kproc.kp_eproc.e_vm.vm_dsize
#define	U_ssize	u_kproc.kp_eproc.e_vm.vm_ssize
#define	U_sig	u_sigacts.ps_sig
#define	U_code	u_sigacts.ps_code

#ifndef KERNEL
#define	u_ar0	U_ar0
#define	u_tsize	U_tsize
#define	u_dsize	U_dsize
#define	u_ssize	U_ssize
#define	u_sig	U_sig
#define	u_code	U_code
#endif /* KERNEL */
