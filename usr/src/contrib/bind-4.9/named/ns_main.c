#if !defined(lint) && !defined(SABER)
static char sccsid[] = "@(#)ns_main.c	4.55 (Berkeley) 7/1/91";
static char rcsid[] = "$Id: ns_main.c,v 1.1 1993/06/01 02:33:47 vixie Exp vixie $";
#endif /* not lint */

/*
 * ++Copyright++ 1986, 1989, 1990
 * -
 * Copyright (c) 1986, 1989, 1990 Regents of the University of California.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
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

#if !defined(lint) && !defined(SABER)
char copyright[] =
"@(#) Copyright (c) 1986, 1989, 1990 The Regents of the University of California.\n\
 portions Copyright (c) 1993 Digital Equipment Corporation\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * Internet Name server (see rfc883 & others).
 */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#if !defined(SYSV) && defined(XXX)
#include <sys/wait.h>
#endif /* !SYSV */
#include <sys/resource.h>
#include <sys/ioctl.h>
#if defined(__osf__)
#define _SOCKADDR_LEN		/* XXX - should be in portability.h but that
				 * would need to be included before socket.h
				 */
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <resolv.h>
#include "../conf/portability.h"
#include "../conf/options.h"
#include "ns.h"
#include "db.h"
#include "pathnames.h"

#undef nsaddr

#ifdef BOOTFILE 			/* default boot file */
char	*bootfile = BOOTFILE;
#else
char	*bootfile = _PATH_BOOT;
#endif

#ifdef DEBUGFILE 			/* default debug output file */
char	*debugfile = DEBUGFILE;
#else
char	*debugfile = _PATH_DEBUG;
#endif

#ifdef WANT_PIDFILE
#ifdef PIDFILE 				/* file to store current named PID */
char	*PidFile = PIDFILE;
#else
char	*PidFile = _PATH_PIDFILE;
#endif
#endif /*WANT_PIDFILE*/

#ifndef FD_SET
#define	NFDBITS		32
#define	FD_SETSIZE	32
#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#endif

FILE	*fp;  				/* file descriptor for pid file */

#ifdef DEBUG
FILE	*ddt;
#endif

int	debug = 0;			/* debugging flag */
int	ds;				/* datagram socket */
int	vs;				/* listening TCP socket */
int	needreload = 0;			/* received SIGHUP, need to reload db */
int	needmaint = 0;			/* need to call ns_maint()*/
int	needzoneload = 0;		/* need to reload secondary zone(s) */
int     needToDoadump = 0;              /* need to dump database */
int     needToChkpt = 0;	        /* need to checkpoint cache */
int	needStatsDump = 0;		/* need to dump statistics */
#ifdef ALLOW_UPDATES
int     needToExit = 0;                 /* need to exit (may need to doadump
					 * first, if database has changed since
					 * it was last dumped/booted). Gets
					 * set by shutdown signal handler
					 *  (onintr)
					 */
#endif /* ALLOW_UPDATES */

#ifdef QRYLOG
int qrylog = 0;
#endif /*QRYLOG*/

int	priming = 0;			/* is cache being primed */

#ifdef SO_RCVBUF
int	rbufsize = 8 * 1024;		/* UDP recive buffer size */
#endif

struct	qstream *streamq = QSTREAM_NULL; /* list of open streams */
struct	qdatagram *datagramq = QDATAGRAM_NULL; /* list of datagram interfaces */
static	struct sockaddr_in nsaddr;
struct	timeval tt;

/*
 * We keep a list of favored networks headed by nettab.
 * There are three (possibly empty) parts to this list, in this order:
 *	1. directly attached (sub)nets.
 *	2. logical networks for directly attached subnetted networks.
 *	3. networks from the sort list.
 * The value (*elocal) points at the first entry in the second part of the
 * list, if any, while (*enettab) points at the first entry in the sort list.
 */
struct	netinfo *nettab = NULL;
struct	netinfo **elocal = &nettab;
struct	netinfo **enettab = &nettab;
#ifdef XFRNETS
struct	netinfo *xfrnets = NULL;
#endif
#ifdef BOGUSNS
struct	netinfo *boglist = NULL;	/* list of bogus nameservers */
#endif
struct	netinfo netloop;
struct	netinfo *findnetinfo();
u_int32_t	net_mask();
u_short	ns_port;			/* port to which we send queries */
static	u_short	local_ns_port;		/* port on which we service queries */
struct	sockaddr_in from_addr;		/* Source addr of last packet */
int	from_len;			/* Source addr size of last packet */
time_t	boottime, resettime;		/* Used by ns_stats */
static	fd_set	mask;			/* select mask of open descriptors */

char		**Argv = NULL;		/* pointer to argument vector */
char		*LastArg = NULL;	/* end of argv */

extern void ns_req();

void	sqrm(), sqflush(), sq_query(), sq_done(),
	setproctitle(), usage(), getnetconf(),
	opensocket(), setdebug();

#ifdef DEBUG
void printnetinfo();
#endif

extern int getdtablesize();

/*ARGSUSED*/
void
main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	register int n, udpcnt;
	register char *arg;
	register struct qstream *sp;
	register struct qdatagram *dqp;
	struct qstream *nextsp;
	int nfds;
	int on = 1;
	int rfd, size;
	u_int32_t lasttime, maxctime;
	u_char buf[BUFSIZ];
#ifndef SYSV
	struct sigvec vec;
