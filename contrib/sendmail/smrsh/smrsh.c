/*
 * Copyright (c) 1998 Sendmail, Inc.  All rights reserved.
 * Copyright (c) 1993 Eric P. Allman.  All rights reserved.
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the sendmail distribution.
 *
 */

#ifndef lint
static char sccsid[] = "@(#)smrsh.c	8.11 (Berkeley) 5/19/1998";
#endif /* not lint */

/*
**  SMRSH -- sendmail restricted shell
**
**	This is a patch to get around the prog mailer bugs in most
**	versions of sendmail.
**
**	Use this in place of /bin/sh in the "prog" mailer definition
**	in your sendmail.cf file.  You then create CMDDIR (owned by
**	root, mode 755) and put links to any programs you want
**	available to prog mailers in that directory.  This should
**	include things like "vacation" and "procmail", but not "sed"
**	or "sh".
**
**	Leading pathnames are stripped from program names so that
**	existing .forward files that reference things like
**	"/usr/bin/vacation" will continue to work.
**
**	The following characters are completely illegal:
**		<  >  |  ^  ;  &  $  `  (  ) \n \r
**	This is more restrictive than strictly necessary.
**
**	To use this, edit /etc/sendmail.cf, search for ^Mprog, and
**	change P=/bin/sh to P=/usr/libexec/smrsh, where this compiled
**	binary is installed /usr/libexec/smrsh.
**
**	This can be used on any version of sendmail.
**
**	In loving memory of RTM.  11/02/93.
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#ifdef EX_OK
# undef EX_OK
#endif
#include <sysexits.h>
#include <syslog.h>
#include <stdlib.h>

/* directory in which all commands must reside */
#ifndef CMDDIR
# define CMDDIR		"/usr/libexec/sm.bin"
#endif

/* characters disallowed in the shell "-c" argument */
#define SPECIALS	"<|>^();&`$\r\n"

/* default search path */
#ifndef PATH
# define PATH		"/bin:/usr/bin"
#endif

int
main(argc, argv)
	int argc;
	char **argv;
{
	register char *p;
	register char *q;
	register char *cmd;
	int i;
	char *newenv[2];
	char cmdbuf[1000];
	char pathbuf[1000];

#ifndef LOG_MAIL
	openlog("smrsh", 0);
#else
	openlog("smrsh", LOG_ODELAY|LOG_CONS, LOG_MAIL);
#endif

	strcpy(pathbuf, "PATH=");
	strcat(pathbuf, PATH);
	newenv[0] = pathbuf;
	newenv[1] = NULL;

	/*
	**  Do basic argv usage checking
	*/

	if (argc != 3 || strcmp(argv[1], "-c") != 0)
	{
		fprintf(stderr, "Usage: %s -c command\n", argv[0]);
		syslog(LOG_ERR, "usage");
		exit(EX_USAGE);
	}

	/*
	**  Disallow special shell syntax.  This is overly restrictive,
	**  but it should shut down all attacks.
	**  Be sure to include 8-bit versions, since many shells strip
	**  the address to 7 bits before checking.
	*/

	strcpy(cmdbuf, SPECIALS);
	for (p = cmdbuf; *p != '\0'; p++)
		*p |= '\200';
	strcat(cmdbuf, SPECIALS);
	p = strpbrk(argv[2], cmdbuf);
	if (p != NULL)
	{
		fprintf(stderr, "%s: cannot use %c in command\n",
			argv[0], *p);
		syslog(LOG_CRIT, "uid %d: attempt to use %c in command: %s",
			getuid(), *p, argv[2]);
		exit(EX_UNAVAILABLE);
	}

	/*
	**  Do a quick sanity check on command line length.
	*/

	i = strlen(argv[2]);
	if (i > (sizeof cmdbuf - sizeof CMDDIR - 2))
	{
		fprintf(stderr, "%s: command too long: %s\n", argv[0], argv[2]);
		syslog(LOG_WARNING, "command too long: %.40s", argv[2]);
		exit(EX_UNAVAILABLE);
	}

	/*
	**  Strip off a leading pathname on the command name.  For
	**  example, change /usr/ucb/vacation to vacation.
	*/

	/* strip leading spaces */
	for (q = argv[2]; *q != '\0' && isascii(*q) && isspace(*q); )
		q++;

	/* find the end of the command name */
	p = strpbrk(q, " \t");
	if (p == NULL)
		cmd = &q[strlen(q)];
	else
	{
		*p = '\0';
		cmd = p;
	}

	/* search backwards for last / (allow for 0200 bit) */
	while (cmd > q)
	{
		if ((*--cmd & 0177) == '/')
		{
			cmd++;
			break;
		}
	}

	/* cmd now points at final component of path name */

	/*
	**  Check to see if the command name is legal.
	*/

	(void) strcpy(cmdbuf, CMDDIR);
	(void) strcat(cmdbuf, "/");
	(void) strcat(cmdbuf, cmd);
#ifdef DEBUG
	printf("Trying %s\n", cmdbuf);
#endif
	if (access(cmdbuf, X_OK) < 0)
	{
		/* oops....  crack attack possiblity */
		fprintf(stderr, "%s: %s not available for sendmail programs\n",
			argv[0], cmd);
		if (p != NULL)
			*p = ' ';
		syslog(LOG_CRIT, "uid %d: attempt to use %s", getuid(), cmd);
		exit(EX_UNAVAILABLE);
	}
	if (p != NULL)
		*p = ' ';

	/*
	**  Create the actual shell input.
	*/

	strcpy(cmdbuf, CMDDIR);
	strcat(cmdbuf, "/");
	strcat(cmdbuf, cmd);

	/*
	**  Now invoke the shell
	*/

#ifdef DEBUG
	printf("%s\n", cmdbuf);
#endif
	execle("/bin/sh", "/bin/sh", "-c", cmdbuf, NULL, newenv);
	syslog(LOG_CRIT, "Cannot exec /bin/sh: %m");
	perror("/bin/sh");
	exit(EX_OSFILE);
}
