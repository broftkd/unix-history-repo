#ifndef lint
static char *sccsid = "@(#)cron.c	4.9 (Berkeley) %G%";
#endif

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <pwd.h>

#define	LISTS	(2*BUFSIZ)
#define	MAXLIN	BUFSIZ

#ifndef CRONTAB
#define CRONTAB "/usr/lib/crontab"
#endif

#define	EXACT	100
#define	ANY	101
#define	LIST	102
#define	RANGE	103
#define	EOS	104
char	crontab[]	= CRONTAB;
time_t	itime;
struct	tm *loct;
struct	tm *localtime();
char	*malloc();
char	*realloc();
int	reapchild();
int	flag;
char	*list;
unsigned listsize;

main()
{
	register char *cp;
	char *cmp();
	time_t filetime = 0;

	if (fork())
		exit(0);
	chdir("/");
	freopen("/", "r", stdout);
	freopen("/", "r", stderr);
	untty();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGCHLD, reapchild);
	time(&itime);
	itime -= localtime(&itime)->tm_sec;

	for (;; itime+=60, slp()) {
		struct stat cstat;

		if (stat(crontab, &cstat) == -1)
			continue;
		if (cstat.st_mtime > filetime) {
			filetime = cstat.st_mtime;
			init();
		}
		loct = localtime(&itime);
		loct->tm_mon++;		 /* 1-12 for month */
		if (loct->tm_wday == 0)
			loct->tm_wday = 7;	/* sunday is 7, not 0 */
		for(cp = list; *cp != EOS;) {
			flag = 0;
			cp = cmp(cp, loct->tm_min);
			cp = cmp(cp, loct->tm_hour);
			cp = cmp(cp, loct->tm_mday);
			cp = cmp(cp, loct->tm_mon);
			cp = cmp(cp, loct->tm_wday);
			if(flag == 0)
				ex(cp);
			while(*cp++ != 0)
				;
		}
	}
}

char *
cmp(p, v)
char *p;
{
	register char *cp;

	cp = p;
	switch(*cp++) {

	case EXACT:
		if (*cp++ != v)
			flag++;
		return(cp);

	case ANY:
		return(cp);

	case LIST:
		while(*cp != LIST)
			if(*cp++ == v) {
				while(*cp++ != LIST)
					;
				return(cp);
			}
		flag++;
		return(cp+1);

	case RANGE:
		if(*cp > v || cp[1] < v)
			flag++;
		return(cp+2);
	}
	if(cp[-1] != v)
		flag++;
	return(cp);
}

slp()
{
	register i;
	time_t t;

	time(&t);
	i = itime - t;
	if(i < -60 * 60 || i > 60 * 60) {
		itime = t;
		i = 60 - localtime(&itime)->tm_sec;
		itime += i;
	}
	if(i > 0)
		sleep(i);
}

ex(s)
char *s;
{
	int st;
	register struct passwd *pwd;
	char user[BUFSIZ];
	char *c = user;

	if(fork()) {
		return;
	}

	while(*s != ' ' && *s != '\t')
		*c++ = *s++;
	*c = '\0';
	if ((pwd = getpwnam(user)) == NULL) {
		exit(1);
	}
	(void) setgid(pwd->pw_gid);
	initgroups(pwd->pw_name, pwd->pw_gid);
	(void) setuid(pwd->pw_uid);
	freopen("/", "r", stdin);
	execl("/bin/sh", "sh", "-c", ++s, 0);
	exit(0);
}

init()
{
	register i, c;
	register char *cp;
	register char *ocp;
	register int n;

	freopen(crontab, "r", stdin);
	if (list) {
		free(list);
		list = realloc(list, LISTS);
	} else
		list = malloc(LISTS);
	listsize = LISTS;
	cp = list;

loop:
	if(cp > list+listsize-MAXLIN) {
		char *olist;
		listsize += LISTS;
		olist = list;
		free(list);
		list = realloc(list, listsize);
		cp = list + (cp - olist);
	}
	ocp = cp;
	for(i=0;; i++) {
		do
			c = getchar();
		while(c == ' ' || c == '\t')
			;
		if(c == EOF || c == '\n')
			goto ignore;
		if(i == 5)
			break;
		if(c == '*') {
			*cp++ = ANY;
			continue;
		}
		if ((n = number(c)) < 0)
			goto ignore;
		c = getchar();
		if(c == ',')
			goto mlist;
		if(c == '-')
			goto mrange;
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = EXACT;
		*cp++ = n;
		continue;

	mlist:
		*cp++ = LIST;
		*cp++ = n;
		do {
			if ((n = number(getchar())) < 0)
				goto ignore;
			*cp++ = n;
			c = getchar();
		} while (c==',');
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = LIST;
		continue;

	mrange:
		*cp++ = RANGE;
		*cp++ = n;
		if ((n = number(getchar())) < 0)
			goto ignore;
		c = getchar();
		if(c != '\t' && c != ' ')
			goto ignore;
		*cp++ = n;
	}
	while(c != '\n') {
		if(c == EOF)
			goto ignore;
		if(c == '%')
			c = '\n';
		*cp++ = c;
		c = getchar();
	}
	*cp++ = '\n';
	*cp++ = 0;
	goto loop;

ignore:
	cp = ocp;
	while(c != '\n') {
		if(c == EOF) {
			*cp++ = EOS;
			*cp++ = EOS;
			fclose(stdin);
			return;
		}
		c = getchar();
	}
	goto loop;
}

number(c)
register c;
{
	register n = 0;

	while (isdigit(c)) {
		n = n*10 + c - '0';
		c = getchar();
	}
	ungetc(c, stdin);
	if (n>=100)
		return(-1);
	return(n);
}

reapchild()
{
	union wait status;

	while(wait3(&status, WNOHANG, 0) > 0)
		;
}

untty()
{
	int i;

	i = open("/dev/tty", O_RDWR);
	if (i >= 0) {
		ioctl(i, TIOCNOTTY, (char *)0);
		(void) close(i);
	}
}
