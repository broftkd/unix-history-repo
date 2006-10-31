/*-
 * Copyright (c) 2006 Pawel Jakub Dawidek <pjd@FreeBSD.org>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Copyright (c) 1982, 1986, 1989, 1993
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/disklabel.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libufs.h>
#include <strings.h>
#include <err.h>
#include <assert.h>

#include "fsck.h"

struct cgchain {
	union {
		struct cg cgcu_cg;
		char cgcu_buf[MAXBSIZE];
	} cgc_union;
	int	cgc_busy;
	int	cgc_dirty;
	LIST_ENTRY(cgchain) cgc_next;
};
#define cgc_cg	cgc_union.cgcu_cg

#define	MAX_CACHED_CGS	1024
static unsigned ncgs = 0;
static LIST_HEAD(, cgchain) cglist = LIST_HEAD_INITIALIZER(&cglist);

static const char *devnam;
static struct uufsd *disk = NULL;
static struct fs *fs = NULL;
struct ufs2_dinode ufs2_zino;

static void putcgs(void);

/*
 * Write current block of inodes.
 */
static int
putino(struct uufsd *disk, ino_t inode)
{
	caddr_t inoblock;
	struct fs *fs;
	ssize_t ret;

	fs = &disk->d_fs;
	inoblock = disk->d_inoblock;

	assert(inoblock != NULL);
	assert(inode >= disk->d_inomin && inode <= disk->d_inomax);
	ret = bwrite(disk, fsbtodb(fs, ino_to_fsba(fs, inode)), inoblock,
	    fs->fs_bsize);

	return (ret == -1 ? -1 : 0);
}

/*
 * Return cylinder group from the cache or load it if it is not in the
 * cache yet.
 * Don't cache more than MAX_CACHED_CGS cylinder groups.
 */
static struct cgchain *
getcg(int cg)
{
	struct cgchain *cgc;

	assert(disk != NULL && fs != NULL);
	LIST_FOREACH(cgc, &cglist, cgc_next) {
		if (cgc->cgc_cg.cg_cgx == cg) {
			//printf("%s: Found cg=%d\n", __func__, cg);
			return (cgc);
		}
	}
	/*
	 * Our cache is full? Let's clean it up.
	 */
	if (ncgs >= MAX_CACHED_CGS) {
		//printf("%s: Flushing CGs.\n", __func__);
		putcgs();
	}
	cgc = malloc(sizeof(*cgc));
	if (cgc == NULL) {
		/*
		 * Cannot allocate memory?
		 * Let's put all currently loaded and not busy cylinder groups
		 * on disk and try again.
		 */
		//printf("%s: No memory, flushing CGs.\n", __func__);
		putcgs();
		cgc = malloc(sizeof(*cgc));
		if (cgc == NULL)
			err(1, "malloc(%zu)", sizeof(*cgc));
	}
	if (cgread1(disk, cg) == -1)
		err(1, "cgread1(%d)", cg);
	bcopy(&disk->d_cg, &cgc->cgc_cg, sizeof(cgc->cgc_union));
	cgc->cgc_busy = 0;
	cgc->cgc_dirty = 0;
	LIST_INSERT_HEAD(&cglist, cgc, cgc_next);
	ncgs++;
	//printf("%s: Read cg=%d\n", __func__, cg);
	return (cgc);
}

/*
 * Mark cylinder group as dirty - it will be written back on putcgs().
 */
static void
dirtycg(struct cgchain *cgc)
{

	cgc->cgc_dirty = 1;
}

/*
 * Mark cylinder group as busy - it will not be freed on putcgs().
 */
static void
busycg(struct cgchain *cgc)
{

	cgc->cgc_busy = 1;
}

/*
 * Unmark the given cylinder group as busy.
 */
static void
unbusycg(struct cgchain *cgc)
{

	cgc->cgc_busy = 0;
}

/*
 * Write back all dirty cylinder groups.
 * Free all non-busy cylinder groups.
 */
static void
putcgs(void)
{
	struct cgchain *cgc, *cgc2;

	assert(disk != NULL && fs != NULL);
	LIST_FOREACH_SAFE(cgc, &cglist, cgc_next, cgc2) {
		if (cgc->cgc_busy)
			continue;
		LIST_REMOVE(cgc, cgc_next);
		ncgs--;
		if (cgc->cgc_dirty) {
			bcopy(&cgc->cgc_cg, &disk->d_cg,
			    sizeof(cgc->cgc_union));
			if (cgwrite1(disk, cgc->cgc_cg.cg_cgx) == -1)
				err(1, "cgwrite1(%d)", cgc->cgc_cg.cg_cgx);
			//printf("%s: Wrote cg=%d\n", __func__,
			//    cgc->cgc_cg.cg_cgx);
		}
		free(cgc);
	}
}

