#ifndef lint
static	char *sccsid = "@(#)server.c	4.17 (Berkeley) 84/05/03";
#endif

#include "defs.h"

#define	ack() 	(void) write(rem, "\0\n", 2)
#define	err() 	(void) write(rem, "\1\n", 2)

char	buf[BUFSIZ];		/* general purpose buffer */
char	target[BUFSIZ];		/* target/source directory name */
char	*tp;			/* pointer to end of target name */
int	catname;		/* cat name to target name */
char	*stp[32];		/* stack of saved tp's for directories */
int	oumask;			/* old umask for creating files */

extern	FILE *lfp;		/* log file for mailing changes */

int	cleanup();

/*
 * Server routine to read requests and process them.
 * Commands are:
 *	Tname	- Transmit file if out of date
 *	Vname	- Verify if file out of date or not
 *	Qname	- Query if file exists. Return mtime & size if it does.
 */
server()
{
	char cmdbuf[BUFSIZ];
	register char *cp;

	signal(SIGHUP, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGPIPE, cleanup);

	rem = 0;
	oumask = umask(0);
	(void) sprintf(buf, "V%d\n", VERSION);
	(void) write(rem, buf, strlen(buf));

	for (;;) {
		cp = cmdbuf;
		if (read(rem, cp, 1) <= 0)
			return;
		if (*cp++ == '\n') {
			error("server: expected control record\n");
			continue;
		}
		do {
			if (read(rem, cp, 1) != 1)
				cleanup();
		} while (*cp++ != '\n' && cp < &cmdbuf[BUFSIZ]);
		*--cp = '\0';
		cp = cmdbuf;
		switch (*cp++) {
		case 'T':  /* init target file/directory name */
			catname = 1;	/* target should be directory */
			goto dotarget;

		case 't':  /* init target file/directory name */
			catname = 0;
		dotarget:
			if (exptilde(target, cp) == NULL)
				continue;
			tp = target;
			while (*tp)
				tp++;
			ack();
			continue;

		case 'R':  /* Receive. Transfer file. */
			recvf(cp, 0);
			continue;

		case 'D':  /* Directory. Transfer file. */
			recvf(cp, 1);
			continue;

		case 'E':  /* End. (of directory) */
			*tp = '\0';
			if (catname <= 0) {
				error("server: too many 'E's\n");
				continue;
			}
			tp = stp[--catname];
			*tp = '\0';
			ack();
			continue;

		case 'C':  /* Clean. Cleanup a directory */
			clean(cp);
			continue;

		case 'Q':  /* Query. Does the file/directory exist? */
			query(cp);
			continue;

		case 'S':  /* Special. Execute commands */
			dospecial(cp);
			continue;

#ifdef notdef
		/*
		 * These entries are reserved but not currently used.
		 * The intent is to allow remote hosts to have master copies.
		 * Currently, only the host rdist runs on can have masters.
		 */
		case 'X':  /* start a new list of files to exclude */
			except = bp = NULL;
		case 'x':  /* add name to list of files to exclude */
			if (*cp == '\0') {
				ack();
				continue;
			}
			if (*cp == '~') {
				if (exptilde(buf, cp) == NULL)
					continue;
				cp = buf;
			}
			if (bp == NULL)
				except = bp = expand(makeblock(NAME, cp), E_VARS);
			else
				bp->b_next = expand(makeblock(NAME, cp), E_VARS);
			while (bp->b_next != NULL)
				bp = bp->b_next;
			ack();
			continue;

		case 'I':  /* Install. Transfer file if out of date. */
			opts = 0;
			while (*cp >= '0' && *cp <= '7')
				opts = (opts << 3) | (*cp++ - '0');
			if (*cp++ != ' ') {
				error("server: options not delimited\n");
				return;
			}
			install(cp, opts);
			continue;

		case 'L':  /* Log. save message in log file */
			log(lfp, cp);
			continue;
#endif

		case '\1':
			nerrs++;
			continue;

		case '\2':
			return;

		default:
			error("server: unknown command '%s'\n", cp);
		case '\0':
			continue;
		}
	}
}

