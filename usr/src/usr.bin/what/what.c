/*
 * Copyright (c) 1980, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)what.c	5.3 (Berkeley) %G%";
#endif /* not lint */

#include <stdio.h>

/*
 * what
 */
/* ARGSUSED */
main(argc, argv)
	int argc;
	char **argv;
{
	if (!*++argv) 
		search();
	else do {
		if (!freopen(*argv, "r", stdin)) {
			perror(*argv);
			exit(1);
		}
		printf("%s\n", *argv);
		search();
	} while(*++argv);
	exit(0);
}

static
search()
{
	register int c;

	while ((c = getchar()) != EOF) {
loop:		if (c != '@')
			continue;
		if ((c = getchar()) != '(')
			goto loop;
		if ((c = getchar()) != '#')
			goto loop;
		if ((c = getchar()) != ')')
			goto loop;
		putchar('\t');
		while ((c = getchar()) != EOF && c && c != '"' &&
		    c != '>' && c != '\n')
			putchar(c);
		putchar('\n');
	}
}