#if 0
/*
 * Free all non-busy cylinder groups without storing the dirty ones.
 */
static void
cancelcgs(void)
{
	struct cgchain *cgc;

	assert(disk != NULL && fs != NULL);
	while ((cgc = LIST_FIRST(&cglist)) != NULL) {
		if (cgc->cgc_busy)
			continue;
		LIST_REMOVE(cgc, cgc_next);
		//printf("%s: Canceled cg=%d\n", __func__, cgc->cgc_cg.cg_cgx);
		free(cgc);
	}
}
#endif

/*
 * Open the given provider, load statistics.
 */
static void
getdisk(void)
{
	int i;

	if (disk != NULL)
		return;
	disk = malloc(sizeof(*disk));
	if (disk == NULL)
		err(1, "malloc(%zu)", sizeof(*disk));
	if (ufs_disk_fillout(disk, devnam) == -1) {
		err(1, "ufs_disk_fillout(%s) failed: %s", devnam,
		    disk->d_error);
	}
	fs = &disk->d_fs;
	fs->fs_csp = malloc((size_t)fs->fs_cssize);
	if (fs->fs_csp == NULL)
		err(1, "malloc(%zu)", (size_t)fs->fs_cssize);
	bzero(fs->fs_csp, (size_t)fs->fs_cssize);
	for (i = 0; i < fs->fs_cssize; i += fs->fs_bsize) {
		if (bread(disk, fsbtodb(fs, fs->fs_csaddr + numfrags(fs, i)),
		    (void *)(((char *)fs->fs_csp) + i),
		    (size_t)(fs->fs_cssize - i < fs->fs_bsize ? fs->fs_cssize - i : fs->fs_bsize)) == -1) {
			err(1, "bread: %s", disk->d_error);
		}
	}
	if (fs->fs_contigsumsize > 0) {
		fs->fs_maxcluster = malloc(fs->fs_ncg * sizeof(int32_t));
		if (fs->fs_maxcluster == NULL)
			err(1, "malloc(%zu)", fs->fs_ncg * sizeof(int32_t));
		for (i = 0; i < fs->fs_ncg; i++)
			fs->fs_maxcluster[i] = fs->fs_contigsumsize;
	}
}

/*
 * Mark file system as clean, write the super-block back, close the disk.
 */
static void
closedisk(void)
{

	free(fs->fs_csp);
	if (fs->fs_contigsumsize > 0) {
		free(fs->fs_maxcluster);
		fs->fs_maxcluster = NULL;
	}
	fs->fs_clean = 1;
	if (sbwrite(disk, 0) == -1)
		err(1, "sbwrite(%s)", devnam);
	if (ufs_disk_close(disk) == -1)
		err(1, "ufs_disk_close(%s)", devnam);
	free(disk);
	disk = NULL;
	fs = NULL;
}

/*
 * Write the statistics back, call closedisk().
 */
static void
putdisk(void)
{
	int i;

	assert(disk != NULL && fs != NULL);
	for (i = 0; i < fs->fs_cssize; i += fs->fs_bsize) {
		if (bwrite(disk, fsbtodb(fs, fs->fs_csaddr + numfrags(fs, i)),
		    (void *)(((char *)fs->fs_csp) + i),
		    (size_t)(fs->fs_cssize - i < fs->fs_bsize ? fs->fs_cssize - i : fs->fs_bsize)) == -1) {
			err(1, "bwrite: %s", disk->d_error);
		}
	}
	closedisk();
}

#if 0
/*
 * Free memory, close the disk, but don't write anything back.
 */
static void
canceldisk(void)
{
	int i;

	assert(disk != NULL && fs != NULL);
	free(fs->fs_csp);
	if (fs->fs_contigsumsize > 0)
		free(fs->fs_maxcluster);
	if (ufs_disk_close(disk) == -1)
		err(1, "ufs_disk_close(%s)", devnam);
	free(disk);
	disk = NULL;
	fs = NULL;
}
#endif

static int
isblock(unsigned char *cp, ufs1_daddr_t h)
{
	unsigned char mask;

	switch ((int)fs->fs_frag) {
	case 8:
		return (cp[h] == 0xff);
	case 4:
		mask = 0x0f << ((h & 0x1) << 2);
		return ((cp[h >> 1] & mask) == mask);
	case 2:
		mask = 0x03 << ((h & 0x3) << 1);
		return ((cp[h >> 2] & mask) == mask);
	case 1:
		mask = 0x01 << (h & 0x7);
		return ((cp[h >> 3] & mask) == mask);
	default:
		assert(!"isblock: invalid number of fragments");
	}
	return (0);
}