/*
 * Update the file(s) if they are different.
 * destdir = 1 if destination should be a directory
 * (i.e., more than one source is being copied to the same destination).
 */
install(src, dest, destdir, opts)
	char *src, *dest;
	int destdir, opts;
{
	char *rname;

	if (dest == NULL) {
		opts &= ~WHOLE; /* WHOLE mode only useful if renaming */
		dest = src;
	}

	if (nflag || debug) {
		printf("%s%s%s%s%s %s %s\n", opts & VERIFY ? "verify":"install",
			opts & WHOLE ? " -w" : "",
			opts & YOUNGER ? " -y" : "",
			opts & COMPARE ? " -b" : "",
			opts & REMOVE ? " -R" : "", src, dest);
		if (nflag)
			return;
	}

	rname = exptilde(target, src);
	if (rname == NULL)
		return;
	tp = target;
	while (*tp)
		tp++;
	/*
	 * If we are renaming a directory and we want to preserve
	 * the directory heirarchy (-w), we must strip off the leading
	 * directory name and preserve the rest.
	 */
	if (opts & WHOLE) {
		while (*rname == '/')
			rname++;
		destdir = 1;
	} else {
		rname = rindex(target, '/');
		if (rname == NULL)
			rname = target;
		else
			rname++;
	}
	if (debug)
		printf("target = %s, rname = %s\n", target, rname);
	/*
	 * Pass the destination file/directory name to remote.
	 */
	(void) sprintf(buf, "%c%s\n", destdir ? 'T' : 't', dest);
	if (debug)
		printf("buf = %s", buf);
	(void) write(rem, buf, strlen(buf));
	if (response() < 0)
		return;

	sendf(rname, opts);
}

/*
 * Transfer the file or directory in target[].
 * rname is the name of the file on the remote host.
 */
sendf(rname, opts)
	char *rname;
	int opts;
{
	register struct subcmd *sc;
	struct stat stb;
	int sizerr, f, u;
	off_t i;
	extern struct subcmd *subcmds;

	if (debug)
		printf("sendf(%s, %x)\n", rname, opts);

	if (except(target))
		return;
	if (access(target, 4) < 0 || lstat(target, &stb) < 0) {
		error("%s: %s\n", target, sys_errlist[errno]);
		return;
	}
	if ((u = update(rname, opts, &stb)) == 0)
		return;

	if (pw == NULL || pw->pw_uid != stb.st_uid)
		if ((pw = getpwuid(stb.st_uid)) == NULL) {
			error("%s: no password entry for uid %d\n", target,
				stb.st_uid);
			return;
		}
	if (gr == NULL || gr->gr_gid != stb.st_gid)
		if ((gr = getgrgid(stb.st_gid)) == NULL) {
			error("%s: no name for group %d\n", target, stb.st_gid);
			return;
		}
	if (u == 1) {
		if (opts & VERIFY) {
			log(lfp, "need to install: %s\n", target);
			goto dospecial;
		}
		log(lfp, "installing: %s\n", target);
		opts &= ~(COMPARE|REMOVE);
	}

	switch (stb.st_mode & S_IFMT) {
	case S_IFREG:
		break;

	case S_IFLNK:
		error("%s: cannot install soft links - use 'special'\n", target);
		return;

	case S_IFDIR:
		rsendf(rname, opts, &stb, pw->pw_name, gr->gr_name);
		return;

	default:
		error("%s: not a plain file\n", target);
		return;
	}

	if (u == 2) {
		if (opts & VERIFY) {
			log(lfp, "need to update: %s\n", target);
			goto dospecial;
		}
		log(lfp, "updating: %s\n", target);
	}
	if (stb.st_nlink != 1)
		log(lfp, "%s: Warning: more than one hard link\n", target);

	if ((f = open(target, 0)) < 0) {
		error("%s: %s\n", target, sys_errlist[errno]);
		return;
	}
	(void) sprintf(buf, "R%o %04o %D %D %s %s %s\n", opts,
		stb.st_mode & 07777, stb.st_size, stb.st_mtime,
		pw->pw_name, gr->gr_name, rname);
	if (debug)
		printf("buf = %s", buf);
	(void) write(rem, buf, strlen(buf));
	if (response() < 0) {
		(void) close(f);
		return;
	}
	sizerr = 0;
	for (i = 0; i < stb.st_size; i += BUFSIZ) {
		int amt = BUFSIZ;
		if (i + amt > stb.st_size)
			amt = stb.st_size - i;
		if (sizerr == 0 && read(f, buf, amt) != amt)
			sizerr = 1;
		(void) write(rem, buf, amt);
	}
	(void) close(f);
	if (sizerr) {
		error("%s: file changed size\n", target);
		err();
	} else
		ack();
	if (response() == 0 && (opts & COMPARE))
		return;
dospecial:
	for (sc = subcmds; sc != NULL; sc = sc->sc_next) {
		if (sc->sc_type != SPECIAL)
			continue;
		if (!inlist(sc->sc_args, target))
			continue;
		log(lfp, "special \"%s\"\n", sc->sc_name);
		if (opts & VERIFY)
			continue;
		(void) sprintf(buf, "S%s\n", sc->sc_name);
		if (debug)
			printf("buf = %s", buf);
		(void) write(rem, buf, strlen(buf));
		while (response() > 0)
			;
	}
}

