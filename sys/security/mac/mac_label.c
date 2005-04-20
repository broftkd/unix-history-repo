/*-
 * Copyright (c) 2003-2004 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project in part by Network
 * Associates Laboratories, the Security Research Division of Network
 * Associates, Inc. under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"),
 * as part of the DARPA CHATS research program.
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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_mac.h"

#include <sys/param.h>
#include <sys/mac.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <vm/uma.h>

#include <security/mac/mac_internal.h>

uma_zone_t	zone_label;

static int	mac_labelzone_ctor(void *mem, int size, void *arg, int flags);
static void	mac_labelzone_dtor(void *mem, int size, void *arg);

void
mac_labelzone_init(void)
{

	zone_label = uma_zcreate("MAC labels", sizeof(struct label),
	    mac_labelzone_ctor, mac_labelzone_dtor, NULL, NULL,
	    UMA_ALIGN_PTR, 0);
}

static int
mac_labelzone_ctor(void *mem, int size, void *arg, int flags)
{
	struct label *label;

	KASSERT(size == sizeof(*label), ("mac_labelzone_ctor: wrong size\n"));
	label = mem;
	bzero(label, sizeof(*label));
	label->l_flags = MAC_FLAG_INITIALIZED;
	return (0);
}

static void
mac_labelzone_dtor(void *mem, int size, void *arg)
{
	struct label *label;

	KASSERT(size == sizeof(*label), ("mac_labelzone_dtor: wrong size\n"));
	label = mem;
	KASSERT(label->l_flags & MAC_FLAG_INITIALIZED,
	    ("mac_labelzone_dtor: label not initialized"));
#ifdef DIAGNOSTIC
	bzero(label, sizeof(*label));
#else
	label->l_flags &= ~MAC_FLAG_INITIALIZED;
#endif
}

struct label *
mac_labelzone_alloc(int flags)
{

	return (uma_zalloc(zone_label, flags));
}

void
mac_labelzone_free(struct label *label)
{

	uma_zfree(zone_label, label);
}
