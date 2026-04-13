/*
 * Copyright (c) 2000-2008,2010-2011,2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* $FreeBSD: src/sys/msdosfs/msdosfs_denode.c,v 1.48 2000/05/05 09:58:34 phk Exp $ */
/*	$NetBSD: msdosfs_denode.c,v 1.28 1998/02/10 14:10:00 mrg Exp $	*/

/*-
 * Copyright (C) 1994, 1995, 1997 Wolfgang Solfrank.
 * Copyright (C) 1994, 1995, 1997 TooLs GmbH.
 * All rights reserved.
 * Original code by Paul Popelka (paulp@uts.amdahl.com) (see below).
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
 *	This product includes software developed by TooLs GmbH.
 * 4. The name of TooLs GmbH may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TOOLS GMBH ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TOOLS GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Written by Paul Popelka (paulp@uts.amdahl.com)
 *
 * You can do anything you want with this software, just don't say you wrote
 * it, and don't remove this notice.
 *
 * This software is provided "as is".
 *
 * The author supplies this software to be publicly redistributed on the
 * understanding that the author is not responsible for the correct
 * functioning of this software in any circumstances and is not liable for
 * any damages caused by this software.
 *
 * October 1992
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/ubc.h>
#include <sys/namei.h>
#include <mach/boolean.h>
#include <libkern/OSMalloc.h>

#include "bpb.h"
#include "msdosfsmount.h"
#include "direntry.h"
#include "denode.h"
#include "fat.h"
#include "msdosfs_kdebug.h"

#ifndef DEBUG
#define DEBUG 0
#endif

OSMallocTag  msdosfs_node_tag = NULL;
static lck_mtx_t *msdosfs_hash_lock = NULL;

static struct denode **dehashtbl = NULL;
static unsigned long dehash;			/* size of hash table - 1 */
#define	DEHASH(dev, dcl, doff)	(dehashtbl[(minor(dev) + (dcl) + (doff)) & dehash])

union _qcvt {
	quad_t qcvt;
	int32_t val[2];
};
#define SETHIGH(q, h) { \
	union _qcvt tmp; \
	tmp.qcvt = (q); \
	tmp.val[_QUAD_HIGHWORD] = (h); \
	(q) = tmp.qcvt; \
}
#define SETLOW(q, l) { \
	union _qcvt tmp; \
	tmp.qcvt = (q); \
	tmp.val[_QUAD_LOWWORD] = (l); \
	(q) = tmp.qcvt; \
}

struct denode *msdosfs_hash_get(dev_t dev, uint32_t dirclust, uint32_t diroff);
void msdosfs_hash_insert(struct denode *dep);
void msdosfs_hash_remove(struct denode *dep);

void msdosfs_hash_init(void)
{
	msdosfs_hash_lock = lck_mtx_alloc_init(msdosfs_lck_grp, msdosfs_lck_attr);
    msdosfs_node_tag = OSMalloc_Tagalloc("msdosfs denode", OSMT_DEFAULT);
    dehashtbl = hashinit(desiredvnodes, M_TEMP, &dehash);
}

void msdosfs_hash_uninit(void)
{
	if (dehashtbl)
		FREE(dehashtbl, M_TEMP);
	if (msdosfs_node_tag)
		OSMalloc_Tagfree(msdosfs_node_tag);
	if (msdosfs_hash_lock)
		lck_mtx_free(msdosfs_hash_lock, msdosfs_lck_grp);
}

/*
 * Look for a given denode in the denode hash.  If found,
 * return with a use_count reference on the vnode.
 *
 * Assumes the msdosfs_hash_lock has already been acquired.
 */