rsendf(rname, opts, st, owner, group)
	char *rname;
	int opts;
	struct stat *st;
	char *owner, *group;
{
	DIR *d;
	struct direct *dp;
	char *otp, *cp;
	int len;

	if (debug)
		printf("rsendf(%s, %x, %x, %s, %s)\n", rname, opts, st,
			owner, group);

	if ((d = opendir(target)) == NULL) {
		error("%s: %s\n", target, sys_errlist[errno]);
		return;
	}
	(void) sprintf(buf, "D%o %04o 0 0 %s %s %s\n", opts,
		st->st_mode & 0777, owner, group, rname);
	if (debug)
		printf("buf = %s", buf);
	(void) write(rem, buf, strlen(buf));
	if (response() < 0) {
		closedir(d);
		return;
	}

	if (opts & REMOVE)
		rmchk(opts);

	otp = tp;
	len = tp - target;
	while (dp = readdir(d)) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (len + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error("%s/%s: Name too long\n", target, dp->d_name);
			continue;
		}
		tp = otp;
		*tp++ = '/';
		cp = dp->d_name;
		while (*tp++ = *cp++)
			;
		tp--;
		sendf(dp->d_name, opts);
	}
	closedir(d);
	(void) write(rem, "E\n", 2);
	(void) response();
	tp = otp;
	*tp = '\0';
}

/*
 * Check to see if file needs to be updated on the remote machine.
 * Returns 0 if no update, 1 if remote doesn't exist, 2 if out of date
 * and 3 if comparing binaries to determine if out of date.
 */
update(rname, opts, st)
	char *rname;
	int opts;
	struct stat *st;
{
	register char *cp, *s;
	register off_t size;
	register time_t mtime;

	if (debug) 
		printf("update(%s, %x, %x)\n", rname, opts, st);

	/*
	 * Check to see if the file exists on the remote machine.
	 */
	(void) sprintf(buf, "Q%s\n", rname);
	if (debug)
		printf("buf = %s", buf);
	(void) write(rem, buf, strlen(buf));
	cp = s = buf;
	do {
		if (read(rem, cp, 1) != 1)
			lostconn();
	} while (*cp++ != '\n' && cp < &buf[BUFSIZ]);

	switch (*s++) {
	case 'Y':
		break;

	case 'N':  /* file doesn't exist so install it */
		return(1);

	case '\1':
		nerrs++;
		if (*s != '\n') {
			if (!iamremote) {
				fflush(stdout);
				(void) write(2, s, cp - s);
			}
			if (lfp != NULL)
				(void) fwrite(s, 1, cp - s, lfp);
		}
		return(0);

	default:
		printf("buf = ");
		fwrite(buf, 1, cp - s, stdout);
		error("update: unexpected response '%c'\n", buf[0]);
		return(0);
	}

