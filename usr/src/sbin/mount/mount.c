/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)mount.c	5.9 (Berkeley) %G%";
#endif not lint

#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <fstab.h>
#include <mtab.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <sys/dir.h>
#include <sys/uio.h>
#include <sys/namei.h>
#include <sys/mount.h>
#ifdef NFS
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <nfs/rpcv2.h>
#include <nfs/nfsv2.h>
#include <nfs/nfs.h>
#endif

#define	BADTYPE(type) \
	(strcmp(type, FSTAB_RO) && strcmp(type, FSTAB_RW) && \
	    strcmp(type, FSTAB_RQ))
#define	SETTYPE(type) \
	(!strcmp(type, FSTAB_RW) || !strcmp(type, FSTAB_RQ))

#define	MTAB	"/etc/mtab"

static struct mtab mtab[NMOUNT];
static int fake, verbose, mnttype;
#ifdef NFS
int xdr_dir(), xdr_fh();
struct nfs_args nfsargs = {
	(struct sockaddr_in *)0,
	(nfsv2fh_t *)0,
	0,
	NFS_WSIZE,
	NFS_RSIZE,
	NFS_TIMEO,
	NFS_RETRANS,
	(char *)0,
};

struct nfhret {
	u_long	stat;
	nfsv2fh_t nfh;
};
int retrycnt = 10000;
#define	BGRND	1
#define	ISBGRND	2
int opflags = 0;
#endif

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	register struct mtab *mp;
	register struct fstab *fs;
	register int cnt;
	int all, ch, fd, rval, sfake;
	char *type;

	all = 0;
	type = NULL;
	mnttype = MOUNT_UFS;
	while ((ch = getopt(argc, argv, "afrwvt:o:")) != EOF)
		switch((char)ch) {
		case 'a':
			all = 1;
			break;
		case 'f':
			fake = 1;
			break;
		case 'r':
			type = FSTAB_RO;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'w':
			type = FSTAB_RW;
			break;
#ifdef NFS
		case 't':
			if (!strcmp(optarg, "nfs"))
				mnttype = MOUNT_NFS;
			break;
		case 'o':
			getoptions(optarg,&nfsargs,&opflags,&retrycnt);
			break;
#endif
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	/* NOSTRICT */
	if ((fd = open(MTAB, O_RDONLY, 0)) >= 0) {
		if (read(fd, (char *)mtab, NMOUNT * sizeof(struct mtab)) < 0)
			mtaberr();
		(void)close(fd);
	}

	if (all) {
		rval = 0;
		for (sfake = fake; fs = getfsent(); fake = sfake) {
			if (BADTYPE(fs->fs_type))
				continue;
			/* `/' is special, it's always mounted */
			if (!strcmp(fs->fs_file, "/"))
				fake = 1;
#ifdef NFS
			if (index(fs->fs_spec, '@') != NULL) {
				if (fs->fs_vfstype != NULL &&
				    strcmp(fs->fs_vfstype, "nfs"))
					continue;
				if (fs->fs_mntops != NULL)
					getoptions(fs->fs_mntops, &nfsargs,
						&opflags, &retrycnt);
				mnttype = MOUNT_NFS;
			} else
				mnttype = MOUNT_UFS;
#endif
			rval |= mountfs(fs->fs_spec, fs->fs_file,
			    type ? type : fs->fs_type);
		}
		exit(rval);
	}

	if (argc == 0) {
		if (verbose || fake || type)
			usage();
		for (mp = mtab, cnt = NMOUNT; cnt--; ++mp)
			if (*mp->m_path)
				prmtab(mp);
		exit(0);
	}

	if (all)
		usage();

	if (argc == 1) {
		if (!(fs = getfsfile(*argv)) && !(fs = getfsspec(*argv))) {
			fprintf(stderr,
			    "mount: unknown special file or file system %s.\n",
			    *argv);
			exit(1);
		}
		if (BADTYPE(fs->fs_type)) {
			fprintf(stderr,
			    "mount: %s has unknown file system type.\n", *argv);
			exit(1);
		}
		exit(mountfs(fs->fs_spec, fs->fs_file,
		    type ? type : fs->fs_type));
	}

	if (argc != 2)
		usage();

	exit(mountfs(argv[0], argv[1], type ? type : "rw"));
}