struct denode *msdosfs_hash_get(dev_t dev, uint32_t dirclust, uint32_t diroff)
{
	int error;
	struct denode *dep;
	vnode_t vp;
	u_int32_t vid;
	struct denode *found = 0;	/* Only used if DEBUG is non-zero */

loop:
	for (dep = DEHASH(dev, dirclust, diroff); dep; dep = dep->de_next)
	{
		if (dirclust == dep->de_dirclust
		    && diroff == dep->de_diroffset
		    && dev == dep->de_dev
		    && dep->de_refcnt != 0)
		{
			if (ISSET(dep->de_flag, DE_INIT))
			{
				/*
				 * Denode is being initialized. Wait for it to complete.
				 * We unlock the hash lock while sleeping to avoid deadlock.
				 */
				SET(dep->de_flag, DE_WAITINIT);
				msleep(dep, msdosfs_hash_lock, PINOD, "msdosfs_hash_get", 0);
				goto loop;
			}
			
			/* If the vnode has been deleted, ignore it */
			if (dep->de_refcnt <= 0)
			{
				if (DEBUG)
					printf("msdosfs_hash_get: found deleted object\n");
				msdosfs_hash_remove(dep);
				goto loop;
			}
			
			/*
			 * Make sure the vnode isn't being terminated.  NOTE: we have to
			 * drop the hash lock to avoid deadlock with other threads that
			 * may be trying to terminate this vnode.
			 */
			vp = DETOV(dep);
			vid = vnode_vid(vp);
			lck_mtx_unlock(msdosfs_hash_lock);
			error = vnode_getwithvid(vp, vid);
			lck_mtx_lock(msdosfs_hash_lock);
			if (error)
				goto loop;
			
			if (DEBUG)
			{
				if (found)
					panic("msdosfs_hash_get: multiple denodes");
				found = dep;
				continue;
			}
			return dep;
		}
	}
	if (DEBUG)
		return found;

	return NULL;
}

/*
 * Insert a given denode in the denode hash.
 *
 * Assumes the msdosfs_hash_lock has already been acquired.  Assumes the
 * denode is not currently in the hash.
 */
void msdosfs_hash_insert(struct denode *dep)
{
	struct denode **depp, *deq;

	if (DEBUG)
	{
		struct denode *found;
		found = msdosfs_hash_get(dep->de_dev, dep->de_dirclust, dep->de_diroffset);
		if (found)
			panic("msdosfs_hash_insert: denode already in hash? found=%p", found);
	}
	
	depp = &DEHASH(dep->de_dev, dep->de_dirclust, dep->de_diroffset);
	deq = *depp;
	if (deq)
		deq->de_prev = &dep->de_next;
	dep->de_next = deq;
	dep->de_prev = depp;
	*depp = dep;
}

/*
 * Remove a given denode in the denode hash.
 *
 * Assumes the msdosfs_hash_lock has already been acquired.
 */
void msdosfs_hash_remove(struct denode *dep)
{
	struct denode *deq;

	deq = dep->de_next;
	if (deq)
		deq->de_prev = dep->de_prev;
	*dep->de_prev = deq;
}

/*
 * If msdosfs_deget() succeeds it returns with an io_count reference on the denode's
 * corresponding vnode.  If not returning that vnode to VFS, then be sure
 * to vnode_put it!
 *
 * pmp	     - address of msdosfsmount structure of the filesystem containing
 *	       the denode of interest.  The pm_dev field and the address of
 *	       the msdosfsmount structure are used.
 * dirclust  - which cluster bp contains, if dirclust is 0 (root directory)
 *	       diroffset is relative to the beginning of the root directory,
 *	       otherwise it is cluster relative.
 * diroffset - offset from start of parent directory to the directory entry we want
 * dvp		 - parent directory
 * cnp		 - name used to look up the denode
 * depp	     - returns the address of the gotten denode.
 */