	if (*s == '\n')
		return(2);

	if (opts & COMPARE)
		return(3);

	size = 0;
	while (isdigit(*s))
		size = size * 10 + (*s++ - '0');
	if (*s++ != ' ') {
		error("update: size not delimited\n");
		return(0);
	}
	mtime = 0;
	while (isdigit(*s))
		mtime = mtime * 10 + (*s++ - '0');
	if (*s != '\n') {
		error("update: mtime not delimited\n");
		return(0);
	}
	/*
	 * File needs to be updated?
	 */
	if (opts & YOUNGER) {
		if (st->st_mtime == mtime)
			return(0);
		if (st->st_mtime < mtime) {
			log(lfp, "Warning: %s: remote copy is newer\n", target);
			return(0);
		}
	} else if (st->st_mtime == mtime && st->st_size == size)
		return(0);
	return(2);
}

/*
 * Query. Check to see if file exists. Return one of the following:
 *	N\n		- doesn't exist
 *	Ysize mtime\n	- exists and its a regular file (size & mtime of file)
 *	Y\n		- exists and its a directory
 *	^Aerror message\n
 */
query(name)
	char *name;
{
	struct stat stb;

	if (catname)
		(void) sprintf(tp, "/%s", name);

	if (stat(target, &stb) < 0) {
		(void) write(rem, "N\n", 2);
		*tp = '\0';
		return;
	}

	switch (stb.st_mode & S_IFMT) {
	case S_IFREG:
		(void) sprintf(buf, "Y%D %D\n", stb.st_size, stb.st_mtime);
		(void) write(rem, buf, strlen(buf));
		break;

	case S_IFDIR:
		(void) write(rem, "Y\n", 2);
		break;

	default:
		error("%s: not a plain file\n", name);
		break;
	}
	*tp = '\0';
}

