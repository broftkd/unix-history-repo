/*-
 * Copyright (c) 2003 Jake Burkholder <jake@freebsd.org>.
 * Copyright (c) 2003 Marcel Moolenaar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

/*
 * Machine-dependent thread prototypes/definitions.
 */
#ifndef _PTHREAD_MD_H_
#define	_PTHREAD_MD_H_

#include <stddef.h>

#define	CPU_SPINWAIT

#define	DTV_OFFSET		offsetof(struct tcb, tcb_dtv)

/*
 * Variant II tcb, first two members are required by rtld.
 * %g7 points to the structure.
 */
struct tcb {
	struct tcb		*tcb_self;	/* required by rtld */
	void			*tcb_dtv;	/* required by rtld */
	struct pthread		*tcb_thread;	/* our hook */
	void			*tcb_spare[1];
};

/* Called from the thread to set its private data. */
static __inline void
_tcb_set(struct tcb *tcb)
{

	__asm __volatile("mov %0, %%g7" : : "r" (tcb));
}

static __inline struct tcb *
_tcb_get(void)
{
	register struct tcb *tp __asm("%g7");

	return (tp);
}

static __inline struct pthread *
_get_curthread(void)
{

	return (_tcb_get()->tcb_thread);
}

#define HAS__UMTX_OP_ERR	1

#endif /* _PTHREAD_MD_H_ */