int msdosfs_deget(
	struct msdosfsmount *pmp,	/* so we know the maj/min number */
	uint32_t dirclust,			/* cluster this dir entry came from */
	uint32_t diroffset,			/* index of entry within the cluster */
	vnode_t dvp,				/* parent directory */
	struct componentname *cnp,	/* name used to find this node */
	struct denode **depp,		/* returns the addr of the gotten denode */
	vfs_context_t context)
{
	int error;
	dev_t dev = pmp->pm_dev;
	struct mount *mntp = pmp->pm_mountp;
	struct denode *dep;
	struct dosdirentry *direntptr;
	struct buf *bp = NULL;
	struct timeval tv;
	struct vnode_fsparam vfsp;
	enum vtype vtype;

	/*
	 * On FAT32 filesystems, root is a (more or less) normal
	 * directory
	 */
	if (FAT32(pmp) && dirclust == MSDOSFSROOT)
		dirclust = pmp->pm_rootdirblk;

	/*
	 * Lock the hash so that we can't race against another msdosfs_deget(),
	 * VNOP_RECLAIM, etc.
	 */
	lck_mtx_lock(msdosfs_hash_lock);
	
	/*
	 * See if the denode is already in our hash.  If so,
	 * just return it.
	 */
	dep = msdosfs_hash_get(dev, dirclust, diroffset);
	if (dep != NULL) {
		*depp = dep;
		if (dvp && cnp && (cnp->cn_flags & MAKEENTRY) && (dep->de_flag & DE_ROOT) == 0)
			cache_enter(dvp, DETOV(dep), cnp);
		if (dep->de_parent == NULL && dvp != NULLVP && (dep->de_flag & DE_ROOT) == 0)
		{
			if (DEBUG) printf("msdosfs_deget: fixing de_parent\n");
			dep->de_parent = VTODE(dvp);
		}
		lck_mtx_unlock(msdosfs_hash_lock);
		return 0;
	}

	/*
	 * There was nothing in the hash.  Before we block on I/O,
	 * we need to insert a matching denode marked as being
	 * initialized.  Any other msdosfs_deget() will block until we're
	 * finished here, and either find the fully initialized
	 * denode, or none at all.
	 */
	dep = OSMalloc(sizeof(struct denode), msdosfs_node_tag);
	if (dep == NULL) {
		*depp = NULL;
		lck_mtx_unlock(msdosfs_hash_lock);
		return ENOMEM;
	}
	bzero(dep, sizeof *dep);
	dep->de_lock = lck_mtx_alloc_init(msdosfs_lck_grp, msdosfs_lck_attr);
	dep->de_cluster_lock = lck_mtx_alloc_init(msdosfs_lck_grp, msdosfs_lck_attr);
	dep->de_pmp = pmp;
	dep->de_devvp = pmp->pm_devvp;
	dep->de_dev = dev;
	dep->de_dirclust = dirclust;
	dep->de_diroffset = diroffset;
	dep->de_refcnt = 1;
	SET(dep->de_flag, DE_INIT);
	msdosfs_hash_insert(dep);
	lck_mtx_unlock(msdosfs_hash_lock);
	
	vfsp.vnfs_markroot = 0;	/* Assume not the root */
	
	/*
	 * Copy the directory entry into the denode
	 */
	if ((dirclust == MSDOSFSROOT
	     || (FAT32(pmp) && dirclust == pmp->pm_rootdirblk))
	    && diroffset == MSDOSFSROOT_OFS) {
		/*
		 * Directory entry for the root directory. There isn't one,
		 * so we manufacture one. We should probably rummage
		 * through the root directory and find a label entry (if it
		 * exists), and then use the time and date from that entry
		 * as the time and date for the root denode.
		 */
		vfsp.vnfs_markroot = 1;
		SET(dep->de_flag, DE_ROOT);

		bcopy("           ", dep->de_Name, 11);
		dep->de_Attributes = ATTR_DIRECTORY;
		dep->de_LowerCase = 0;
		if (FAT32(pmp))
			dep->de_StartCluster = pmp->pm_rootdirblk;
			/* de_FileSize will be filled in further down */
		else {
			dep->de_StartCluster = MSDOSFSROOT;
			dep->de_FileSize = pmp->pm_rootdirsize * pmp->pm_BlockSize;
		}
		/*
		 * fill in time and date so that msdosfs_dos2unixtime() doesn't
		 * spit up when called from msdosfs_vnop_getattr() with root
		 * denode
		 */
		dep->de_CHun = 0;
		dep->de_CTime = 0x0000;	/* 00:00:00	 */
		dep->de_CDate = (0 << DD_YEAR_SHIFT) | (1 << DD_MONTH_SHIFT)
		    | (1 << DD_DAY_SHIFT);
		/* Jan 1, 1980	 */
		dep->de_ADate = dep->de_CDate;
		dep->de_MTime = dep->de_CTime;
		dep->de_MDate = dep->de_CDate;
		/* leave the other fields as garbage */
                
		/*
		 * If there is a volume label entry, then grab the times from it instead.
		 */
		if (pmp->pm_label_cluster != CLUST_EOFE) {
			error = msdosfs_readep(pmp, pmp->pm_label_cluster, pmp->pm_label_offset,
					&bp, &direntptr, context);
			if (!error) {
				dep->de_CHun = direntptr->deCHundredth;
				dep->de_CTime = getuint16(direntptr->deCTime);
				dep->de_CDate = getuint16(direntptr->deCDate);
				dep->de_ADate = getuint16(direntptr->deADate);
				dep->de_MTime = getuint16(direntptr->deMTime);
				dep->de_MDate = getuint16(direntptr->deMDate);
				buf_brelse(bp);
				bp = NULL;
			}
		}
	} else {
		/* Not the root directory */
		
		/* Get the non-date values from the given directory entry */
		error = msdosfs_readep(pmp, dirclust, diroffset, &bp, &direntptr, context);
		if (error) goto fail;
		bcopy(direntptr->deName, dep->de_Name, 11);
		dep->de_Attributes = direntptr->deAttributes;
		dep->de_LowerCase = direntptr->deLowerCase;
		dep->de_StartCluster = getuint16(direntptr->deStartCluster);
		if (FAT32(pmp))
			dep->de_StartCluster |= getuint16(direntptr->deHighClust) << 16;
		dep->de_FileSize = getuint32(direntptr->deFileSize);

		/* For directories, the dates/times come from its "." entry */
		if (direntptr->deAttributes & ATTR_DIRECTORY)
		{
			buf_brelse(bp);
			bp = NULL;
			if (DEBUG)
			{
				if (dep->de_StartCluster < CLUST_FIRST || dep->de_StartCluster > pmp->pm_maxcluster)
					panic("msdosfs_deget: directory de_StartCluster=%u", dep->de_StartCluster);
			}
			error = msdosfs_readep(pmp, dep->de_StartCluster, 0, &bp, &direntptr, context);
			if (error) goto fail;
		}
		
		/* Copy the dates and times */
		dep->de_CHun = direntptr->deCHundredth;
		dep->de_CTime = getuint16(direntptr->deCTime);
		dep->de_CDate = getuint16(direntptr->deCDate);
		dep->de_ADate = getuint16(direntptr->deADate);
		dep->de_MTime = getuint16(direntptr->deMTime);
		dep->de_MDate = getuint16(direntptr->deMDate);
		
		buf_brelse(bp);
		bp = NULL;
	}

	/*
	 * Determine initial values for vnode fields, and finish
	 * populating the denode.
	 */
	if (dep->de_Attributes & ATTR_DIRECTORY) {
		/*
		 * Since DOS directory entries that describe directories
		 * have 0 in the filesize field, we take this opportunity
		 * to find out the length of the directory and plug it into
		 * the denode structure.
		 */
		uint32_t size;

		vtype = VDIR;
		if (dep->de_StartCluster != MSDOSFSROOT) {
			error = msdosfs_pcbmap(dep, 0xFFFFFFFF, 1, NULL, &size, &dep->de_LastCluster);
			if (error == E2BIG) {
				dep->de_FileSize = de_cn2off(pmp, size);
				error = 0;
			}
			if (error)
				goto fail;
		}
	} else {
		/*
		 * We found a regular file.  See if it is really a symlink.
		 */
		vtype = msdosfs_check_link(dep, context);
	}
	getmicrouptime(&tv);
	SETHIGH(dep->de_modrev, (int32_t)tv.tv_sec);
	SETLOW(dep->de_modrev, tv.tv_usec * 4294);

	/* Remember the parent denode */
	dep->de_parent = (dvp != NULLVP) ? VTODE(dvp) : NULL;
	
	/*
	 * Create the vnode
	 */
	vfsp.vnfs_mp = mntp;
	vfsp.vnfs_vtype = vtype;
	vfsp.vnfs_str = "msdosfs";
	vfsp.vnfs_dvp = dvp;
	vfsp.vnfs_fsnode = dep;
	vfsp.vnfs_cnp = cnp;
	vfsp.vnfs_vops = msdosfs_vnodeop_p;
	vfsp.vnfs_rdev = 0;		/* msdosfs doesn't support block devices */
	vfsp.vnfs_filesize = dep->de_FileSize;
	if (dvp && cnp && (cnp->cn_flags & MAKEENTRY))
		vfsp.vnfs_flags = 0;
	else
		vfsp.vnfs_flags = VNFS_NOCACHE;
	/* vfsp.vnfs_markroot was set or cleared above */
	vfsp.vnfs_marksystem = 0;	/* msdosfs has no "system" vnodes */
	
	error = vnode_create(VNCREATE_FLAVOR, VCREATESIZE, &vfsp, &dep->de_vnode);
	if (error)
		goto fail;

	/*
	 * Take an "fs"  reference on the new vnode on
	 * behalf of the denode.
	 */
	vnode_addfsref(dep->de_vnode);
	
	/*
	 * Return it.  We're done.
	 */
	*depp = dep;

	CLR(dep->de_flag, DE_INIT);
	if (ISSET(dep->de_flag, DE_WAITINIT))
		wakeup(dep);
	
	return 0;

fail:
	if (bp)
		buf_brelse(bp);

	lck_mtx_lock(msdosfs_hash_lock);
	msdosfs_hash_remove(dep);
	lck_mtx_unlock(msdosfs_hash_lock);
	
	if (ISSET(dep->de_flag, DE_WAITINIT))
		wakeup(dep);
	
	OSFree(dep, sizeof *dep, msdosfs_node_tag);
	
	return error;
}

