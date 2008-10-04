/*-
 * Copyright (c) 2008 Semihalf, Rafal Jaworowski <raj@semihalf.com>
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

/*
 * This is the header file for conveniency wrapper routines (API glue)
 */

#ifndef _API_GLUE_H_
#define _API_GLUE_H_

#include "api_public.h"

int syscall(int, int *, ...);
void *syscall_ptr;

int api_search_sig(struct api_signature **sig);

#define	UB_MAX_MR	5		/* max mem regions number */
#define	UB_MAX_DEV	6		/* max devices number */

/*
 * The ub_ library calls are part of the application, not U-Boot code!  They
 * are front-end wrappers that are used by the consumer application: they
 * prepare arguments for particular syscall and jump to the low level
 * syscall()
 */

/* console */
int ub_getc(void);
int ub_tstc(void);
void ub_putc(char c);
void ub_puts(const char *s);

/* system */
void ub_reset(void);
struct sys_info *ub_get_sys_info(void);

/* time */
void ub_udelay(unsigned long);
unsigned long ub_get_timer(unsigned long);

/* env vars */
char *ub_env_get(const char *name);
void ub_env_set(const char *name, char *value);
const char *ub_env_enum(const char *last);

/* devices */
int ub_dev_enum(void);
int ub_dev_open(int handle);
int ub_dev_close(int handle);
int ub_dev_read(int handle, void *buf, lbasize_t len, lbastart_t start,
    lbasize_t *rlen);
int ub_dev_send(int handle, void *buf, int len);
int ub_dev_recv(int handle, void *buf, int len, int *rlen);
struct device_info *	ub_dev_get(int);

void ub_dump_di(int);
void ub_dump_si(struct sys_info *);
char *ub_mem_type(int);

#endif /* _API_GLUE_H_ */