#endif

	fd_set tmpmask;

	struct timeval t, *tp;
	struct qstream *candidate = QSTREAM_NULL;
	extern SIG_FN onintr(), maint_alarm(), endxfer();
	extern SIG_FN setdumpflg(), onhup();
	extern SIG_FN setIncrDbgFlg(), setNoDbgFlg(), sigprof();
	extern SIG_FN setchkptflg(), setstatsflg();
#if defined(QRYLOG) && defined(SIGWINCH)
	extern SIG_FN setQrylogFlg();
#endif
	extern int loadxfer();
	extern struct qstream *sqadd();
	extern char Version[];
	char **argp;
#ifdef PID_FIX
	char oldpid[10];
#endif

	local_ns_port = ns_port = htons(NAMESERVER_PORT);

	/*
	**  Save start and extent of argv for setproctitle.
	*/

	Argv = argp = argv;
	while (*argp)
		argp++;
	LastArg = argp[-1] + strlen(argp[-1]);

	(void) umask(022);
	while (--argc > 0) {
		arg = *++argv;
		if (*arg == '-') {
			while (*++arg)
				switch (*arg) {
				case 'b':
					if (--argc <= 0)
						usage();
					bootfile = savestr(*++argv);
					break;

  				case 'd':
 					++argv;

 					if (*argv != 0) {
 					    if (**argv == '-') {
 						argv--;
 						break;
 					    }
 					    debug = atoi(*argv);
 					    --argc;
 					}
					if (debug <= 0)
						debug = 1;
					setdebug(1);
					break;

				case 'p':
					/* use nonstandard port number.
					 * usage: -p remote/local
					 * remote is the port number to which
					 * we send queries.  local is the port
					 * on which we listen for queries.
					 * local defaults to same as remote.
					 */
					if (--argc <= 0)
						usage();
					ns_port = htons((u_short)atoi(*++argv));
					{
					    char *p = strchr(*argv, '/');
					    if (p) {
						local_ns_port =
						    htons((u_short)atoi(p+1));
					    } else {
						local_ns_port = ns_port;
					    }
					}
					break;

#ifdef QRYLOG
				case 'q':
					qrylog = 1;
					break;
#endif

				default:
					usage();
				}
		} else
			bootfile = savestr(*argv);
	}

	if (!debug)
		for (n = getdtablesize() - 1; n > 2; n--)
			(void) close(n);	/* don't use my_close() here */
#ifdef DEBUG
	else {
		fprintf(ddt,"Debug turned ON, Level %d\n",debug);
		fprintf(ddt,"Version = %s\t",Version);
		fprintf(ddt,"bootfile = %s\n",bootfile);
	}		
#endif

#ifdef LOG_DAEMON
	openlog("named", LOG_PID|LOG_CONS|LOG_NDELAY, LOG_DAEMON);
#else
	openlog("named", LOG_PID);
#endif

#ifdef WANT_PIDFILE
	/* tuck my process id away */
#ifdef PID_FIX
	fp = fopen(PidFile, "r+");
	if (fp != NULL) {
		(void) fgets(oldpid, sizeof(oldpid), fp);
		(void) rewind(fp);
		fprintf(fp, "%d\n", getpid());
		(void) my_fclose(fp);
	}
#else /*PID_FIX*/
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		(void) my_fclose(fp);
	}