int msdosfs_deupdat(struct denode *dep, int waitfor, vfs_context_t context)
{
#pragma unused (waitfor)
    int error = 0;
    int isRoot = 0;
    int isDir = 0;
    uint32_t dirclust, diroffset;
    struct buf *bp;
    struct dosdirentry *dirp;
    struct timespec ts;
    struct msdosfsmount *pmp = dep->de_pmp;
    
    if (vnode_vfsisrdonly(DETOV(dep)))
	return (0);
    getnanotime(&ts);
    DETIMES(dep, &ts, &ts, &ts);
    if ((dep->de_flag & DE_MODIFIED) == 0)
	return 0;
    
    isRoot = dep->de_flag & DE_ROOT;
    isDir = dep->de_Attributes & ATTR_DIRECTORY;
    
    if (isRoot && pmp->pm_label_cluster == CLUST_EOFE)
	return 0;				/* There is no volume label entry to update. */
    if (dep->de_refcnt <= 0)
	return 0;				/* Object was deleted from the namespace. */
    
    /*
     * Update the fields, like times, which go in the directory entry.
     *
     * Windows never updates the times in a directory's directory entry.
     * Since we wish to update them persistently, we use the times from
     * the "." entry in the directory.  Since the root doesn't have a "."
     * entry, we use the times in the volume label entry (if there is one).
     */
    if (dep->de_flag & DE_MODIFIED)
    {
	dep->de_flag &= ~DE_MODIFIED;
	
	/*
	 * Read in the directory entry to be updated.
	 * For files, this is just the file's directory entry (in its parent).
	 * For directories, this is the "." entry inside the directory.
	 * For the root, this is the volume label's directory entry.
	 */
	if (isDir)
	{
	    if (isRoot)
	    {
		dirclust = pmp->pm_label_cluster;
		diroffset = pmp->pm_label_offset;
	    }
	    else
	    {
		dirclust = dep->de_StartCluster;
		diroffset = 0;
	    }
	}
	else
	{
	    dirclust = dep->de_dirclust;
	    diroffset = dep->de_diroffset;
	}
	error = msdosfs_readep(pmp, dirclust, diroffset, &bp, &dirp, context);
	if (error) return (error);
	
	if (isRoot)
	    DE_EXTERNALIZE_ROOT(dirp, dep);
	else
	{
	    if (DEBUG)
	    {
		if ((dirp->deAttributes ^ dep->de_Attributes) & ATTR_DIRECTORY)
		    panic("deupdate: attributes are wrong");
		if (!isDir && bcmp(dirp->deName, dep->de_Name, SHORT_NAME_LEN))
		{
		    panic("msdosfs_deupdat: file name is wrong");
		}
	    }
	    DE_EXTERNALIZE(dirp, dep);
	}
	
	error = buf_bdwrite(bp);
	bp = NULL;
    }
    
    /*
     * We need to be able to update the de_Attributes for a directory (in the
     * directory's own entry, not the "." or volume label entry) so that we can
     * change its hidden bit.  For files, we've already updated the de_Attributes
     * above.
     */
    if (error == 0 && (dep->de_flag & DE_ATTR_MOD))
    {
	dep->de_flag &= ~DE_ATTR_MOD;
	
	if (isDir && !isRoot)
	{
	    error = msdosfs_readep(pmp, dep->de_dirclust, dep->de_diroffset, &bp, &dirp, context);
	    if (error) return error;
	    dirp->deAttributes = dep->de_Attributes;
	    error = buf_bdwrite(bp);
	    bp = NULL;
	}
    }
    return error;
}

