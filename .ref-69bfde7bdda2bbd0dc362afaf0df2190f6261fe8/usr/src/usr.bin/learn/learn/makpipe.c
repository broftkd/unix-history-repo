/*-
 * Copyright (c) 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
static char sccsid[] = "@(#)makpipe.c	8.1 (Berkeley) %G%";
#endif /* not lint */

#include "stdio.h"
#include "pathnames.h"

makpipe()
{
	int f[2];

	pipe(f);
	if (fork()==0) {
		close(f[1]);
		close(0);
		dup(f[0]);
		close(f[0]);
#if BSD4_2
		execl (_PATH_BSHELL, "sh", "-i", 0);
#else
		execlp(_PATH_CSHELL, "csh", "-if", 0);
#endif
		write(2, "Exec error\n", 11);
	}
	close(f[0]);
	sleep(2);	/* so shell won't eat up too much input */
	return(f[1]);
}