#endif /*PID_FIX*/
#endif /*WANT_PIDFILE*/

	syslog(LOG_NOTICE, "starting");

	_res.options &= ~(RES_DEFNAMES | RES_DNSRCH | RES_RECURSE);

	nsaddr.sin_family = AF_INET;
	nsaddr.sin_addr.s_addr = INADDR_ANY;
	nsaddr.sin_port = local_ns_port;

	/*
	** Open stream port.
	*/
	for (n = 0; ; n++) {
		if ((vs = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			syslog(LOG_ERR, "socket(SOCK_STREAM): %m");
			exit(1);
		}	
		if (setsockopt(vs, SOL_SOCKET, SO_REUSEADDR, (char *)&on,
			sizeof(on)) != 0)
		{
			syslog(LOG_ERR, "setsockopt(vs, reuseaddr): %m");
			(void) my_close(vs);
			continue;
		}
		if (bind(vs, (struct sockaddr *)&nsaddr, sizeof(nsaddr)) == 0)
			break;

		if (errno != EADDRINUSE || n > 4) {
			if (errno == EADDRINUSE) {
				syslog(LOG_ERR,
				 "There may be a name server already running");
				syslog(LOG_ERR, "exiting");
			} else {
				syslog(LOG_ERR, "bind(vs, %s[%d]): %m",
					inet_ntoa(nsaddr.sin_addr),
					ntohs(nsaddr.sin_port));
			}
#if defined(WANT_PIDFILE) && defined(PID_FIX)
			/* put old pid back */
			fp = fopen(PidFile, "w");
			if (fp != NULL) {
				fprintf(fp, "%s", oldpid);
				(void) my_fclose(fp);
			}
#endif /*WANT_PIDFILE && PID_FIX*/
			exit(1);
		} else {	/* Retry opening the socket a few times */
			my_close(vs);
			sleep(1);
		}
	}
	if (listen(vs, 5) != 0) {
		syslog(LOG_ERR, "listen(vs, 5): %m");
		exit(1);
	}

	/*
	 * Get list of local addresses and set up datagram sockets.
	 */
	getnetconf();

	/*
	** Initialize and load database.
	*/
	gettime(&tt);
	buildservicelist();
	buildprotolist();
	ns_init(bootfile);
#ifdef DEBUG
	if (debug) {
		fprintf(ddt, "Network and sort list:\n");
		printnetinfo(nettab);
	}
#endif

	time(&boottime);
	resettime = boottime;

	(void) signal(SIGHUP, onhup);
#if defined(SIGXFSZ)
	(void) signal(SIGXFSZ, onhup);	/* wierd DEC Hesiodism, harmless */
#endif /*SIGXFSZ*/
#if defined(SYSV)
	(void) signal(SIGCLD, endxfer);
	(void) signal(SIGALRM, maint_alarm);
#else
	bzero((char *)&vec, sizeof(vec));
	vec.sv_handler = maint_alarm;
	vec.sv_mask = sigmask(SIGCHLD);
	(void) sigvec(SIGALRM, &vec, (struct sigvec *)NULL);

	vec.sv_handler = endxfer;
	vec.sv_mask = sigmask(SIGALRM);
	(void) sigvec(SIGCHLD, &vec, (struct sigvec *)NULL);
#endif /* SYSV */
	(void) signal(SIGPIPE, SIG_IGN);
	(void) signal(SIGSYS, sigprof);
	(void) signal(SIGINT, setdumpflg);
	(void) signal(SIGQUIT, setchkptflg);
	(void) signal(SIGIOT, setstatsflg);

#ifdef ALLOW_UPDATES
        /* Catch SIGTERM so we can dump the database upon shutdown if it
           has changed since it was last dumped/booted */
        (void) signal(SIGTERM, onintr);
#endif

#if defined(SIGUSR1) && defined(SIGUSR2)
	(void) signal(SIGUSR1, setIncrDbgFlg);
	(void) signal(SIGUSR2, setNoDbgFlg);
#else /* SIGUSR1&&SIGUSR2 */
	(void) signal(SIGEMT, setIncrDbgFlg);
	(void) signal(SIGFPE, setNoDbgFlg);
#endif /* SIGUSR1&&SIGUSR2 */

#if defined(SIGWINCH) && defined(QRYLOG)
	(void) signal(SIGWINCH, setQrylogFlg);
#endif

#ifdef DEBUG
	if (debug) {
		fprintf(ddt,"database initialized\n");
	}
#endif

	t.tv_usec = 0;

	/*
	 * Fork and go into background now that
	 * we've done any slow initialization
	 * and are ready to answer queries.
	 */
#ifdef USE_SETSID
	if (!debug || !isatty(0)) {
		if (fork() > 0)
			exit(0);
		setsid();
		if (!debug) {
			n = open(_PATH_DEVNULL, O_RDONLY);
			(void) dup2(n, 0);
			(void) dup2(n, 1);
			(void) dup2(n, 2);
			if (n > 2)
				(void) my_close(n);
		}
	}
#else
	if (!debug) {
#ifdef HAVE_DAEMON
		daemon(1, 0);
#else
		switch (fork()) {
		case -1:
			syslog(LOG_ERR, "fork: %m");
			exit(1);
			/*FALLTHROUGH*/
		case 0:
			/* child */
			break;
		default:
			/* parent */
			exit(0);
		}
		n = open(_PATH_DEVNULL, O_RDONLY);
		(void) dup2(n, 0);
		(void) dup2(n, 1);
		(void) dup2(n, 2);
		if (n > 2)
			(void) my_close(n);
#ifdef SYSV
		setpgrp();
#else
		{
			struct itimerval ival;

			/*
			 * The open below may hang on pseudo ttys if the person
			 * who starts named logs out before this point.
			 *
			 * needmaint may get set inapropriately if the open
			 * hangs, but all that will happen is we will see that
			 * no maintenance is required.
			 */
			bzero((char *)&ival, sizeof(ival));
			ival.it_value.tv_sec = 120;
			(void) setitimer(ITIMER_REAL, &ival,
				    (struct itimerval *)NULL);
			n = open(_PATH_TTY, O_RDWR);
			ival.it_value.tv_sec = 0;
			(void) setitimer(ITIMER_REAL, &ival,
				    (struct itimerval *)NULL);
			if (n > 0) {
				(void) ioctl(n, TIOCNOTTY, (char *)NULL);
				(void) my_close(n);
			}
		}
#endif /* SYSV */
#endif /* HAVE_DAEMON */
	}
#endif /* USE_SETSID */
#ifdef WANT_PIDFILE
	/* tuck my process id away again */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		(void) my_fclose(fp);
	}
#endif

#ifdef DEBUG
	if (debug)
		fprintf(ddt,"Ready to answer queries.\n");