/*
 * Truncate the file described by dep to the length specified by length.
 *
 * NOTE: This function takes care of updating dep->de_FileSize and calling
 * ubc_setsize with new length.
 */
int msdosfs_detrunc(struct denode *dep, uint32_t length, int flags, vfs_context_t context)
{
    int error;
    int allerror;
    int cluster_locked = 0;
    uint32_t eofentry;
    uint32_t chaintofree;
    int isadir = dep->de_Attributes & ATTR_DIRECTORY;
    struct msdosfsmount *pmp = dep->de_pmp;
    vnode_t vp = DETOV(dep);

    /*
     * Disallow attempts to truncate the root directory since it is of
     * fixed size.  That's just the way dos filesystems are.  We use
     * the VROOT bit in the vnode because checking for the directory
     * bit and a startcluster of 0 in the denode is not adequate to
     * recognize the root directory at this point in a file or
     * directory's life.
     */
    if (vnode_isvroot(vp) && !FAT32(pmp)) {
        printf("msdosfs_detrunc(): can't truncate root directory, clust %d, offset %d\n",
               dep->de_dirclust, dep->de_diroffset);
        return (EINVAL);
    }

    if (dep->de_FileSize < length) {
        return msdosfs_deextend(dep, length, flags, context);
    }

    lck_mtx_lock(dep->de_cluster_lock);
    cluster_locked = 1;
	
    /*
     * If the desired length is 0 then remember the starting cluster of
     * the file and set the StartCluster field in the directory entry
     * to 0.  If the desired length is not zero, then get the number of
     * the last cluster in the shortened file.  Then get the number of
     * the first cluster in the part of the file that is to be freed.
     * Then set the next cluster pointer in the last cluster of the
     * file to CLUST_EOFE.
     */
    if (length == 0) {
        chaintofree = dep->de_StartCluster;
        dep->de_StartCluster = 0;
        dep->de_LastCluster = 0;
	dep->de_cluster_count = 0;
    } else {
	uint32_t length_clusters;
	
	length_clusters = de_clcount(pmp, length);
	
	/*
	 * Determine the new last cluster of the file.
	 *
	 * Note: this has a side effect of updating the cluster extent cache
	 * to be the extent containing the new end of file.  We must do this
	 * before updating the cluster extent cache; if not, msdosfs_pcbmap_internal
	 * could update the cluster extent cache (if the previously cached
	 * extent didn't contain the new last cluster), leaving it inconsistent
	 * once we actually truncate the extra clusters.
	 */
	lck_mtx_lock(dep->de_pmp->pm_fat_lock);
	error = msdosfs_pcbmap_internal(dep, length_clusters - 1, 1, NULL,
		   &eofentry, NULL);
	lck_mtx_unlock(dep->de_pmp->pm_fat_lock);

	if (error)
	{
	    allerror = error;
	    goto exit;
	}

	/*
	 * Update the cluster extent cache.  We have to be careful that we
	 * don't call msdosfs_pcbmap_internal until after the cluster chain has actually
	 * been truncated.  (Otherwise our cluster extent cache would be
	 * inconsistent with the actual cluster chain on disk.)
	 *
	 * We could also move this code below, after the msdosfs_fatentry() call that
	 * truncates the chain.
	 */
	if (length_clusters >= dep->de_cluster_logical)
	{
	    /*
	     * New length is in or after the currently cached extent.
	     * If within the cached extent, then truncate it.
	     */
	    if (length_clusters < (dep->de_cluster_logical + dep->de_cluster_count))
		    dep->de_cluster_count = length_clusters - dep->de_cluster_logical;
	}
	else
	{
	    /*
	     * The new length is before the currently cached extent,
	     * so completely invalidate the cached extent (none of it
	     * exists any more).
	     */
	    dep->de_cluster_count = 0;
	}
    }

    allerror = buf_invalidateblks(vp, ((length > 0) ? BUF_WRITE_DATA : 0), 0, 0);
	
    dep->de_FileSize = length;

    /*
     * Write out the updated directory entry.  Even if the update fails
     * we free the trailing clusters.
     */
    if (!isadir)
        dep->de_flag |= DE_UPDATE|DE_MODIFIED;

    error = msdosfs_deupdat(dep, 1, context);
    if (error && (allerror == 0))
        allerror = error;

    /*
     * If we need to break the cluster chain for the file then do it
     * now.
     */
    if (length != 0) {
        error = msdosfs_fatentry(FAT_GET_AND_SET, pmp, eofentry,
                         &chaintofree, CLUST_EOFE);
        if (error)
	{
	    dep->de_cluster_count = 0;
	    allerror = error;
	    goto exit;
	}
		
	dep->de_LastCluster = eofentry;
    }

    /*
     * Now free the clusters removed from the file because of the
     * truncation.
     */
    if (chaintofree != 0 && !MSDOSFSEOF(pmp, chaintofree))
        msdosfs_freeclusterchain(pmp, chaintofree);

    /*
     * If "length" is not a multiple of the page size, ubc_setsize will
     * cause the page containing offset "length" to be flushed.  This will
     * call VNOP_BLOCKMAP, which will need the de_cluster_lock.  Since
     * we're done manipulating the cached cluster extent, release the
     * de_cluster_lock now.
     */
    lck_mtx_unlock(dep->de_cluster_lock);
    cluster_locked = 0;
    
    ubc_setsize(vp, (off_t)length); /* XXX check errors */

exit:
    if (cluster_locked)
	lck_mtx_unlock(dep->de_cluster_lock);

    return (allerror);
}