recvf(cmd, isdir)
	char *cmd;
	int isdir;
{
	register char *cp;
	int f, mode, opts, wrerr, olderrno;
	off_t i, size;
	time_t mtime;
	struct stat stb;
	struct timeval tvp[2];
	char *owner, *group, *dir;
	char new[BUFSIZ];
	extern char *tmpname;

	cp = cmd;
	opts = 0;
	while (*cp >= '0' && *cp <= '7')
		opts = (opts << 3) | (*cp++ - '0');
	if (*cp++ != ' ') {
		error("recvf: options not delimited\n");
		return;
	}
	mode = 0;
	while (*cp >= '0' && *cp <= '7')
		mode = (mode << 3) | (*cp++ - '0');
	if (*cp++ != ' ') {
		error("recvf: mode not delimited\n");
		return;
	}
	size = 0;
	while (isdigit(*cp))
		size = size * 10 + (*cp++ - '0');
	if (*cp++ != ' ') {
		error("recvf: size not delimited\n");
		return;
	}
	mtime = 0;
	while (isdigit(*cp))
		mtime = mtime * 10 + (*cp++ - '0');
	if (*cp++ != ' ') {
		error("recvf: mtime not delimited\n");
		return;
	}
	owner = cp;
	while (*cp && *cp != ' ')
		cp++;
	if (*cp != ' ') {
		error("recvf: owner name not delimited\n");
		return;
	}
	*cp++ = '\0';
	group = cp;
	while (*cp && *cp != ' ')
		cp++;
	if (*cp != ' ') {
		error("recvf: group name not delimited\n");
		return;
	}
	*cp++ = '\0';

	if (isdir) {
		if (catname >= sizeof(stp)) {
			error("%s: too many directory levels\n", target);
			return;
		}
		stp[catname] = tp;
		if (catname++) {
			*tp++ = '/';
			while (*tp++ = *cp++)
				;
			tp--;
		}
		if (opts & VERIFY) {
			ack();
			return;
		}
		if (stat(target, &stb) == 0) {
			if (ISDIR(stb.st_mode)) {
				if ((stb.st_mode & 0777) == mode) {
					ack();
					return;
				}
				buf[0] = '\0';
				(void) sprintf(buf + 1,
					"%s:%s: Warning: mode %o != %o\n",
					host, target, stb.st_mode & 0777, mode);
				(void) write(rem, buf, strlen(buf + 1) + 1);
				return;
			}
			error("%s: %s\n", target, sys_errlist[ENOTDIR]);
		} else if (chkparent(target) < 0 || mkdir(target, mode) < 0)
			error("%s: %s\n", target, sys_errlist[errno]);
		else if (chog(target, owner, group, mode) == 0) {
			ack();
			return;
		}
		tp = stp[--catname];
		*tp = '\0';
		return;
	}

	new[0] = '\0';
	if (catname)
		(void) sprintf(tp, "/%s", cp);
	if (stat(target, &stb) == 0 && (stb.st_mode & S_IFMT) != S_IFREG) {
		error("%s: not a regular file\n", target);
		return;
	}
	if (chkparent(target) < 0)
		goto bad;
	cp = rindex(target, '/');
	if (cp == NULL)
		dir = ".";
	else if (cp == target) {
		dir = "";
		cp = NULL;
	} else {
		dir = target;
		*cp = '\0';
	}
	(void) sprintf(new, "%s/%s", dir, tmpname);
	if (cp != NULL)
		*cp = '/';
	if ((f = creat(new, mode)) < 0)
		goto bad1;
	ack();

	wrerr = 0;
	for (i = 0; i < size; i += BUFSIZ) {
		int amt = BUFSIZ;

		cp = buf;
		if (i + amt > size)
			amt = size - i;
		do {
			int j = read(rem, cp, amt);

			if (j <= 0) {
				(void) close(f);
				(void) unlink(new);
				cleanup();
			}
			amt -= j;
			cp += j;
		} while (amt > 0);
		amt = BUFSIZ;
		if (i + amt > size)
			amt = size - i;
		if (wrerr == 0 && write(f, buf, amt) != amt) {
			olderrno = errno;
			wrerr++;
		}
	}
	(void) close(f);
	(void) response();
	if (wrerr) {
		error("%s: %s\n", cp, sys_errlist[olderrno]);
		(void) unlink(new);
		return;
	}
	if (opts & COMPARE) {
		FILE *f1, *f2;
		int c;

		if ((f1 = fopen(target, "r")) == NULL)
			goto bad;
		if ((f2 = fopen(new, "r")) == NULL)
			goto bad1;
		while ((c = getc(f1)) == getc(f2))
			if (c == EOF) {
				(void) fclose(f1);
				(void) fclose(f2);
				(void) unlink(new);
				ack();
				return;
			}
		(void) fclose(f1);
		(void) fclose(f2);
		if (opts & VERIFY) {
			(void) unlink(new);
			buf[0] = '\0';
			(void) sprintf(buf + 1, "need to update %s:%s\n",
				host, target);
			(void) write(rem, buf, strlen(buf + 1) + 1);
			return;
		}
	}

	/*
	 * Set last modified time
	 */
	tvp[0].tv_sec = stb.st_atime;	/* old accessed time from target */
	tvp[0].tv_usec = 0;
	tvp[1].tv_sec = mtime;
	tvp[1].tv_usec = 0;
	if (utimes(new, tvp) < 0) {
bad1:
		error("%s: %s\n", new, sys_errlist[errno]);
		(void) unlink(new);
		return;
	}
	if (chog(new, owner, group, mode) < 0) {
		(void) unlink(new);
		return;
	}
	
	if (rename(new, target) < 0) {
bad:
		error("%s: %s\n", target, sys_errlist[errno]);
		if (new[0])
			(void) unlink(new);
		return;
	}
	if (opts & COMPARE) {
		buf[0] = '\0';
		(void) sprintf(buf + 1, "updated %s:%s\n", host, target);
		(void) write(rem, buf, strlen(buf + 1) + 1);
	} else
		ack();
}

/*
 * Check parent directory for write permission and create if it doesn't
 * exist.
 */
