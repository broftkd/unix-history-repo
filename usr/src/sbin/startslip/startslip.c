
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1990, 1991 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)startslip.c	5.8 (Berkeley) %G%";
#endif /* not lint */

#include <sys/param.h>
#if BSD >= 199006
#define POSIX
#endif
#ifdef POSIX
#include <sys/termios.h>
#include <sys/ioctl.h>
#else
#include <sgtty.h>
#endif
#include <sys/socket.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_slvar.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

#define DEFAULT_BAUD    B9600
int     speed = DEFAULT_BAUD;
int	hup;
int	logged_in;
int	wait_time = 60;		/* then back off */
#define	MAXTRIES	6	/* w/60 sec and doubling, takes an hour */
#define	PIDFILE		"/var/run/startslip.pid"

#ifdef DEBUG
int	debug = 1;
#undef LOG_ERR
#undef LOG_INFO
#define syslog fprintf
#define LOG_ERR stderr
#define LOG_INFO stderr
#else
int	debug = 0;
#endif
#define	printd	if (debug) printf

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	char *cp, **ap;
	int ch, disc;
	int fd = -1;
	void sighup();
	FILE *wfd = NULL, *pfd;
	char *dialerstring = 0, buf[BUFSIZ];
	int first = 1, tries = 0;
	int pausefirst = 0;
	int pid;
#ifdef POSIX
	struct termios t;
#else
	struct sgttyb sgtty;
#endif

	while ((ch = getopt(argc, argv, "db:s:p:")) != EOF)
		switch (ch) {
		case 'd':
			debug = 1;
			break;
#ifdef POSIX
		case 'b':
			speed = atoi(optarg);
			break;
#endif
		case 'p':
			pausefirst = atoi(optarg);
			break;
		case 's':
			dialerstring = optarg;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 3)
		usage();

	openlog("startslip", LOG_PID, LOG_DAEMON);

#if BSD <= 43
	if (debug == 0 && (fd = open("/dev/tty", 0)) >= 0) {
		ioctl(fd, TIOCNOTTY, 0);
		close(fd);
		fd = -1;
	}
#endif

	if (debug)
		setbuf(stdout, NULL);
	
	if (pfd = fopen(PIDFILE, "r")) {
		pid = 0;
		fscanf(pfd, "%d", &pid);
		if (pid > 0)
			kill(pid, SIGUSR1);
		fclose(pfd);
	}
restart:
	logged_in = 0;
	if (++tries > MAXTRIES) {
		syslog(LOG_ERR, "exiting after %d tries\n", tries);
		/* ???
		if (first)
		*/
			exit(1);
	}

	/*
	 * We may get a HUP below, when the parent (session leader/
	 * controlling process) exits; ignore HUP until into new session.
	 */
	signal(SIGHUP, SIG_IGN);
	hup = 0;
	if (fork() > 0) {
		if (pausefirst)
			sleep(pausefirst);
		if (first)
			printd("parent exit\n");
		exit(0);
	}
	pausefirst = 0;
#ifdef POSIX
	if (setsid() == -1)
		perror("setsid");
#endif
	pid = getpid();
	printd("restart: pid %d: ", pid);
	if (pfd = fopen(PIDFILE, "w")) {
		fprintf(pfd, "%d\n", pid);
		fclose(pfd);
	}
	if (wfd) {
		printd("fclose, ");
		fclose(wfd);
		wfd == NULL;
	}
	if (fd >= 0) {
		printd("close, ");
		close(fd);
		sleep(5);
	}
	printd("open");
	if ((fd = open(argv[0], O_RDWR)) < 0) {
		perror(argv[0]);
		syslog(LOG_ERR, "open %s: %m\n", argv[0]);
		if (first)
			exit(1);
		else {
			sleep(wait_time * tries);
			goto restart;
		}
	}
	printd(" %d", fd);
#ifdef TIOCSCTTY
	if (ioctl(fd, TIOCSCTTY, 0) < 0)
		perror("ioctl (TIOCSCTTY)");
#endif
	signal(SIGHUP, sighup);
	if (debug) {
		if (ioctl(fd, TIOCGETD, &disc) < 0)
			perror("ioctl(TIOCSETD)");
		printf(" (disc was %d)", disc);
	}
	disc = TTYDISC;
	if (ioctl(fd, TIOCSETD, &disc) < 0) {
	        perror("ioctl(TIOCSETD)");
		syslog(LOG_ERR, "%s: ioctl (TIOCSETD 0): %m\n",
		    argv[0]);
	}
	printd(", ioctl");