/*
 * Extend the file described by dep to length specified by length.
 * We must be able to allocate all of the new space, else fail this request.
 */
int msdosfs_deextend(struct denode *dep, uint32_t length, int flags, vfs_context_t context)
{
    struct msdosfsmount *pmp = dep->de_pmp;
    uint32_t count;
    int error;

    /*
     * The root of a DOS filesystem cannot be extended.
     */
    if (vnode_isvroot(DETOV(dep)) && !FAT32(pmp))
        return (EINVAL);

    /*
     * Directories cannot be extended.
     */
    if (dep->de_Attributes & ATTR_DIRECTORY)
        return (EISDIR);

    if (length <= dep->de_FileSize)
        panic("msdosfs_deextend: file too large");

    /*
     * Compute the number of clusters to allocate.
     */
    count = de_clcount(pmp, length) - de_clcount(pmp, dep->de_FileSize);
    if (count > 0) {
        if (count > pmp->pm_freeclustercount)
            return (ENOSPC);
        error = msdosfs_extendfile(dep, count, NULL);
        if (error) {
            /* truncate the added clusters away again */
            (void) msdosfs_detrunc(dep, dep->de_FileSize, 0, context);
            return (error);
        }
    }

    /* Zero fill the newly allocated bytes, except if IO_NOZEROFILL was given. */
    if (!(flags & IO_NOZEROFILL)) {
        error = cluster_write(DETOV(dep), (struct uio *) 0,
                              (off_t)dep->de_FileSize, (off_t)(length),
                              (off_t)dep->de_FileSize, (off_t)0,
                              (flags | IO_HEADZEROFILL));
        if (error)
            return (error);
    }
    
    ubc_setsize(DETOV(dep), (off_t)length); /* XXX check errors */

    dep->de_FileSize = length;

    dep->de_flag |= DE_UPDATE|DE_MODIFIED;
    return (msdosfs_deupdat(dep, 1, context));
}

