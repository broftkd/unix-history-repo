/*-
 * Copyright (c) 1982 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1982 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)comp.c	5.2 (Berkeley) %G%";
#endif /* not lint */

#include <stdio.h>
#define MAX ' '

char new[MAX], old[MAX];

main ()
{
	register int i, j;
	old[0] = '\0';
	while (fgets(&new[0], MAX, stdin) != NULL) {
		for (i=0; i<MAX && old[i]==new[i]; i++);
		if (i >= MAX) {
			fprintf(stderr, "long word\n");
			exit(1);
		}
		putc(i, stdout);
		for (j=0; (old[j]=new[j]) != '\n'; j++);
		old[j] = '\0';
		fputs(&old[i], stdout);
	}
}