#ifdef POSIX
	if (tcgetattr(fd, &t) < 0) {
		perror("tcgetattr");
		syslog(LOG_ERR, "%s: tcgetattr: %m\n", argv[0]);
	        exit(2);
	}
	cfmakeraw(&t);
	t.c_iflag &= ~IMAXBEL;
	t.c_cflag |= CRTSCTS;
	cfsetspeed(&t, speed);
	if (tcsetattr(fd, TCSAFLUSH, &t) < 0) {
		perror("tcsetattr");
		syslog(LOG_ERR, "%s: tcsetattr: %m\n", argv[0]);
	        if (first) 
			exit(2);
		else {
			sleep(wait_time * tries);
			goto restart;
		}
	}
#else
	if (ioctl(fd, TIOCGETP, &sgtty) < 0) {
	        perror("ioctl (TIOCGETP)");
		syslog(LOG_ERR, "%s: ioctl (TIOCGETP): %m\n",
		    argv[0]);
	        exit(2);
	}
	sgtty.sg_flags = RAW | ANYP;
	sgtty.sg_erase = sgtty.sg_kill = 0377;
	sgtty.sg_ispeed = sgtty.sg_ospeed = speed;
	if (ioctl(fd, TIOCSETP, &sgtty) < 0) {
	        perror("ioctl (TIOCSETP)");
		syslog(LOG_ERR, "%s: ioctl (TIOCSETP): %m\n",
		    argv[0]);
	        if (first) 
			exit(2);
		else {
			sleep(wait_time * tries);
			goto restart;
		}
	}
#endif
	sleep(2);		/* wait for flakey line to settle */
	if (hup)
		goto restart;

	wfd = fdopen(fd, "w+");
	if (wfd == NULL) {
		syslog(LOG_ERR, "can't fdopen slip line\n");
		exit(10);
	}
	setbuf(wfd, (char *)0);
	if (dialerstring) {
		printd(", send dialstring");
		fprintf(wfd, "%s\r", dialerstring);
	} else
		putc('\r', wfd);
	printd("\n");

	/*
	 * Log in
	 */
	printd("look for login: ");
	for (;;) {
		if (getline(buf, BUFSIZ, fd) == 0 || hup) {
			sleep(wait_time * tries);
			goto restart;
		}
	        if (bcmp(&buf[1], "ogin:", 5) == 0) {
	                fprintf(wfd, "%s\r", argv[1]);
			printd("Sent login: %s\n", argv[1]);
	                continue;
	        }
	        if (bcmp(&buf[1], "assword:", 8) == 0) {
	                fprintf(wfd, "%s\r", argv[2]);
			printd("Sent password: %s\n", argv[2]);
	                break;
	        }
	}
	
	/*
	 * Security hack.  Do not want private information such as the
	 * password and possible phone number to be left around.
	 * So we clobber the arguments.
	 */
	for (ap = argv - optind + 1; ap < argv + 3; ap++)
		for (cp = *ap; *cp != 0; cp++)
			*cp = '\0';

	/*
	 * Attach
	 */
	printd("setd");
	disc = SLIPDISC;
	if (ioctl(fd, TIOCSETD, &disc) < 0) {
	        perror("ioctl(TIOCSETD)");
		syslog(LOG_ERR, "%s: ioctl (TIOCSETD SLIP): %m\n",
		    argv[0]);
	        exit(1);
	}
	if (first && debug == 0) {
		close(0);
		close(1);
		close(2);
		(void) open("/dev/null", O_RDWR);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
	}
	(void) system("ifconfig sl0 up");
	printd(", ready\n");
	if (!first)
		syslog(LOG_INFO, "reconnected (%d tries).\n", tries);
	first = 0;
	tries = 0;
	logged_in = 1;
	while (hup == 0) {
		sigpause(0L);
		printd("sigpause return\n");
	}
	goto restart;
}

void
sighup()
{

	printd("hup\n");
	if (hup == 0 && logged_in)
		syslog(LOG_INFO, "hangup signal\n");
	hup = 1;
}

getline(buf, size, fd)
	char *buf;
	int size, fd;
{
	register int i;
	int ret;

	size--;
	for (i = 0; i < size; i++) {
		if (hup)
			return (0);
	        if ((ret = read(fd, &buf[i], 1)) == 1) {
	                buf[i] &= 0177;
	                if (buf[i] == '\r' || buf[i] == '\0')
	                        buf[i] = '\n';
	                if (buf[i] != '\n' && buf[i] != ':')
	                        continue;
	                buf[i + 1] = '\0';
			printd("Got %d: \"%s\"\n", i + 1, buf);
	                return (i+1);
	        }
		if (ret <= 0) {
			if (ret < 0)
				perror("getline: read");
			else
				fprintf(stderr, "read returned 0\n");
			buf[i] = '\0';
			printd("returning 0 after %d: \"%s\"\n", i, buf);
			return (0);
		}
	}
	return (0);
}

usage()
{
	(void)fprintf(stderr,
	    "usage: startslip [-d] [-b speed] [-s string] dev user passwd\n");
	exit(1);
}