/*
 * Move a denode to its correct hash queue after the file it represents has
 * been moved to a new directory.
 *
 * Assumes the msdosfs_hash_lock has NOT been acquired.
 */
void msdosfs_hash_reinsert(struct denode *dep)
{
	/*
	 * Fix up the denode cache.  The hash is based on the location of the
	 * directory entry, so we must remove it from the cache and re-enter it
	 * with the hash based on the new location of the directory entry.
	 */
	lck_mtx_lock(msdosfs_hash_lock);
	msdosfs_hash_remove(dep);
	msdosfs_hash_insert(dep);
	lck_mtx_unlock(msdosfs_hash_lock);
}

int msdosfs_vnop_reclaim(struct vnop_reclaim_args *ap)
/* {
		vnode_t a_vp;
		vfs_context_t a_context;
	} */
{
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);

    KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_RECLAIM|DBG_FUNC_START, dep, 0, 0, 0, 0);
	/*
	 * Skip everything if there was no denode.  This can happen
	 * If the vnode was temporarily created in msdosfs_check_link.
	 */
	if (dep == NULL)
		goto done;

	/*
	 * Remove the denode from its hash chain.
	 */
	lck_mtx_lock(msdosfs_hash_lock);
	msdosfs_hash_remove(dep);
	lck_mtx_unlock(msdosfs_hash_lock);
	
	/*
	 * Purge old data structures associated with the denode.
	 */
	cache_purge(vp);
	
	lck_mtx_free(dep->de_cluster_lock, msdosfs_lck_grp);
	lck_mtx_free(dep->de_lock, msdosfs_lck_grp);
	OSFree(dep, sizeof(struct denode), msdosfs_node_tag);
	vnode_clearfsnode(vp);
	vnode_removefsref(vp);
	
