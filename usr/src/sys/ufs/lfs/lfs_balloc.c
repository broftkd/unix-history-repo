/*
 * Copyright (c) 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)lfs_balloc.c	8.4 (Berkeley) %G%
 */
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/resourcevar.h>
#include <sys/trace.h>

#include <miscfs/specfs/specdev.h>

#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufsmount.h>

#include <ufs/lfs/lfs.h>
#include <ufs/lfs/lfs_extern.h>

int
lfs_balloc(vp, offset, iosize, lbn, bpp)
	struct vnode *vp;
	int offset;
	u_long iosize;
	ufs_daddr_t lbn;
	struct buf **bpp;
{
	struct buf *ibp, *bp;
	struct inode *ip;
	struct lfs *fs;
	struct indir indirs[NIADDR+2];
	ufs_daddr_t	daddr, lastblock;
 	int bb;		/* number of disk blocks in a block disk blocks */
 	int error, frags, i, nsize, osize, num;

	ip = VTOI(vp);
	fs = ip->i_lfs;

	/* 
	 * Three cases: it's a block beyond the end of file, it's a block in
	 * the file that may or may not have been assigned a disk address or
	 * we're writing an entire block.  Note, if the daddr is unassigned,
	 * the block might still have existed in the cache (if it was read
	 * or written earlier).  If it did, make sure we don't count it as a
	 * new block or zero out its contents.  If it did not, make sure
	 * we allocate any necessary indirect blocks.
	 * If we are writing a block beyond the end of the file, we need to
	 * check if the old last block was a fragment.  If it was, we need
	 * to rewrite it.
	 */

	*bpp = NULL;
	if (error = ufs_bmaparray(vp, lbn, &daddr, &indirs[0], &num, NULL ))
		return (error);

	/* Check for block beyond end of file and fragment extension needed. */
	lastblock = lblkno(fs, ip->i_size);
	if (lastblock < NDADDR && lastblock < lbn) {
		osize = blksize(fs, ip, lastblock);
		if (osize < fs->lfs_bsize && osize > 0) {
			if (error = lfs_fragextend(vp, osize, fs->lfs_bsize,
			    lastblock, &bp))
				return(error);
			ip->i_size = (lastblock + 1) * fs->lfs_bsize;
			vnode_pager_setsize(vp, (u_long)ip->i_size);
			ip->i_flag |= IN_CHANGE | IN_UPDATE;
			VOP_BWRITE(bp);
		}
	}

	bb = VFSTOUFS(vp->v_mount)->um_seqinc;
	if (daddr == UNASSIGNED)
		/* May need to allocate indirect blocks */
		for (i = 1; i < num; ++i)
			if (!indirs[i].in_exists) {
				ibp = getblk(vp, indirs[i].in_lbn, fs->lfs_bsize,
				    0, 0);
				if ((ibp->b_flags & (B_DONE | B_DELWRI))) 
					panic ("Indirect block should not exist");

				if (!ISSPACE(fs, bb, curproc->p_ucred)){
					ibp->b_flags |= B_INVAL;
					brelse(ibp);
					return(ENOSPC);
				} else {
					ip->i_blocks += bb;
					ip->i_lfs->lfs_bfree -= bb;
					clrbuf(ibp);
					if(error = VOP_BWRITE(ibp))
						return(error);
				}
			}

	/*
	 * If the block we are writing is a direct block, it's the last
	 * block in the file, and offset + iosize is less than a full
	 * block, we can write one or more fragments.  There are two cases:
	 * the block is brand new and we should allocate it the correct
	 * size or it already exists and contains some fragments and
	 * may need to extend it.
	 */
	if (lbn < NDADDR && lblkno(fs, ip->i_size) == lbn) {
		nsize = fragroundup(fs, offset + iosize);
		frags = numfrags(fs, nsize);
		bb = fragstodb(fs, frags);
		if (lblktosize(fs, lbn) == ip->i_size)
			/* Brand new block or fragment */
			*bpp = bp = getblk(vp, lbn, nsize, 0, 0);
		else {
			/* Extend existing block */
			if (error = lfs_fragextend(vp, (int)blksize(fs, ip, lbn), 
			    nsize, lbn, &bp))
				return(error);
			*bpp = bp;
		}
	} else {
		/*
		 * Get the existing block from the cache either because the
		 * block is 1) not a direct block or because it's not the last
		 * block in the file.
		 */
		frags = dbtofrags(fs, bb);
		*bpp = bp = getblk(vp, lbn, blksize(fs, ip, lbn), 0, 0);
	}

	/* 
	 * The block we are writing may be a brand new block
	 * in which case we need to do accounting (i.e. check
	 * for free space and update the inode number of blocks.
	 */
	if (!(bp->b_flags & (B_CACHE | B_DONE | B_DELWRI))) {
		if (daddr == UNASSIGNED) 
			if (!ISSPACE(fs, bb, curproc->p_ucred)) {
				bp->b_flags |= B_INVAL;
				brelse(bp);
				return(ENOSPC);
			} else {
				ip->i_blocks += bb;
				ip->i_lfs->lfs_bfree -= bb;
				if (iosize != fs->lfs_bsize)
					clrbuf(bp);
			}
		else if (iosize == fs->lfs_bsize)
			/* Optimization: I/O is unnecessary. */
			bp->b_blkno = daddr;
		else  {
			/*
			 * We need to read the block to preserve the
			 * existing bytes.
			 */
			bp->b_blkno = daddr;
			bp->b_flags |= B_READ;
			VOP_STRATEGY(bp);
			return(biowait(bp));
		}
	}
	return (0);
}

lfs_fragextend(vp, osize, nsize, lbn, bpp)
	struct vnode *vp;
	int osize;
	int nsize;
	daddr_t lbn;
	struct buf **bpp;
{
	struct inode *ip;
	struct lfs *fs;
	long bb;
	int error;

	ip = VTOI(vp);
	fs = ip->i_lfs;
	bb = (long)fragstodb(fs, numfrags(fs, nsize - osize));
	if (!ISSPACE(fs, bb, curproc->p_ucred)) {
		return(ENOSPC);
	}

	if (error = bread(vp, lbn, osize, NOCRED, bpp)) {
		brelse(*bpp);
		return(error);
	}
#ifdef QUOTA
	if (error = chkdq(ip, bb, curproc->p_ucred, 0)) {
		brelse(*bpp);
		return (error);
	}
#endif
	ip->i_blocks += bb;
	ip->i_flag |= IN_CHANGE | IN_UPDATE;
	fs->lfs_bfree -= fragstodb(fs, numfrags(fs, (nsize - osize)));
	allocbuf(*bpp, nsize);
	bzero((char *)((*bpp)->b_data) + osize, (u_int)(nsize - osize));
	return(0);
}