#endif
	syslog(LOG_NOTICE, "Ready to answer queries.\n");
	prime_cache();
	nfds = getdtablesize();       /* get the number of file descriptors */
	if (nfds > FD_SETSIZE) {
		nfds = FD_SETSIZE;	/* Bulletproofing */
		syslog(LOG_ERR, "Return from getdtablesize() > FD_SETSIZE");
#ifdef DEBUG
		if (debug)
		      fprintf(ddt,"Return from getdtablesize() > FD_SETSIZE\n");
#endif
	}
	FD_ZERO(&mask);
	FD_SET(vs, &mask);
	for (dqp = datagramq; dqp != QDATAGRAM_NULL; dqp = dqp->dq_next)
		FD_SET(dqp->dq_dfd, &mask);
	for (;;) {
#ifdef DEBUG
		if (ddt && debug == 0) {
			fprintf(ddt,"Debug turned OFF\n");
			(void) my_fclose(ddt);
			ddt = 0;
		}
#endif
#ifdef ALLOW_UPDATES
                if (needToExit) {
			struct zoneinfo *zp;
			sigblock(~0);   /*
					 * Block all blockable signals
					 * to ensure a consistant
					 * state during final dump
					 */
#ifdef DEBUG
			if (debug)
				fprintf(ddt, "Received shutdown signal\n");                     
#endif
			for (zp = zones; zp < &zones[nzones]; zp++) {
				if (zp->z_state & Z_CHANGED)
					zonedump(zp);
                        }
                        exit(0);
                }
#endif /* ALLOW_UPDATES */
		if (needreload) {
			needreload = 0;
			db_reload();
		}
#ifdef STATS
		if (needStatsDump) {
			needStatsDump = 0;
			ns_stats();
		}
#endif /* STATS */
		if (needzoneload) {
			needzoneload = 0;
			loadxfer();
		}
		if (needmaint) {
                        needmaint = 0;
                        ns_maint();
                }
	        if(needToChkpt) {
                        needToChkpt = 0;
                        doachkpt();
	        }
                if(needToDoadump) {
                        needToDoadump = 0;
                        doadump();
                }
		/*
		** Wait until a query arrives
		*/
		if (retryqp != NULL) {
			gettime(&tt);
			/*
			** The tv_sec field might be unsigned 
			** and thus cannot be negative.
			*/
			if ((int32_t) retryqp->q_time <= tt.tv_sec) {
				retry(retryqp);
				continue;
			}
			t.tv_sec = (int32_t) retryqp->q_time - tt.tv_sec;
			tp = &t;
		} else
			tp = NULL;
		tmpmask = mask;
		n = select(nfds, &tmpmask, (fd_set *)NULL, (fd_set *)NULL, tp);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "select: %m");
#ifdef DEBUG
			if (debug)
				fprintf(ddt,"select error\n");
#endif
			;
		}
		if (n == 0)
			continue;

		for (dqp = datagramq; dqp != QDATAGRAM_NULL;
		    dqp = dqp->dq_next) {
		    if (FD_ISSET(dqp->dq_dfd, &tmpmask))
		        for(udpcnt = 0; udpcnt < 25; udpcnt++) {
			    from_len = sizeof(from_addr);
			    if ((n = recvfrom(dqp->dq_dfd, buf, sizeof(buf), 0,
				(struct sockaddr *)&from_addr, &from_len)) < 0)
			    {
				if ((n == -1) && (errno == PORT_WOULDBLK))
					break;
				syslog(LOG_WARNING, "recvfrom: %m");
				break;
			    }
			    if (n == 0)
				break;
#ifdef STATS
			    stats[S_INPKTS].cnt++;
#endif
#ifdef DEBUG
			    if (debug)
				fprintf(ddt,
				 "\ndatagram from %s port %d, fd %d, len %d\n",
				    inet_ntoa(from_addr.sin_addr),
				    ntohs(from_addr.sin_port), dqp->dq_dfd, n);
			    if (debug >= 10)
				fp_query((char *)buf, ddt);
#endif
			    /*
			     * Consult database to get the answer.
			     */
			    gettime(&tt);
			    ns_req(buf, n, PACKETSZ, QSTREAM_NULL, &from_addr,
				    dqp->dq_dfd);
		        }
		}
		/*
		** Process stream connection
		*/
		if (FD_ISSET(vs, &tmpmask)) {
			from_len = sizeof(from_addr);
			rfd = accept(vs,
				     (struct sockaddr *)&from_addr,
				     &from_len);
			gettime(&tt);
			if (rfd < 0 && errno == EMFILE && streamq != NULL) {
				maxctime = 0;
				candidate = QSTREAM_NULL;
				for (sp = streamq; sp != QSTREAM_NULL;
				   sp = nextsp) {
					nextsp = sp->s_next;
					if (sp->s_refcnt != 0)
					    continue;
					lasttime = tt.tv_sec - sp->s_time;
					if (lasttime >= 900)
					    sqrm(sp);
					else if (lasttime > maxctime) {
					    candidate = sp;
					    maxctime = lasttime;
					}
				}
				rfd = accept(vs,
					     (struct sockaddr *)&from_addr,
					     &from_len);
				if ((rfd < 0) && (errno == EMFILE) &&
				    candidate != QSTREAM_NULL) {
					sqrm(candidate);
					rfd = accept(vs,
						     (struct sockaddr *)
							&from_addr,
						     &from_len);
				}
			}
			if (rfd < 0) {
				syslog(LOG_WARNING, "accept: %m");
				continue;
			}
			if (fcntl(rfd, F_SETFL, PORT_NONBLOCK) != 0) {
				syslog(LOG_ERR, "fcntl(rfd, non-blocking): %m");
				(void) my_close(rfd);
				continue;
			}
			if (setsockopt(rfd, SOL_SOCKET, SO_KEEPALIVE,
				(char *)&on, sizeof(on)) != 0)
			{
				syslog(LOG_ERR, "setsockopt(rfd, keepalive): %m");
				(void) my_close(rfd);
				continue;
			}
			if ((sp = sqadd()) == QSTREAM_NULL) {
				(void) my_close(rfd);
				continue;
			}
			sp->s_rfd = rfd;	/* stream file descriptor */
			sp->s_size = -1;	/* amount of data to receive */
			gettime(&tt);
			sp->s_time = tt.tv_sec;	/* last transaction time */
			sp->s_from = from_addr;	/* address to respond to */
			sp->s_bufp = (u_char *)&sp->s_tempsize;
			FD_SET(rfd, &mask);
			FD_SET(rfd, &tmpmask);
#ifdef DEBUG
			if (debug) {
				fprintf(ddt,
				   "\nTCP connection from %s port %d (fd %d)\n",
					inet_ntoa(sp->s_from.sin_addr),
					ntohs(sp->s_from.sin_port), rfd);
			}
#endif
		}