chkparent(name)
	char *name;
{
	register char *cp, *dir;
	extern int userid, groupid;

	cp = rindex(name, '/');
	if (cp == NULL)
		dir = ".";
	else if (cp == name) {
		dir = "/";
		cp = NULL;
	} else {
		dir = name;
		*cp = '\0';
	}
	if (access(dir, 2) == 0) {
		if (cp != NULL)
			*cp = '/';
		return(0);
	}
	if (errno == ENOENT) {
		if (rindex(dir, '/') != NULL && chkparent(dir) < 0)
			goto bad;
		if (!strcmp(dir, ".") || !strcmp(dir, "/"))
			goto bad;
		if (mkdir(dir, 0777 & ~oumask) < 0)
			goto bad;
		if (chown(dir, userid, groupid) < 0) {
			(void) unlink(dir);
			goto bad;
		}
		if (cp != NULL)
			*cp = '/';
		return(0);
	}

bad:
	if (cp != NULL)
		*cp = '/';
	return(-1);
}

/*
 * Change owner, group and mode of file.
 */
chog(file, owner, group, mode)
	char *file, *owner, *group;
	int mode;
{
	extern int userid, groupid;
	extern char user[];
	register int i;
	int uid, gid;

	uid = userid;
	if (userid == 0) {
		if (pw == NULL || strcmp(owner, pw->pw_name) != 0) {
			if ((pw = getpwnam(owner)) == NULL) {
				if (mode & 04000) {
					error("%s: unknown login name\n", owner);
					return(-1);
				}
			} else
				uid = pw->pw_uid;
		} else
			uid = pw->pw_uid;
	} else if ((mode & 04000) && strcmp(user, owner) != 0)
		mode &= ~04000;
	gid = groupid;
	if (gr == NULL || strcmp(group, gr->gr_name) != 0) {
		if ((gr = getgrnam(group)) == NULL) {
			if (mode & 02000) {
				error("%s: unknown group\n", group);
				return(-1);
			}
		} else
			gid = gr->gr_gid;
	} else
		gid = gr->gr_gid;
	if (userid && groupid != gid) {
		for (i = 0; gr->gr_mem[i] != NULL; i++)
			if (!(strcmp(user, gr->gr_mem[i])))
				goto ok;
		mode &= ~02000;
		gid = groupid;
	}
ok:
	if (chown(file, uid, gid) < 0) {
		error("%s: %s\n", file, sys_errlist[errno]);
		return(-1);
	}
	/*
	 * Restore set-user-id or set-group-id bit if appropriate.
	 */
	if ((mode & 06000) && chmod(file, mode) < 0) {
		error("%s: %s\n", file, sys_errlist[errno]);
		return(-1);
	}
	return(0);
}

/*
 * Check for files on the machine being updated that are not on the master
 * machine and remove them.
 */
rmchk(opts)
	int opts;
{
	register char *cp, *s;
	struct stat stb;

	if (debug)
		printf("rmchk()\n");

	/*
	 * Tell the remote to clean the files from the last directory sent.
	 */
	(void) sprintf(buf, "C%o\n", opts & VERIFY);
	if (debug)
		printf("buf = %s", buf);
	(void) write(rem, buf, strlen(buf));
	if (response() < 0)
		return;
	for (;;) {
		cp = s = buf;
		do {
			if (read(rem, cp, 1) != 1)
				lostconn();
		} while (*cp++ != '\n' && cp < &buf[BUFSIZ]);

		switch (*s++) {
		case 'Q': /* Query if file should be removed */
			/*
			 * Return the following codes to remove query.
			 * N\n -- file exists - DON'T remove.
			 * Y\n -- file doesn't exist - REMOVE.
			 */
			*--cp = '\0';
			(void) sprintf(tp, "/%s", s);
			if (debug)
				printf("check %s\n", target);
			if (except(target))
				(void) write(rem, "N\n", 2);
			else if (stat(target, &stb) < 0)
				(void) write(rem, "Y\n", 2);
			else
				(void) write(rem, "N\n", 2);
			break;

		case '\0':
			*--cp = '\0';
			if (*s != '\0')
				log(lfp, "%s\n", s);
			break;

		case 'E':
			*tp = '\0';
			ack();
			return;

		case '\1':
		case '\2':
			nerrs++;
			if (*s != '\n') {
				if (!iamremote) {
					fflush(stdout);
					(void) write(2, s, cp - s);
				}
				if (lfp != NULL)
					(void) fwrite(s, 1, cp - s, lfp);
			}
			if (buf[0] == '\2')
				lostconn();
			break;

		default:
			error("rmchk: unexpected response '%s'\n", buf);
			err();
		}
	}
}

