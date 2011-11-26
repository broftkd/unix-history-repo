/*-
 * Copyright (c) 2011 Mikolaj Golub
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/param.h>
#include <sys/time.h>
#define _RLIMIT_IDENT
#include <sys/resourcevar.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#include <err.h>
#include <errno.h>
#include <libprocstat.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "procstat.h"

static struct rlimit rlimit[RLIM_NLIMITS];

void
procstat_rlimit(struct kinfo_proc *kipp)
{
	int error, i, name[4];
	size_t len;

	if (!hflag)
		printf("%5s %-16s %-10s %12s %12s\n", "PID", "COMM", "RLIMIT",
		    "CURRENT", "MAX");
	name[0] = CTL_KERN;
	name[1] = KERN_PROC;
	name[2] = KERN_PROC_RLIMIT;
	name[3] = kipp->ki_pid;
	len = sizeof(rlimit);
	error = sysctl(name, 4, rlimit, &len, NULL, 0);
	if (error < 0 && errno != ESRCH) {
		warn("sysctl: kern.proc.rlimit: %d", kipp->ki_pid);
		return;
	}
	if (error < 0 || len != sizeof(rlimit))
		return;

	for (i = 0; i < RLIM_NLIMITS; i++) {
		printf("%5d %-16s %-10s %12jd %12jd\n", kipp->ki_pid,
		    kipp->ki_comm, rlimit_ident[i],
		    rlimit[i].rlim_cur == RLIM_INFINITY ?
		    -1 : rlimit[i].rlim_cur,
		    rlimit[i].rlim_max == RLIM_INFINITY ?
		    -1 : rlimit[i].rlim_max);
        }
}