#ifdef DEBUG
		if (debug > 2 && streamq)
			fprintf(ddt,"streamq  = x%x\n",streamq);
#endif
		for (sp = streamq;  sp != QSTREAM_NULL;  sp = nextsp) {
			nextsp = sp->s_next;
			if (!FD_ISSET(sp->s_rfd, &tmpmask))
				continue;
#ifdef DEBUG
			if (debug > 5) {
				fprintf(ddt,
					"sp x%x rfd %d size %d time %d ",
					sp, sp->s_rfd, sp->s_size,
					sp->s_time
				);
				fprintf(ddt," next x%x \n", sp->s_next);
				fprintf(ddt,"\tbufsize %d",sp->s_bufsize);
				fprintf(ddt," buf x%x ",sp->s_buf);
				fprintf(ddt," bufp x%x\n",sp->s_bufp);
			}
#endif /* DEBUG */
			if (sp->s_size < 0) {
				size = sizeof(u_short)
				    - (sp->s_bufp - (u_char *)&sp->s_tempsize);
			        while (size > 0 &&
			           (n = read(sp->s_rfd, sp->s_bufp, size)) > 0
				       ) {
					sp->s_bufp += n;
					size -= n;
			        }
				if ((n < 0) && (errno == PORT_WOULDBLK))
					continue;
				if (n <= 0) {
					sqrm(sp);
					continue;
			        }
			        if ((sp->s_bufp - (u_char *)&sp->s_tempsize) ==
					sizeof(u_short)) {
					sp->s_size = htons(sp->s_tempsize);
					if (sp->s_bufsize == 0) {
					    if ( (sp->s_buf = (u_char *)
						  malloc(BUFSIZ))
						== NULL) {
						    sp->s_buf = buf;
						    sp->s_size  = sizeof(buf);
					    } else {
						    sp->s_bufsize = BUFSIZ;
					    }
					}
					if (sp->s_size > sp->s_bufsize &&
					    sp->s_bufsize != 0
					) {
					    sp->s_buf = (u_char *)
						realloc((char *)sp->s_buf,
							(unsigned)sp->s_size);
					    if (sp->s_buf == NULL) {
						sp->s_buf = buf;
						sp->s_bufsize = 0;
						sp->s_size  = sizeof(buf);
					    } else {
						sp->s_bufsize = sp->s_size;
					    }
					}
					sp->s_bufp = sp->s_buf;	
				}
			}
			gettime(&tt);
			sp->s_time = tt.tv_sec;
			while (sp->s_size > 0 &&
			      (n = read(sp->s_rfd,
					sp->s_bufp,
					sp->s_size)
			       ) > 0
			) {
				sp->s_bufp += n;
				sp->s_size -= n;
			}
			/*
			 * we don't have enough memory for the query.
			 * if we have a query id, then we will send an
			 * error back to the user.
			 */
			if (sp->s_bufsize == 0 &&
			    (sp->s_bufp - sp->s_buf > sizeof(u_short))) {
				HEADER *hp;

				hp = (HEADER *)sp->s_buf;
				hp->qr = 1;
				hp->ra = 1;
				hp->ancount = 0;
				hp->qdcount = 0;
				hp->nscount = 0;
				hp->arcount = 0;
				hp->rcode = SERVFAIL;
				(void) writemsg(sp->s_rfd, sp->s_buf,
						sizeof(HEADER));
				continue;
			}
			if ((n == -1) && (errno == PORT_WOULDBLK))
				continue;
			if (n <= 0) {
				sqrm(sp);
				continue;
			}
			/*
			 * Consult database to get the answer.
			 */
			if (sp->s_size == 0) {
				sq_query(sp);
				ns_req(sp->s_buf,
				       sp->s_bufp - sp->s_buf,
				       sp->s_bufsize, sp,
				       &sp->s_from, -1);
				/* ns_req() can call sqrm() - check for it */
				if (sq_here(sp)) {
					sp->s_bufp = (u_char *)&sp->s_tempsize;
					sp->s_size = -1;
				}
				continue;
			}
		}
	}
	/* NOTREACHED */
}

void
usage()
{
	fprintf(stderr, "Usage: named [-d #] [-q] [-p port[/localport]] [{-b} bootfile]\n");
	exit(1);
}

void
getnetconf()
{
	register struct netinfo *ntp;
	struct netinfo *ontp;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	struct	qdatagram *dqp;
	static int first = 1;
	char buf[BUFSIZ], *cp, *cplim;
	u_int32_t nm;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(vs, SIOCGIFCONF, (char *)&ifc) < 0) {
		syslog(LOG_ERR, "get interface configuration: %m - exiting");
		exit(1);
	}
	ntp = NULL;