/*
 * Check the current directory (initialized by the 'T' command to server())
 * for extraneous files and remove them.
 */
clean(cp)
	register char *cp;
{
	DIR *d;
	register struct direct *dp;
	struct stat stb;
	char *otp;
	int len, opts;

	opts = 0;
	while (*cp >= '0' && *cp <= '7')
		opts = (opts << 3) | (*cp++ - '0');
	if (*cp != '\0') {
		error("clean: options not delimited\n");
		return;
	}
	if (access(target, 6) < 0 || (d = opendir(target)) == NULL) {
		error("%s: %s\n", target, sys_errlist[errno]);
		return;
	}
	ack();

	otp = tp;
	len = tp - target;
	while (dp = readdir(d)) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (len + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error("%s/%s: Name too long\n", target, dp->d_name);
			continue;
		}
		tp = otp;
		*tp++ = '/';
		cp = dp->d_name;;
		while (*tp++ = *cp++)
			;
		tp--;
		if (stat(target, &stb) < 0) {
			error("%s: %s\n", target, sys_errlist[errno]);
			continue;
		}
		(void) sprintf(buf, "Q%s\n", dp->d_name);
		(void) write(rem, buf, strlen(buf));
		cp = buf;
		do {
			if (read(rem, cp, 1) != 1)
				cleanup();
		} while (*cp++ != '\n' && cp < &buf[BUFSIZ]);
		*--cp = '\0';
		cp = buf;
		if (*cp != 'Y')
			continue;
		if (opts & VERIFY) {
			cp = buf;
			*cp++ = '\0';
			(void) sprintf(cp, "need to remove %s\n", target);
			(void) write(rem, buf, strlen(cp) + 1);
		} else
			remove(&stb);
	}
	closedir(d);
	(void) write(rem, "E\n", 2);
	(void) response();
	tp = otp;
	*tp = '\0';
}

/*
 * Remove a file or directory (recursively) and send back an acknowledge
 * or an error message.
 */
remove(st)
	struct stat *st;
{
	DIR *d;
	struct direct *dp;
	register char *cp;
	struct stat stb;
	char *otp;
	int len;

	switch (st->st_mode & S_IFMT) {
	case S_IFREG:
		if (unlink(target) < 0)
			goto bad;
		goto removed;

	case S_IFDIR:
		break;

	default:
		error("%s: not a plain file\n", target);
		return;
	}

	if (access(target, 6) < 0 || (d = opendir(target)) == NULL)
		goto bad;

	otp = tp;
	len = tp - target;
	while (dp = readdir(d)) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (len + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error("%s/%s: Name too long\n", target, dp->d_name);
			continue;
		}
		tp = otp;
		*tp++ = '/';
		cp = dp->d_name;;
		while (*tp++ = *cp++)
			;
		tp--;
		if (stat(target, &stb) < 0) {
			error("%s: %s\n", target, sys_errlist[errno]);
			continue;
		}
		remove(&stb);
	}
	closedir(d);
	tp = otp;
	*tp = '\0';
	if (rmdir(target) < 0) {
bad:
		error("%s: %s\n", target, sys_errlist[errno]);
		return;
	}
removed:
	cp = buf;
	*cp++ = '\0';
	(void) sprintf(cp, "removed %s\n", target);
	(void) write(rem, buf, strlen(cp) + 1);
}

/*
 * Execute a shell command to handle special cases.
 */
