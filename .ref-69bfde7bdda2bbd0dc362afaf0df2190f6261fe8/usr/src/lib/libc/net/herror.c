/*-
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * -
 * --Copyright--
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)herror.c	8.1 (Berkeley) %G%";
static char rcsid[] = "$Id: herror.c,v 4.9.1.1 1993/05/02 23:14:35 vixie Rel $";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/uio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

char	*h_errlist[] = {
	"Error 0",
	"Unknown host",				/* 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/* 2 TRY_AGAIN */
	"Unknown server error",			/* 3 NO_RECOVERY */
	"No address associated with name",	/* 4 NO_ADDRESS */
};
int	h_nerr = { sizeof(h_errlist)/sizeof(h_errlist[0]) };

extern int	h_errno;

/*
 * herror --
 *	print the error indicated by the h_errno value.
 */
void
herror(s)
	const char *s;
{
	struct iovec iov[4];
	register struct iovec *v = iov;

	if (s && *s) {
		v->iov_base = (char *)s;
		v->iov_len = strlen(s);
		v++;
		v->iov_base = ": ";
		v->iov_len = 2;
		v++;
	}
	v->iov_base = (u_int)h_errno < h_nerr ?
	    h_errlist[h_errno] : "Unknown error";
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = "\n";
	v->iov_len = 1;
	writev(STDERR_FILENO, iov, (v - iov) + 1);
}

char *
hstrerror(err)
	int err;
{
	return (u_int)err < h_nerr ? h_errlist[err] : "Unknown resolver error";
}