#if defined(AF_LINK) && !defined(RISCOS_BSD)
#define max(a, b) (a > b ? a : b)
#define size(p)	max((p).sa_len, sizeof(p))
#else
#define size(p) (sizeof (p))
#endif
	cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim;
			cp += sizeof (ifr->ifr_name) + size(ifr->ifr_addr)) {
#undef size
		ifr = (struct ifreq *)cp;
		if (ifr->ifr_addr.sa_family != AF_INET ||
		   ((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr == 0)
			continue;
		ifreq = *ifr;
		/*
		 * Don't test IFF_UP, packets may still be received at this
		 * address if any other interface is up.
		 */
		if (ioctl(vs, SIOCGIFADDR, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "get interface addr: %m");
			continue;
		}
		/* build datagram queue */
		/* 
		 * look for an already existing source interface address.
		 * This happens mostly when reinitializing.  Also, if
		 * the machine has multiple point to point interfaces, then 
		 * the local address may appear more than once.
		 */
		if (aIsUs(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr)) {
#ifdef DEBUG
			if (debug)
				fprintf(ddt,
					"dup interface address %s on %s\n",
					inet_ntoa(((struct sockaddr_in *)
						   &ifreq.ifr_addr)->sin_addr),
					ifreq.ifr_name);
#endif		      
			continue;
		}

		/*
		 * Skip over address 0.0.0.0 since this will conflict
		 * with binding to wildcard address later.  Interfaces
		 * which are not completely configured can have this addr.
		 */
		if (((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr
		        == inet_addr("0.0.0.0")) {	/* XXX */
#ifdef DEBUG
			  if (debug)
			      fprintf(ddt, "skipping address 0.0.0.0 on %s\n",
				    ifreq.ifr_name);
#endif		      
			  continue;
		}
		if ((dqp = (struct qdatagram *)calloc(1,
		    sizeof(struct qdatagram))) == NULL) {
#ifdef DEBUG
			if (debug >= 5)
				fprintf(ddt,"getnetconf: malloc error\n");
#endif
			syslog(LOG_ERR, "getnetconf: Out Of Memory");
			exit(12);
		}
		dqp->dq_next = datagramq;
		datagramq = dqp;
		dqp->dq_addr = ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
		opensocket(dqp);

		/*
		 * Add interface to list of directly-attached (sub)nets
		 * for use in sorting addresses.
		 */
		if (ntp == NULL)
			ntp = (struct netinfo *)malloc(sizeof(struct netinfo));
		ntp->my_addr = 
		    ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
#ifdef SIOCGIFNETMASK
		if (ioctl(vs, SIOCGIFNETMASK, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "get netmask: %m");
			ntp->mask = net_mask(ntp->my_addr);
		} else
			ntp->mask = ((struct sockaddr_in *)
			    &ifreq.ifr_addr)->sin_addr.s_addr;
#else
		/* 4.2 does not support subnets */
		ntp->mask = net_mask(ntp->my_addr);
#endif
		if (ioctl(vs, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "get interface flags: %m");
			continue;
		}
#ifdef IFF_LOOPBACK
		if (ifreq.ifr_flags & IFF_LOOPBACK)
#else
		/* test against 127.0.0.1 (yuck!!) */
		if (ntp->my_addr.s_addr == htonl(0x7F000001))
#endif
		{
		    if (netloop.my_addr.s_addr == 0) {
			netloop.my_addr = ntp->my_addr;
			netloop.mask = 0xffffffff;
			netloop.net = ntp->my_addr.s_addr;
#ifdef DEBUG
			if (debug) 
			    fprintf(ddt,"loopback address: x%lx\n",
				    netloop.my_addr.s_addr);
#endif
		    }
		    continue;
		} else if ((ifreq.ifr_flags & IFF_POINTOPOINT)) {
			if (ioctl(vs, SIOCGIFDSTADDR, (char *)&ifreq) < 0) {
		    	    syslog(LOG_ERR, "get dst addr: %m");
		            continue;
			}
			ntp->mask = 0xffffffff;
			ntp->net = ((struct sockaddr_in *)
		    	    &ifreq.ifr_addr)->sin_addr.s_addr;
		} else {
			ntp->net = ntp->mask & ntp->my_addr.s_addr;
		}
		/*
		 * Place on end of list of locally-attached (sub)nets,
		 * but before logical nets for subnetted nets.
		 */
		ntp->next = *elocal;
		*elocal = ntp;
		if (elocal == enettab)
		    enettab = &ntp->next;
		elocal = &ntp->next;
		ntp = NULL;
	}
	if (ntp)
		(void) free((char *)ntp);

	/*
	 * Create separate qdatagram structure for socket
	 * wildcard address.
	 */
	if (first) {
		if ((dqp = (struct qdatagram *)calloc(1, sizeof(*dqp))) == NULL) {
#ifdef DEBUG
			if (debug >= 5)
				fprintf(ddt,"getnetconf: malloc error\n");
#endif
			syslog(LOG_ERR, "getnetconf: Out Of Memory");
			exit(12);
		}
		dqp->dq_next = datagramq;
		datagramq = dqp;
		dqp->dq_addr.s_addr = INADDR_ANY;
		opensocket(dqp);
		ds = dqp->dq_dfd;	/* used externally */
	}

	/*
	 * Compute logical networks to which we're connected
	 * based on attached subnets;
	 * used for sorting based on network configuration.
	 */
	for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
		nm = net_mask(ntp->my_addr);
		if (nm != ntp->mask) {
			if (findnetinfo(ntp->my_addr))
				continue;
			ontp = (struct netinfo *)malloc(sizeof(struct netinfo));
			if (ontp == NULL) {
#ifdef DEBUG
				if (debug >= 5)
				    fprintf(ddt,"getnetconf: malloc error\n");
#endif
				syslog(LOG_ERR, "getnetconf: Out Of Memory");
				exit(12);
			}
			ontp->my_addr = ntp->my_addr;
			ontp->mask = nm;
			ontp->net = ontp->my_addr.s_addr & nm;
			ontp->next = *enettab;
			*enettab = ontp;
			enettab = &ontp->next;
		}
	}
	first = 0;
}

/*
 * Find netinfo structure for logical network implied by address "addr",
 * if it's on list of local/favored networks.
 */
struct netinfo *
findnetinfo(addr)
	struct in_addr addr;
{
	register struct netinfo *ntp;
	u_int32_t net, mask;

	mask = net_mask(addr);
	net = addr.s_addr & mask;
	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->net == net && ntp->mask == mask)
			return (ntp);
	return ((struct netinfo *) NULL);
}

#ifdef DEBUG
void
printnetinfo(ntp)
	register struct netinfo *ntp;
{
	for ( ; ntp != NULL; ntp = ntp->next) {
		fprintf(ddt,"net x%lx mask x%lx", ntp->net, ntp->mask);
		fprintf(ddt," my_addr x%lx", ntp->my_addr.s_addr);
		fprintf(ddt," %s\n", inet_ntoa(ntp->my_addr));
	}
}
#endif

void
opensocket(dqp)
	register struct qdatagram *dqp;
{
	int on = 1;

	/*
	 * Open datagram sockets bound to interface address.
	 */
	if ((dqp->dq_dfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "socket(SOCK_DGRAM): %m - exiting");
		exit(1);
	}	
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"dqp->dq_addr %s d_dfd %d\n",
		    inet_ntoa(dqp->dq_addr), dqp->dq_dfd);
#endif
	if (setsockopt(dqp->dq_dfd, SOL_SOCKET, SO_REUSEADDR,
	    (char *)&on, sizeof(on)) != 0)
	{
		syslog(LOG_ERR, "setsockopt(dqp->dq_dfd, reuseaddr): %m");
		/* XXX press on regardless, this is not too serious. */
	}
#ifdef SO_RCVBUF
	(void) setsockopt(dqp->dq_dfd, SOL_SOCKET, SO_RCVBUF,
	    (char *)&rbufsize, sizeof(rbufsize));
#endif /* SO_RCVBUF */
	if (fcntl(dqp->dq_dfd, F_SETFL, PORT_NONBLOCK) != 0) {
		syslog(LOG_ERR, "fcntl(dqp->dq_dfd, non-blocking): %m");
		/* XXX press on regardless, but this really is a problem. */
	}
	/*
	 *   NOTE: Some versions of SunOS have problems with the following
	 *   call to bind.  Bind still seems to function on these systems
	 *   if you comment out the exit inside the if.  This may cause
	 *   Suns with multiple interfaces to reply strangely.
	 */
	nsaddr.sin_addr = dqp->dq_addr;
	if (bind(dqp->dq_dfd, (struct sockaddr *)&nsaddr, sizeof(nsaddr))) {
		syslog(LOG_ERR, "bind(dfd %d, %s[%d]): %m - exiting",
			dqp->dq_dfd, inet_ntoa(nsaddr.sin_addr),
			ntohs(nsaddr.sin_port));
#if !defined(sun)
		exit(1);
#endif
	}
}