dospecial(cmd)
	char *cmd;
{
	int fd[2], status, pid, i;
	register char *cp, *s;
	char sbuf[BUFSIZ];

	if (pipe(fd) < 0) {
		error("%s\n", sys_errlist[errno]);
		return;
	}
	if ((pid = fork()) == 0) {
		/*
		 * Return everything the shell commands print.
		 */
		(void) close(0);
		(void) close(1);
		(void) close(2);
		(void) open("/dev/null", 0);
		(void) dup(fd[1]);
		(void) dup(fd[1]);
		(void) close(fd[0]);
		(void) close(fd[1]);
		execl("/bin/sh", "sh", "-c", cmd, 0);
		_exit(127);
	}
	(void) close(fd[1]);
	s = sbuf;
	*s++ = '\0';
	while ((i = read(fd[0], buf, sizeof(buf))) > 0) {
		cp = buf;
		do {
			*s++ = *cp++;
			if (cp[-1] != '\n') {
				if (s < &sbuf[sizeof(sbuf)-1])
					continue;
				*s++ = '\n';
			}
			/*
			 * Throw away blank lines.
			 */
			if (s == &sbuf[2]) {
				s--;
				continue;
			}
			(void) write(rem, sbuf, s - sbuf);
			s = &sbuf[1];
		} while (--i);
	}
	if (s > &sbuf[1]) {
		*s++ = '\n';
		(void) write(rem, sbuf, s - sbuf);
	}
	while ((i = wait(&status)) != pid && i != -1)
		;
	if (i == -1)
		status = -1;
	if (status)
		error("shell returned %d\n", status);
	else
		ack();
}

/*VARARGS2*/
log(fp, fmt, a1, a2, a3)
	FILE *fp;
	char *fmt;
	int a1, a2, a3;
{
	/* Print changes locally if not quiet mode */
	if (!qflag)
		printf(fmt, a1, a2, a3);

	/* Save changes (for mailing) if really updating files */
	if (!(options & VERIFY) && fp != NULL)
		fprintf(fp, fmt, a1, a2, a3);
}

/*VARARGS1*/
error(fmt, a1, a2, a3)
	char *fmt;
	int a1, a2, a3;
{
	nerrs++;
	strcpy(buf, "\1rdist: ");
	(void) sprintf(buf+8, fmt, a1, a2, a3);
	if (!iamremote) {
		fflush(stdout);
		(void) write(2, buf+1, strlen(buf+1));
	} else
		(void) write(rem, buf, strlen(buf));
	if (lfp != NULL)
		(void) fwrite(buf+1, 1, strlen(buf+1), lfp);
}

/*VARARGS1*/
fatal(fmt, a1, a2,a3)
	char *fmt;
	int a1, a2, a3;
{
	nerrs++;
	strcpy(buf, "\2rdist: ");
	(void) sprintf(buf+8, fmt, a1, a2, a3);
	if (!iamremote) {
		fflush(stdout);
		(void) write(2, buf+1, strlen(buf+1));
	} else
		(void) write(rem, buf, strlen(buf));
	if (lfp != NULL)
		(void) fwrite(buf+1, 1, strlen(buf+1), lfp);
	cleanup();
}

response()
{
	char *cp, *s;

	if (debug)
		printf("response()\n");

	cp = s = buf;
	do {
		if (read(rem, cp, 1) != 1)
			lostconn();
	} while (*cp++ != '\n' && cp < &buf[BUFSIZ]);

	switch (*s++) {
	case '\0':
		*--cp = '\0';
		if (*s != '\0') {
			log(lfp, "%s\n", s);
			return(1);
		}
		return(0);

	default:
		s--;
		/* fall into... */
	case '\1':
	case '\2':
		nerrs++;
		if (*s != '\n') {
			if (!iamremote) {
				fflush(stdout);
				(void) write(2, s, cp - s);
			}
			if (lfp != NULL)
				(void) fwrite(s, 1, cp - s, lfp);
		}
		if (buf[0] == '\2')
			lostconn();
		return(-1);
	}
}

/*
 * Remove temporary files and do any cleanup operations before exiting.
 */
cleanup()
{
	(void) unlink(tmpfile);
	exit(1);
}