/*
 * put a block into the map
 */
static void
setblock(unsigned char *cp, ufs1_daddr_t h)
{

	switch ((int)fs->fs_frag) {
	case 8:
		cp[h] = 0xff;
		return;
	case 4:
		cp[h >> 1] |= (0x0f << ((h & 0x1) << 2));
		return;
	case 2:
		cp[h >> 2] |= (0x03 << ((h & 0x3) << 1));
		return;
	case 1:
		cp[h >> 3] |= (0x01 << (h & 0x7));
		return;
	default:
		assert(!"setblock: invalid number of fragments");
	}
}

/*
 * check if a block is free
 */
static int
isfreeblock(u_char *cp, ufs1_daddr_t h)
{

	switch ((int)fs->fs_frag) {
	case 8:
		return (cp[h] == 0);
	case 4:
		return ((cp[h >> 1] & (0x0f << ((h & 0x1) << 2))) == 0);
	case 2:
		return ((cp[h >> 2] & (0x03 << ((h & 0x3) << 1))) == 0);
	case 1:
		return ((cp[h >> 3] & (0x01 << (h & 0x7))) == 0);
	default:
		assert(!"isfreeblock: invalid number of fragments");
	}
	return (0);
}

/*
 * Update the frsum fields to reflect addition or deletion
 * of some frags.
 */
void
fragacct(int fragmap, int32_t fraglist[], int cnt)
{
	int inblk;
	int field, subfield;
	int siz, pos;

	inblk = (int)(fragtbl[fs->fs_frag][fragmap]) << 1;
	fragmap <<= 1;
	for (siz = 1; siz < fs->fs_frag; siz++) {
		if ((inblk & (1 << (siz + (fs->fs_frag % NBBY)))) == 0)
			continue;
		field = around[siz];
		subfield = inside[siz];
		for (pos = siz; pos <= fs->fs_frag; pos++) {
			if ((fragmap & field) == subfield) {
				fraglist[siz] += cnt;
				pos += siz;
				field <<= siz;
				subfield <<= siz;
			}
			field <<= 1;
			subfield <<= 1;
		}
	}
}

static void
clusteracct(struct cg *cgp, ufs1_daddr_t blkno)
{
	int32_t *sump;
	int32_t *lp;
	u_char *freemapp, *mapp;
	int i, start, end, forw, back, map, bit;

	if (fs->fs_contigsumsize <= 0)
		return;
	freemapp = cg_clustersfree(cgp);
	sump = cg_clustersum(cgp);
	/*
	 * Clear the actual block.
	 */
	setbit(freemapp, blkno);
	/*
	 * Find the size of the cluster going forward.
	 */
	start = blkno + 1;
	end = start + fs->fs_contigsumsize;
	if (end >= cgp->cg_nclusterblks)
		end = cgp->cg_nclusterblks;
	mapp = &freemapp[start / NBBY];
	map = *mapp++;
	bit = 1 << (start % NBBY);
	for (i = start; i < end; i++) {
		if ((map & bit) == 0)
			break;
		if ((i & (NBBY - 1)) != (NBBY - 1)) {
			bit <<= 1;
		} else {
			map = *mapp++;
			bit = 1;
		}
	}
	forw = i - start;
	/*
	 * Find the size of the cluster going backward.
	 */
	start = blkno - 1;
	end = start - fs->fs_contigsumsize;
	if (end < 0)
		end = -1;
	mapp = &freemapp[start / NBBY];
	map = *mapp--;
	bit = 1 << (start % NBBY);
	for (i = start; i > end; i--) {
		if ((map & bit) == 0)
			break;
		if ((i & (NBBY - 1)) != 0) {
			bit >>= 1;
		} else {
			map = *mapp--;
			bit = 1 << (NBBY - 1);
		}
	}
	back = start - i;
	/*
	 * Account for old cluster and the possibly new forward and
	 * back clusters.
	 */
	i = back + forw + 1;
	if (i > fs->fs_contigsumsize)
		i = fs->fs_contigsumsize;
	sump[i]++;
	if (back > 0)
		sump[back]--;
	if (forw > 0)
		sump[forw]--;
	/*
	 * Update cluster summary information.
	 */
	lp = &sump[fs->fs_contigsumsize];
	for (i = fs->fs_contigsumsize; i > 0; i--)
		if (*lp-- > 0)
			break;
	fs->fs_maxcluster[cgp->cg_cgx] = i;
}