/*
** Set flag saying to reload database upon receiving SIGHUP.
** Must make sure that someone isn't walking through a data
** structure at the time.
*/

SIG_FN
onhup()
{
#if defined(SYSV)
	(void)signal(SIGHUP, onhup);
#endif /* SYSV */
	needreload = 1;
}

/*
** Set flag saying to call ns_maint()
** Must make sure that someone isn't walking through a data
** structure at the time.
*/

SIG_FN
maint_alarm()
{
#if defined(SYSV)
	(void)signal(SIGALRM, maint_alarm);
#endif /* SYSV */
	needmaint = 1;
 }


#ifdef ALLOW_UPDATES
/*
 * Signal handler to schedule shutdown.  Just set flag, to ensure a consistent
 * state during dump.
 */
SIG_FN
onintr()
{
        needToExit = 1;
}
#endif /* ALLOW_UPDATES */

/*
 * Signal handler to schedule a data base dump.  Do this instead of dumping the
 * data base immediately, to avoid seeing it in a possibly inconsistent state
 * (due to updates), and to avoid long disk I/O delays at signal-handler
 * level
 */
SIG_FN
setdumpflg()
{
#if defined(SYSV)
	(void)signal(SIGINT, setdumpflg);
#endif /* SYSV */
        needToDoadump = 1;
}

/*
** Turn on or off debuging by open or closeing the debug file
*/

void
setdebug(code)
	int code;
{
#if defined(lint) && !defined(DEBUG)
	code = code;
#endif
#ifdef DEBUG

	if (code) {
		ddt = freopen(debugfile, "w+", stderr);
		if ( ddt == NULL) {
			syslog(LOG_WARNING, "can't open debug file %s: %m",
			    debugfile);
			debug = 0;
		} else {
#if defined(SYSV)
			setvbuf(ddt, NULL, _IOLBF, BUFSIZ);
#else
			setlinebuf(ddt);
#endif
			(void) fcntl(fileno(ddt), F_SETFL, O_APPEND);
		}
	} else
		debug = 0;
		/* delay closing ddt, we might interrupt someone */
#endif
}

/*
** Catch a special signal and set debug level.
**
**  If debuging is off then turn on debuging else increment the level.
**
** Handy for looking in on long running name servers.
*/

SIG_FN
setIncrDbgFlg()
{
#if defined(SYSV)
	(void)signal(SIGUSR1, setIncrDbgFlg);
#endif /* SYSV */
#ifdef DEBUG
	if (debug == 0) {
		debug++;
		setdebug(1);
	}
	else {
		debug++;
	}
	fprintf(ddt,"Debug turned ON, Level %d\n",debug);
#endif
}

/*
** Catch a special signal to turn off debugging
*/