done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_RECLAIM|DBG_FUNC_END, 0, 0, 0, 0, 0);
	return 0;
}

int msdosfs_vnop_inactive(struct vnop_inactive_args *ap)
/* {
		vnode_t a_vp;
		vfs_context_t a_context;
	} */
{
	vnode_t vp = ap->a_vp;
	vfs_context_t context = ap->a_context;
	struct denode *dep = VTODE(vp);
	int error = 0;
	int needs_flush = 0;

    KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_INACTIVE|DBG_FUNC_START, dep, 0, 0, 0, 0);

	/*
	 * Skip everything if there was no denode.  This can happen
	 * If the vnode was temporarily created in msdosfs_check_link.
	 */
	if (dep == NULL)
		goto done;

	lck_mtx_lock(dep->de_lock);
	
	/*
	 * Ignore denodes related to stale file handles.
	 */
	if (dep->de_Name[0] == SLOT_DELETED)
		goto out;

	/*
	 * If the file has been deleted and it is on a read/write
	 * filesystem, then truncate the file, and mark the directory slot
	 * as empty.  (This may not be necessary for the dos filesystem.)
	 */
	if (dep->de_refcnt <= 0 && ( !vnode_vfsisrdonly(vp))) {
		error = msdosfs_detrunc(dep, (uint32_t) 0, 0, context);
		needs_flush = 1;
		dep->de_flag |= DE_UPDATE;
		dep->de_Name[0] = SLOT_DELETED;
		vnode_recycle(vp);
	}
	msdosfs_deupdat(dep, 1, context);

out:
	if (needs_flush)
		msdosfs_meta_flush(dep->de_pmp, FALSE);

	/*
	 * We used to do a vnode_recycle(vp) here if the file/dir had
	 * been deleted.  That's good for reducing the pressure for vnodes,
	 * but bad for delayed writes (including async mounts) because VFS
	 * calls VNOP_FSYNC(..., MNT_WAIT,...) on the vnode during vclean(),
	 * which causes us to synchronously write all of the volume's metadata.
	 * And that means that mass deletions become totally synchronous.
	 */

	lck_mtx_unlock(dep->de_lock);

done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_INACTIVE|DBG_FUNC_END, error, 0, 0, 0, 0);
	return (error);
}


/*
 * msdosfs_defileid -- Return the file ID (inode number) for a given denode.  This routine
 * is used by msdosfs_vnop_getattr and msdosfs_vnop_readdir to ensure a consistent file
 * ID space.
 *
 * In older versions, the file ID was based on the location of the directory entry
 * on disk (essentially the byte offset of the entry divided by the size of an entry).
 * The file ID of a directory was the ID of the "." entry (which is the first entry
 * in the directory).  The file ID of the root of a FAT12 or FAT16 volume (whose root
 * directory is not in an allocated cluster) is 1.
 *
 * We now use the starting cluster number of the file or directory, or 1 for the root
 * of a FAT12 or FAT16 volume.  Note that empty files have no starting cluster number,
 * and their file ID is a constant that is out of range for any FAT volume (since
 * FAT32 really only uses 28 bits for cluster number).  Directories (other than the root)
 * always contain at least one cluster for their "." and ".." entries.
 */
uint32_t msdosfs_defileid(struct denode *dep)
{
    uint32_t fileid;
    
    fileid = dep->de_StartCluster;
    
    if ((dep->de_Attributes & ATTR_DIRECTORY) && (dep->de_StartCluster == MSDOSFSROOT))
        fileid = FILENO_ROOT;		/* root of FAT12 or FAT16 */
    
    if (fileid == 0)			/* empty? */
        fileid = FILENO_EMPTY;		/* use an out-of-range cluster number */
        
    return fileid;
}
