/*
 * Copyright (c) 1982, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)inode.h	8.1 (Berkeley) %G%
 */

#include <ufs/ufs/dinode.h>

/*
 * Theoretically, directories can be more than 2Gb in length,
 * however, in practice this seems unlikely. So, we define
 * the type doff_t as a long to keep down the cost of doing
 * lookup on a 32-bit machine. If you are porting to a 64-bit
 * architecture, you should make doff_t the same as off_t.
 */
#define doff_t	long

/*
 * The inode is used to describe each active (or recently active)
 * file in the UFS filesystem. It is composed of two types of
 * information. The first part is the information that is needed
 * only while the file is active (such as the identity of the file
 * and linkage to speed its lookup). The second part is the 
 * permannent meta-data associated with the file which is read
 * in from the permanent dinode from long term storage when the
 * file becomes active, and is put back when the file is no longer
 * being used.
 */
struct inode {
	struct	inode *i_next;	/* hash chain forward */
	struct	inode **i_prev;	/* hash chain back */
	struct	vnode *i_vnode;	/* vnode associated with this inode */
	struct	vnode *i_devvp;	/* vnode for block I/O */
	u_long	i_flag;		/* see below */
	dev_t	i_dev;		/* device where inode resides */
	ino_t	i_number;	/* the identity of the inode */
	union {			/* associated filesystem */
		struct	fs *fs;		/* FFS */
		struct	lfs *lfs;	/* LFS */
	} inode_u;
#define	i_fs	inode_u.fs
#define	i_lfs	inode_u.lfs
	struct	dquot *i_dquot[MAXQUOTAS]; /* pointer to dquot structures */
	u_quad_t i_modrev;	/* revision level for lease */
	struct	lockf *i_lockf;	/* head of byte-level lock list */
	pid_t	i_lockholder;	/* DEBUG: holder of inode lock */
	pid_t	i_lockwaiter;	/* DEBUG: latest blocked for inode lock */
	/*
	 * Side effects; used during directory lookup.
	 */
	long	i_count;	/* size of free slot in directory */
	doff_t	i_endoff;	/* end of useful stuff in directory */
	doff_t	i_diroff;	/* offset in dir, where we found last entry */
	doff_t	i_offset;	/* offset of free space in directory */
	ino_t	i_ino;		/* inode number of found directory */
	u_long	i_reclen;	/* size of found directory entry */
	long	i_spare[11];	/* spares to round up to 128 bytes */
	/*
	 * the on-disk dinode itself.
	 */
	struct	dinode i_din;	/* 128 bytes of the on-disk dinode */
};

#define	i_mode		i_din.di_mode
#define	i_nlink		i_din.di_nlink
#define	i_uid		i_din.di_uid
#define	i_gid		i_din.di_gid
#define i_size		i_din.di_size
#define	i_db		i_din.di_db
#define	i_ib		i_din.di_ib
#define	i_atime		i_din.di_atime
#define	i_mtime		i_din.di_mtime
#define	i_ctime		i_din.di_ctime
#define i_blocks	i_din.di_blocks
#define	i_rdev		i_din.di_rdev
#define	i_shortlink	i_din.di_shortlink
#define i_flags		i_din.di_flags
#define i_gen		i_din.di_gen

/* flags */
#define	ILOCKED		0x0001		/* inode is locked */
#define	IWANT		0x0002		/* some process waiting on lock */
#define	IRENAME		0x0004		/* inode is being renamed */
#define	IUPD		0x0010		/* file has been modified */
#define	IACC		0x0020		/* inode access time to be updated */
#define	ICHG		0x0040		/* inode has been changed */
#define	IMOD		0x0080		/* inode has been modified */
#define	ISHLOCK		0x0100		/* file has shared lock */
#define	IEXLOCK		0x0200		/* file has exclusive lock */
#define	ILWAIT		0x0400		/* someone waiting on file lock */

#ifdef KERNEL
/*
 * Structure used to pass around logical block paths generated by
 * ufs_getlbns and used by truncate and bmap code.
 */
struct indir {
	daddr_t	in_lbn;			/* logical block number */
	int	in_off;			/* offset in buffer */
	int	in_exists;		/* does this block exist yet */
};

/* Convert between inode pointers and vnode pointers. */
#define VTOI(vp)	((struct inode *)(vp)->v_data)
#define ITOV(ip)	((ip)->i_vnode)

#define	ITIMES(ip, t1, t2) { \
	if ((ip)->i_flag&(IUPD|IACC|ICHG)) { \
		(ip)->i_flag |= IMOD; \
		if ((ip)->i_flag&IACC) \
			(ip)->i_atime.ts_sec = (t1)->tv_sec; \
		if ((ip)->i_flag&IUPD) { \
			(ip)->i_mtime.ts_sec = (t2)->tv_sec; \
			(ip)->i_modrev++; \
		} \
		if ((ip)->i_flag&ICHG) \
			(ip)->i_ctime.ts_sec = time.tv_sec; \
		(ip)->i_flag &= ~(IACC|IUPD|ICHG); \
	} \
}

/* This overlays the fid structure (see mount.h). */
struct ufid {
	u_short	ufid_len;	/* length of structure */
	u_short	ufid_pad;	/* force long alignment */
	ino_t	ufid_ino;	/* file number (ino) */
	long	ufid_gen;	/* generation number */
};
#endif /* KERNEL */
