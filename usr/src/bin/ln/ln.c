/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1987, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)ln.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int	dirflag,			/* Undocumented force flag. */
	sflag,				/* Symbolic, not hard, link. */
					/* System link call. */
	(*linkf) __P((const char *, const char *));

static int	linkit __P((char *, char *, int));
static void	usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern int optind;
	struct stat sb;
	int ch, exitval;
	char *sourcedir;

	while ((ch = getopt(argc, argv, "Fs")) != EOF)
		switch((char)ch) {
		case 'F':
			dirflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case '?':
		default:
			usage();
		}

	argv += optind;
	argc -= optind;

	linkf = sflag ? symlink : link;

	switch(argc) {
	case 0:
		usage();
	case 1:				/* ln target */
		exit(linkit(argv[0], ".", 1));
	case 2:				/* ln target source */
		exit(linkit(argv[0], argv[1], 0));
	}
					/* ln target1 target2 directory */
	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &sb))
		err(1, "%s", sourcedir);
	if (!S_ISDIR(sb.st_mode))
		usage();
	for (exitval = 0; *argv != sourcedir; ++argv)
		exitval |= linkit(*argv, sourcedir, 1);
	exit(exitval);
}

static int
linkit(target, source, isdir)
	char *target, *source;
	int isdir;
{
	struct stat sb;
	char path[MAXPATHLEN], *p;

	if (!sflag) {
		/* if target doesn't exist, quit now */
		if (stat(target, &sb)) {
			warn("%s", target);
			return (1);
		}
		/* only symbolic links to directories, unless -F option used */
		if (!dirflag && (sb.st_mode & S_IFMT) == S_IFDIR) {
			warnx("%s: is a directory", target);
			return (1);
		}
	}

	/* if the source is a directory, append the target's name */
	if (isdir || !stat(source, &sb) && (sb.st_mode & S_IFMT) == S_IFDIR) {
		if (!(p = strrchr(target, '/')))
			p = target;
		else
			++p;
		(void)snprintf(path, sizeof(path), "%s/%s", source, p);
		source = path;
	}

	if ((*linkf)(target, source)) {
		warn("%s", source);
		return (1);
	}
	return (0);
}

static void
usage()
{
	(void)fprintf(stderr,
	    "usage:\tln [-s] file1 file2\n\tln [-s] file ... directory\n");
	exit(1);
}