static void
blkfree(ufs2_daddr_t bno, long size)
{
	struct cgchain *cgc;
	struct cg *cgp;
	ufs1_daddr_t fragno, cgbno;
	int i, cg, blk, frags, bbase;
	u_int8_t *blksfree;

	cg = dtog(fs, bno);
	cgc = getcg(cg);
	dirtycg(cgc);
	cgp = &cgc->cgc_cg;
	cgbno = dtogd(fs, bno);
	blksfree = cg_blksfree(cgp);
	if (size == fs->fs_bsize) {
		fragno = fragstoblks(fs, cgbno);
		if (!isfreeblock(blksfree, fragno))
			assert(!"blkfree: freeing free block");
		setblock(blksfree, fragno);
		clusteracct(cgp, fragno);
		cgp->cg_cs.cs_nbfree++;
		fs->fs_cstotal.cs_nbfree++;
		fs->fs_cs(fs, cg).cs_nbfree++;
	} else {
		bbase = cgbno - fragnum(fs, cgbno);
		/*
		 * decrement the counts associated with the old frags
		 */
		blk = blkmap(fs, blksfree, bbase);
		fragacct(blk, cgp->cg_frsum, -1);
		/*
		 * deallocate the fragment
		 */
		frags = numfrags(fs, size);
		for (i = 0; i < frags; i++) {
			if (isset(blksfree, cgbno + i))
				assert(!"blkfree: freeing free frag");
			setbit(blksfree, cgbno + i);
		}
		cgp->cg_cs.cs_nffree += i;
		fs->fs_cstotal.cs_nffree += i;
		fs->fs_cs(fs, cg).cs_nffree += i;
		/*
		 * add back in counts associated with the new frags
		 */
		blk = blkmap(fs, blksfree, bbase);
		fragacct(blk, cgp->cg_frsum, 1);
		/*
		 * if a complete block has been reassembled, account for it
		 */
		fragno = fragstoblks(fs, bbase);
		if (isblock(blksfree, fragno)) {
			cgp->cg_cs.cs_nffree -= fs->fs_frag;
			fs->fs_cstotal.cs_nffree -= fs->fs_frag;
			fs->fs_cs(fs, cg).cs_nffree -= fs->fs_frag;
			clusteracct(cgp, fragno);
			cgp->cg_cs.cs_nbfree++;
			fs->fs_cstotal.cs_nbfree++;
			fs->fs_cs(fs, cg).cs_nbfree++;
		}
	}
}

/*
 * Recursively free all indirect blocks.
 */
static void
freeindir(ufs2_daddr_t blk, int level)
{
	char sblks[MAXBSIZE];
	ufs2_daddr_t *blks;
	int i;

	if (bread(disk, fsbtodb(fs, blk), (void *)&sblks, (size_t)fs->fs_bsize) == -1)
		err(1, "bread: %s", disk->d_error);
	blks = (ufs2_daddr_t *)&sblks;
	for (i = 0; i < howmany(fs->fs_bsize, sizeof(ufs2_daddr_t)); i++) {
		if (blks[i] == 0)
			break;
		if (level == 0)
			blkfree(blks[i], fs->fs_bsize);
		else
			freeindir(blks[i], level - 1);
	}
	blkfree(blk, fs->fs_bsize);
}

#define	dblksize(fs, dino, lbn) \
	((dino)->di_size >= smalllblktosize(fs, (lbn) + 1) \
	    ? (fs)->fs_bsize \
	    : fragroundup(fs, blkoff(fs, (dino)->di_size)))

/*
 * Free all blocks associated with the given inode.
 */
static void
clear_inode(struct ufs2_dinode *dino)
{
	ufs2_daddr_t bn;
	int extblocks, i, level;
	off_t osize;
	long bsize;

	extblocks = 0;
	if (fs->fs_magic == FS_UFS2_MAGIC && dino->di_extsize > 0)
		extblocks = btodb(fragroundup(fs, dino->di_extsize));
	/* deallocate external attributes blocks */
	if (extblocks > 0) {
		osize = dino->di_extsize;
		dino->di_blocks -= extblocks;
		dino->di_extsize = 0;
		for (i = 0; i < NXADDR; i++) {
			if (dino->di_extb[i] == 0)
				continue;
			blkfree(dino->di_extb[i], sblksize(fs, osize, i));
		}
	}
#define	SINGLE	0	/* index of single indirect block */
#define	DOUBLE	1	/* index of double indirect block */
#define	TRIPLE	2	/* index of triple indirect block */
	/* deallocate indirect blocks */
	for (level = SINGLE; level <= TRIPLE; level++) {
		if (dino->di_ib[level] == 0)
			break;
		freeindir(dino->di_ib[level], level);
	}
	/* deallocate direct blocks and fragments */
	for (i = 0; i < NDADDR; i++) {
		bn = dino->di_db[i];
		if (bn == 0)
			continue;
		bsize = dblksize(fs, dino, i);
		blkfree(bn, bsize);
	}
}