SIG_FN
setNoDbgFlg()
{
#if defined(SYSV)
	(void)signal(SIGUSR2, setNoDbgFlg);
#endif /* SYSV */
	setdebug(0);
}

#if defined(QRYLOG) && defined(SIGWINCH)
/*
** Set flag for query logging
*/
SIG_FN
setQrylogFlg()
{
#if defined(SYSV)
	(void)signal(SIGWINCH, setQrylogFlg);
#endif /* SYSV */
	qrylog = !qrylog;
	syslog(LOG_NOTICE, "query log %s\n", qrylog ?"on" :"off");
}
#endif /*QRYLOG && SIGWINCH*/

/*
** Set flag for statistics dump
*/
SIG_FN
setstatsflg()
{
#if defined(SYSV)
	(void)signal(SIGIOT, setstatsflg);
#endif /* SYSV */
	needStatsDump = 1;
}

SIG_FN
setchkptflg()
{
#if defined(SYSV)
	(void)signal(SIGQUIT, setchkptflg);
#endif /* SYSV */
	needToChkpt = 1;
}

/*
** Catch a special signal SIGSYS
**
**  this is setup to fork and exit to drop to /usr/tmp/gmon.out
**   and keep the server running
*/

SIG_FN
sigprof()
{
#if defined(SYSV)
	(void)signal(SIGSYS, sigprof);
#endif /* SYSV */
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"sigprof()\n");
#endif
	if (fork() == 0)
	{
		(void) chdir(_PATH_TMPDIR);
		exit(1);
	}
}

/*
** Routines for managing stream queue
*/

struct qstream *
sqadd()
{
	register struct qstream *sqp;

	if ((sqp = (struct qstream *)calloc(1, sizeof(struct qstream)))
	    == NULL ) {
#ifdef DEBUG
		if (debug >= 5)
			fprintf(ddt,"sqadd: malloc error\n");
#endif
		syslog(LOG_ERR, "sqadd: Out Of Memory");
		return(QSTREAM_NULL);
	}
#ifdef DEBUG
	if (debug > 3)
		fprintf(ddt,"sqadd(x%x)\n", sqp);
#endif

	sqp->s_next = streamq;
	streamq = sqp;
	return(sqp);
}

/* sqrm(qp)
 *	remove stream queue structure `qp'.
 *	no current queries may refer to this stream when it is removed.
 * side effects:
 *	memory is deallocated.  sockets are closed.  lists are relinked.
 */
void
sqrm(qp)
	register struct qstream *qp;
{
	register struct qstream *qsp;

#ifdef DEBUG
	if (debug > 1) {
		fprintf(ddt,"sqrm(%#x, %d ) rfcnt=%d\n",
		    qp, qp->s_rfd, qp->s_refcnt);
	}
#endif

	if (qp->s_bufsize != 0)
		free(qp->s_buf);
	FD_CLR(qp->s_rfd, &mask);
	(void) my_close(qp->s_rfd);
	if (qp == streamq) {
		streamq = qp->s_next;
	} else {
		for (qsp = streamq;
		     qsp && (qsp->s_next != qp);
		     qsp = qsp->s_next)
			;
		if (qsp) {
			qsp->s_next = qp->s_next;
		}
	}
	free((char *)qp);
}

/* sqflush()
 *	call sqrm() on all open streams
 * side effects:
 *	global list `streamq' becomes empty
 */
void
sqflush()
{
	register struct qstream *sp, *spnext;

	for (sp = streamq; sp != QSTREAM_NULL; sp = spnext) {
		spnext = sp->s_next;
		sqrm(sp);
	}
}

/* int
 * sq_here(sp)
 *	determine whether stream 'sp' is still on the streamq
 * return:
 *	boolean: is it here?
 */
int
sq_here(sp)
	register struct qstream *sp;
{
	register struct qstream *t;

	for (t = streamq;  t != QSTREAM_NULL;  t = t->s_next)
		if (t == sp)
			return 1;
	return 0;
}

/*
 * Initiate query on stream;
 * mark as referenced and stop selecting for input.
 */
void
sq_query(sp)
	register struct qstream *sp;
{
	sp->s_refcnt++;
	FD_CLR(sp->s_rfd, &mask);
}

/*
 * Note that the current request on a stream has completed,
 * and that we should continue looking for requests on the stream.
 */
void
sq_done(sp)
	register struct qstream *sp;
{

	sp->s_refcnt = 0;
	sp->s_time = tt.tv_sec;
	FD_SET(sp->s_rfd, &mask);
}

void
setproctitle(a, s)
	char *a;
	int s;
{
	int size;
	register char *cp;
	struct sockaddr_in sin;
	char buf[80];

	cp = Argv[0];
	size = sizeof(sin);
	if (getpeername(s, (struct sockaddr *)&sin, &size) == 0)
		(void) sprintf(buf, "-%s [%s]", a, inet_ntoa(sin.sin_addr));
	else {
		syslog(LOG_DEBUG, "getpeername: %m");
		(void) sprintf(buf, "-%s", a);
	}
	(void) strncpy(cp, buf, LastArg - cp);
	cp += strlen(cp);
	while (cp < LastArg)
		*cp++ = ' ';
}

u_int32_t
net_mask(in)
struct in_addr in;
{
	register u_int32_t i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (htonl(IN_CLASSA_NET));
	else if (IN_CLASSB(i))
		return (htonl(IN_CLASSB_NET));
	else
		return (htonl(IN_CLASSC_NET));
}

#if defined(BSD43_BSD43_NFS)
#undef dn_skipname
extern char *dn_skipname();
char *(*hack_skipname)() = dn_skipname;
#endif