static
mountfs(spec, name, type)
	char *spec, *name, *type;
{
	extern int errno;
	register struct mtab *mp, *space;
	register int cnt;
	register char *p;
#ifdef NFS
	register CLIENT *clp;
	struct hostent *hp;
	struct sockaddr_in saddr;
	struct timeval pertry, try;
	enum clnt_stat clnt_stat;
	int so = RPC_ANYSOCK;
	char *hostp, *delimp;
	u_short tport;
	struct nfhret nfhret;
	char nam[MNAMELEN+1];
#endif
	int fd, flags;
	struct ufs_args args;
	char *argp;

	if (!fake) {
		flags = 0;
		if (!strcmp(type, FSTAB_RO))
			flags |= M_RDONLY;
#ifdef NFS
		switch (mnttype) {
		case MOUNT_UFS:
			args.fspec = spec;
			argp = (caddr_t)&args;
			break;
		case MOUNT_NFS:
			strncpy(nam, spec, MNAMELEN);
			nam[MNAMELEN] = '\0';
			if ((delimp = index(spec, '@')) != NULL) {
				hostp = delimp+1;
			} else if ((delimp = index(spec, ':')) != NULL) {
				hostp = spec;
				spec = delimp+1;
			} else {
				fprintf(stderr, "No <host>:<dirpath> or <dirpath>@<host> spec\n");
				return (1);
			}
			*delimp = '\0';
			if ((hp = gethostbyname(hostp)) == NULL) {
				fprintf(stderr,"Can't get net id for host\n");
				return (1);
			}
			bcopy(hp->h_addr,(caddr_t)&saddr.sin_addr,hp->h_length);
			nfhret.stat = EACCES;	/* Mark not yet successful */
			while (retrycnt > 0) {
				saddr.sin_family = AF_INET;
				saddr.sin_port = htons(PMAPPORT);
				if ((tport = pmap_getport(&saddr, RPCPROG_NFS,
					NFS_VER2, IPPROTO_UDP)) == 0) {
					if ((opflags & ISBGRND) == 0)
						clnt_pcreateerror("NFS Portmap");
				} else {
					saddr.sin_port = 0;
					pertry.tv_sec = 10;
					pertry.tv_usec = 0;
					if ((clp = clntudp_create(&saddr, RPCPROG_MNT,
						RPCMNT_VER1, pertry, &so)) == NULL) {
						if ((opflags & ISBGRND) == 0)
							clnt_pcreateerror("Cannot MNT PRC");
					} else {
						clp->cl_auth = authunix_create_default();
						try.tv_sec = 10;
						try.tv_usec = 0;
						clnt_stat = clnt_call(clp, RPCMNT_MOUNT, xdr_dir, spec,
							xdr_fh, &nfhret, try);
						if (clnt_stat != RPC_SUCCESS) {
							if ((opflags & ISBGRND) == 0)
								clnt_perror(clp, "Bad MNT RPC");
						} else {
							auth_destroy(clp->cl_auth);
							clnt_destroy(clp);
							retrycnt = 0;
						}
					}
				}
				if (--retrycnt > 0) {
					if (opflags & BGRND) {
						opflags &= ~BGRND;
						if (fork())
							return (0);
						else
							opflags |= ISBGRND;
					} 
					sleep(10);
				}
			}
			if (nfhret.stat) {
				if (opflags & ISBGRND)
					exit(1);
				fprintf(stderr, "Can't access %s, errno=%d\n",
					spec, nfhret.stat);
				return (1);
			}
			saddr.sin_port = htons(tport);
			nfsargs.addr = &saddr;
			nfsargs.fh = &nfhret.nfh;
			nfsargs.hostname = nam;
			argp = (caddr_t)&nfsargs;
			break;
		};
#endif
		if (mount(mnttype, name, flags, argp)) {
			if (opflags & ISBGRND)
				exit(1);
			fprintf(stderr, "%s on %s: ", spec, name);
			switch (errno) {
			case EMFILE:
				fprintf(stderr, "Mount table full\n");
				break;
			case EINVAL:
				fprintf(stderr, "Bogus super block\n");
				break;
			default:
				perror((char *)NULL);
				break;
			}
			return(1);
		}

		/* we don't do quotas.... */
		if (strcmp(type, FSTAB_RQ) == 0)
			type = FSTAB_RW;
	}

	/* trim trailing /'s and find last component of name */
	for (p = index(spec, '\0'); *--p == '/';);
	*++p = '\0';
	if (p = rindex(spec, '/')) {
		*p = '\0';
		spec = p + 1;
	}

	for (mp = mtab, cnt = NMOUNT, space = NULL; cnt--; ++mp) {
		if (!strcmp(mp->m_dname, spec))
			break;
		if (!space && !*mp->m_path)
			space = mp;
	}
	if (cnt == -1) {
		if (!space) {
			if ((opflags & ISBGRND) == 0)
				fprintf(stderr, "mount: no more room in %s.\n", MTAB);
			exit(1);
		}
		mp = space;
	}

#define	DNMAX	(sizeof(mtab[0].m_dname) - 1)
#define	PNMAX	(sizeof(mtab[0].m_path) - 1)

	(void)strncpy(mp->m_dname, spec, DNMAX);
	mp->m_dname[DNMAX] = '\0';
	(void)strncpy(mp->m_path, name, PNMAX);
	mp->m_path[PNMAX] = '\0';
	(void)strcpy(mp->m_type, type);

	if (verbose)
		prmtab(mp);

	for (mp = mtab + NMOUNT - 1; mp > mtab && !*mp->m_path; --mp);
	if ((fd = open(MTAB, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0)
		mtaberr();
	cnt = (mp - mtab + 1) * sizeof(struct mtab);
	/* NOSTRICT */
	if (write(fd, (char *)mtab, cnt) != cnt)
		mtaberr();
	(void)close(fd);
	if (opflags & ISBGRND)
		exit();
	else
		return(0);
}

static
prmtab(mp)
	register struct mtab *mp;
{
	if (opflags & ISBGRND)
		return;
	printf("%s on %s", mp->m_dname, mp->m_path);
	if (!strcmp(mp->m_type, FSTAB_RO))
		printf("\t(read-only)");
	else if (!strcmp(mp->m_type, FSTAB_RQ))
		printf("\t(with quotas)");
	printf("\n");
}

static
mtaberr()
{
	if (opflags & ISBGRND)
		exit(1);
	fprintf(stderr, "mount: %s: ", MTAB);
	perror((char *)NULL);
	exit(1);
}

static
usage()
{
	fprintf(stderr, "usage: mount [-afrw]\nor mount [-frw] special | node\nor mount [-frw] special node\n");
	exit(1);
}

#ifdef NFS
/*
 * xdr routines for mount rpc's
 */
xdr_dir(xdrsp, dirp)
	XDR *xdrsp;
	char *dirp;
{
	return (xdr_string(xdrsp, &dirp, RPCMNT_PATHLEN));
}

xdr_fh(xdrsp, np)
	XDR *xdrsp;
	struct nfhret *np;
{
	if (!xdr_u_long(xdrsp, &(np->stat)))
		return (0);
	if (np->stat)
		return (1);
	return (xdr_opaque(xdrsp, (caddr_t)&(np->nfh), NFSX_FH));
}

/*
 * Handle the getoption arg.
 * Essentially update "opflags", "retrycnt" and "nfsargs"
 */
getoptions(optarg, nfsargsp, opflagsp, retrycntp)
	char *optarg;
	struct nfs_args *nfsargsp;
	int *opflagsp;
	int *retrycntp;
{
	register char *cp, *nextcp;
	int num;
	char *nump;

	cp = optarg;
	while (cp != NULL && *cp != '\0') {
		if ((nextcp = index(cp, ',')) != NULL)
			*nextcp++ = '\0';
		if ((nump = index(cp, '=')) != NULL) {
			*nump++ = '\0';
			num = atoi(nump);
		} else
			num = -1;
		/*
		 * Just test for a string match and do it
		 */
		if (!strcmp(cp, "bg")) {
			*opflagsp |= BGRND;
		} else if (!strcmp(cp, "soft")) {
			nfsargsp->flags |= NFSMNT_SOFT;
		} else if (!strcmp(cp, "intr")) {
			nfsargsp->flags |= NFSMNT_INT;
		} else if (!strcmp(cp, "retry") && num > 0) {
			*retrycntp = num;
		} else if (!strcmp(cp, "rsize") && num > 0) {
			nfsargsp->rsize = num;
			nfsargsp->flags |= NFSMNT_RSIZE;
		} else if (!strcmp(cp, "wsize") && num > 0) {
			nfsargsp->wsize = num;
			nfsargsp->flags |= NFSMNT_WSIZE;
		} else if (!strcmp(cp, "timeo") && num > 0) {
			nfsargsp->timeo = num;
			nfsargsp->flags |= NFSMNT_TIMEO;
		} else if (!strcmp(cp, "retrans") && num > 0) {
			nfsargsp->retrans = num;
			nfsargsp->flags |= NFSMNT_RETRANS;
		}
		cp = nextcp;
	}
}
#endif