void
gjournal_check(const char *filesys)
{
	struct ufs2_dinode *dino;
	struct cgchain *cgc;
	struct cg *cgp;
	uint8_t *inosused, *blksfree;
	ino_t cino, ino;
	int cg, mode;

	devnam = filesys;
	getdisk();
	/* Are there any unreferenced inodes in this cylinder group? */
	if (fs->fs_unrefs == 0) {
		//printf("No unreferenced inodes.\n");
		closedisk();
		return;
	}

	for (cg = 0; cg < fs->fs_ncg; cg++) {
		/* Show progress if requested. */
		if (got_siginfo) {
			printf("%s: phase j: cyl group %d of %d (%d%%)\n",
			    cdevname, cg, fs->fs_ncg, cg * 100 / fs->fs_ncg);
			got_siginfo = 0;
		}
		if (got_sigalarm) {
			setproctitle("%s pj %d%%", cdevname,
			     cg * 100 / fs->fs_ncg);
			got_sigalarm = 0;
		}
		cgc = getcg(cg);
		cgp = &cgc->cgc_cg;
		/* Are there any unreferenced inodes in this cylinder group? */
		if (cgp->cg_unrefs == 0)
			continue;
		//printf("Analizing cylinder group %d (count=%d)\n", cg, cgp->cg_unrefs);
		/*
		 * We are going to modify this cylinder group, so we want it to
		 * be written back.
		 */
		dirtycg(cgc);
		/* We don't want it to be freed in the meantime. */
		busycg(cgc);
		inosused = cg_inosused(cgp);
		blksfree = cg_blksfree(cgp);
		/*
		 * Now go through the list of all inodes in this cylinder group
		 * to find unreferenced ones.
		 */
		for (cino = 0; cino < fs->fs_ipg; cino++) {
			ino = fs->fs_ipg * cg + cino;
			/* Unallocated? Skip it. */
			if (isclr(inosused, cino))
				continue;
			if (getino(disk, (void **)&dino, ino, &mode) == -1)
				err(1, "getino(cg=%d ino=%d)", cg, ino);
			/* Not a regular file nor directory? Skip it. */
			if (!S_ISREG(dino->di_mode) && !S_ISDIR(dino->di_mode))
				continue;
			/* Has reference(s)? Skip it. */
			if (dino->di_nlink > 0)
				continue;
			//printf("Clearing inode=%d (size=%jd)\n", ino, (intmax_t)dino->di_size);
			/* Free inode's blocks. */
			clear_inode(dino);
			/* Deallocate it. */
			clrbit(inosused, cino);
			/* Update position of last used inode. */
			if (ino < cgp->cg_irotor)
				cgp->cg_irotor = ino;
			/* Update statistics. */
			cgp->cg_cs.cs_nifree++;
			fs->fs_cs(fs, cg).cs_nifree++;
			fs->fs_cstotal.cs_nifree++;
			cgp->cg_unrefs--;
			fs->fs_unrefs--;
			/* If this is directory, update related statistics. */
			if (S_ISDIR(dino->di_mode)) {
				cgp->cg_cs.cs_ndir--;
				fs->fs_cs(fs, cg).cs_ndir--;
				fs->fs_cstotal.cs_ndir--;
			}
			/* Zero-fill the inode. */
			*dino = ufs2_zino;
			/* Write the inode back. */
			if (putino(disk, ino) == -1)
				err(1, "putino(cg=%d ino=%d)", cg, ino);
			if (cgp->cg_unrefs == 0) {
				//printf("No more unreferenced inodes in cg=%d.\n", cg);
				break;
			}
		}
		/*
		 * We don't need this cylinder group anymore, so feel free to
		 * free it if needed.
		 */
		unbusycg(cgc);
		/*
		 * If there are no more unreferenced inodes, there is no need to
		 * check other cylinder groups.
		 */
		if (fs->fs_unrefs == 0) {
			//printf("No more unreferenced inodes (cg=%d/%d).\n", cg,
			//    fs->fs_ncg);
			break;
		}
	}
	/* Write back modified cylinder groups. */
	putcgs();
	/* Write back updated statistics and super-block. */
	putdisk();
}
