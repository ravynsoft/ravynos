/*
 * Copyright (c) 2000-2011,2013 Apple Inc. All rights reserved.
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
/* $FreeBSD: src/sys/msdosfs/msdosfs_fat.c,v 1.24 2000/05/05 09:58:35 phk Exp $ */
/*	$NetBSD: msdosfs_fat.c,v 1.28 1997/11/17 15:36:49 ws Exp $	*/

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

/*
 * kernel include files.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/mount.h>		/* to define statfs structure */
#include <sys/vnode.h>		/* to define vattr structure */
#include <sys/ubc.h>
#include <mach/boolean.h>
#include <libkern/OSBase.h>
#include <libkern/OSAtomic.h>
#include <kern/thread_call.h>
#include <libkern/OSMalloc.h>
#include <IOKit/IOTypes.h>

/*
 * msdosfs include files.
 */
#include "bpb.h"
#include "msdosfsmount.h"
#include "direntry.h"
#include "denode.h"
#include "fat.h"
#include "msdosfs_kdebug.h"

uint32_t msdosfs_meta_delay = 50;	/* in milliseconds */

#ifndef DEBUG
#define DEBUG 0
#endif

/*
 * The fs private data used with FAT vnodes.
 */
struct msdosfs_fat_node {
    struct msdosfsmount *pmp;
    u_int32_t start_block;	/* device block number of first sector */
    u_int32_t file_size;	/* number of bytes covered by this vnode */
};

/*
 * Internal routines
 */
int msdosfs_fat_cache_flush(struct msdosfsmount *pmp, int ioflags);
int msdosfs_fatentry_internal(int function, struct msdosfsmount *pmp, uint32_t cn,
			      uint32_t *oldcontents, uint32_t newcontents);
int msdosfs_clusteralloc_internal(struct msdosfsmount *pmp, uint32_t start, uint32_t count,
				  uint32_t fillwith, uint32_t *retcluster, uint32_t *got);
int msdosfs_chainalloc(struct msdosfsmount *pmp, uint32_t start, uint32_t count,
		       uint32_t fillwith, uint32_t *retcluster, uint32_t *got);
uint32_t msdosfs_chainlength(struct msdosfsmount *pmp, uint32_t start, uint32_t count);
int msdosfs_fatchain(struct msdosfsmount *pmp, uint32_t start, uint32_t count, uint32_t fillwith);
int msdosfs_count_free_clusters(struct msdosfsmount *pmp);
void *msdosfs_fat_map(struct msdosfsmount *pmp, u_int32_t cn, u_int32_t *offp, u_int32_t *sizep, int *error_out);
void msdosfs_meta_flush_internal(struct msdosfsmount *pmp, int sync);

/*
 * vnode operations for the FAT vnode
 */
typedef int vnop_t(void *);
int msdosfs_fat_pagein(struct vnop_pagein_args *ap);
int msdosfs_fat_pageout(struct vnop_pageout_args *ap);
int msdosfs_fat_strategy(struct vnop_strategy_args *ap);
int msdosfs_fat_blockmap(struct vnop_blockmap_args *ap);
int msdosfs_fat_fsync(struct vnop_fsync_args *ap);
int msdosfs_fat_inactive(struct vnop_inactive_args *ap);
int msdosfs_fat_reclaim(struct vnop_reclaim_args *ap);

int msdosfs_fat_pagein(struct vnop_pagein_args *ap)
{
    struct msdosfs_fat_node *node = vnode_fsnode(ap->a_vp);
    
    return cluster_pagein(ap->a_vp, ap->a_pl, ap->a_pl_offset, ap->a_f_offset,
	(int)ap->a_size, node->file_size, ap->a_flags);
}

int msdosfs_fat_pageout(struct vnop_pageout_args *ap)
{
    struct msdosfs_fat_node *node = vnode_fsnode(ap->a_vp);
    
    return cluster_pageout(ap->a_vp, ap->a_pl, ap->a_pl_offset, ap->a_f_offset,
	(int)ap->a_size, node->file_size, ap->a_flags);
}

int msdosfs_fat_strategy(struct vnop_strategy_args *ap)
{
    struct msdosfs_fat_node *node = vnode_fsnode(buf_vnode(ap->a_bp));
    
    return buf_strategy(node->pmp->pm_devvp, ap);
}

int msdosfs_fat_blockmap(struct vnop_blockmap_args *ap)
{
    struct msdosfs_fat_node *node = vnode_fsnode(ap->a_vp);
    
    if (ap->a_bpn == NULL)
	return 0;

    /*
     * Return the physical sector number corresponding to ap->a_foffset,
     * where a_foffset is relative to the start of the first copy of the FAT.
     */
    *ap->a_bpn = node->start_block + (ap->a_foffset / node->pmp->pm_BlockSize);
    
    /* Return the number of contiguous bytes, up to ap->a_size */
    if (ap->a_run)
    {
	if (ap->a_foffset + ap->a_size > node->file_size)
	    *ap->a_run = (size_t)(node->file_size - ap->a_foffset);
	else
	    *ap->a_run = ap->a_size;
    }
    
    return 0;
}

int msdosfs_fat_fsync(struct vnop_fsync_args *ap)
{
    int error = 0;
    int ioflags = 0;
    vnode_t vp = ap->a_vp;
    struct msdosfs_fat_node *node = vnode_fsnode(ap->a_vp);
    struct msdosfsmount *pmp = node->pmp;
    
    if (ap->a_waitfor == MNT_WAIT)
	ioflags = IO_SYNC;

    KERNEL_DEBUG_CONSTANT(MSDOSFS_FAT_FSYNC|DBG_FUNC_START, pmp, ioflags, 0, 0, 0);

    if (pmp->pm_fat_flags & FAT_CACHE_DIRTY)
    {
	lck_mtx_lock(pmp->pm_fat_lock);
	error = msdosfs_fat_cache_flush(pmp, ioflags);
 	lck_mtx_unlock(pmp->pm_fat_lock);
    }
    if (!error)
	cluster_push(vp, ioflags);

    KERNEL_DEBUG_CONSTANT(MSDOSFS_FAT_FSYNC|DBG_FUNC_END, error, 0, 0, 0, 0);
    return error;
}

int msdosfs_fat_inactive(struct vnop_inactive_args *ap)
{
    vnode_recycle(ap->a_vp);
    return 0;
}

int msdosfs_fat_reclaim(struct vnop_reclaim_args *ap)
{
    vnode_t vp = ap->a_vp;
    struct msdosfs_fat_node *node = vnode_fsnode(vp);

    vnode_clearfsnode(vp);
    OSFree(node, sizeof(*node), msdosfs_malloc_tag);
    return 0;
}

static int (**msdosfs_fat_vnodeop_p)(void *);
static struct vnodeopv_entry_desc msdosfs_fat_vnodeop_entries[] = {
    { &vnop_default_desc,   (vnop_t *) vn_default_error },
    { &vnop_pagein_desc,    (vnop_t *) msdosfs_fat_pagein },
    { &vnop_pageout_desc,   (vnop_t *) msdosfs_fat_pageout },
    { &vnop_strategy_desc,  (vnop_t *) msdosfs_fat_strategy },
    { &vnop_blockmap_desc,  (vnop_t *) msdosfs_fat_blockmap },
    { &vnop_blktooff_desc,  (vnop_t *) msdosfs_vnop_blktooff },
    { &vnop_offtoblk_desc,  (vnop_t *) msdosfs_vnop_offtoblk },
    { &vnop_fsync_desc,	    (vnop_t *) msdosfs_fat_fsync },
    { &vnop_inactive_desc,  (vnop_t *) msdosfs_fat_inactive },
    { &vnop_reclaim_desc,   (vnop_t *) msdosfs_fat_reclaim },
    { &vnop_bwrite_desc,    (vnop_t *) vn_bwrite },
    { NULL, NULL }
};

__private_extern__
struct vnodeopv_desc msdosfs_fat_vnodeop_opv_desc =
	{ &msdosfs_fat_vnodeop_p, msdosfs_fat_vnodeop_entries };

int msdosfs_fat_init_vol(struct msdosfsmount *pmp)
{	
    errno_t error;
    struct vnode_fsparam vfsp;
    struct msdosfs_fat_node *node;

    lck_mtx_lock(pmp->pm_fat_lock);
    
    /*
     * Create a vnode to use to access the active FAT.
     */
    node = OSMalloc(sizeof(*node), msdosfs_malloc_tag);
    if (node == NULL)
    {
    	error = ENOMEM;
	goto exit;
    }
    node->pmp = pmp;
    node->start_block = pmp->pm_ResSectors * pmp->pm_BlocksPerSec;
    node->file_size = pmp->pm_fat_bytes;
    vfsp.vnfs_mp = pmp->pm_mountp;
    vfsp.vnfs_vtype = VREG;
    vfsp.vnfs_str = "msdosfs";
    vfsp.vnfs_dvp = NULL;
    vfsp.vnfs_fsnode = node;
    vfsp.vnfs_vops = msdosfs_fat_vnodeop_p;
    vfsp.vnfs_markroot = 0;
    vfsp.vnfs_marksystem = 1;
    vfsp.vnfs_rdev = 0;
    vfsp.vnfs_filesize = pmp->pm_fat_bytes;
    vfsp.vnfs_cnp = NULL;
    vfsp.vnfs_flags = VNFS_CANTCACHE;
    error = vnode_create(VNCREATE_FLAVOR, VCREATESIZE, &vfsp, &pmp->pm_fat_active_vp);
    if (error)
    {
        OSFree(node, sizeof(*node), msdosfs_malloc_tag);
	goto exit;
    }
    vnode_ref(pmp->pm_fat_active_vp);
    vnode_put(pmp->pm_fat_active_vp);
    
    if (pmp->pm_flags & MSDOSFS_FATMIRROR)
    {
	/*
	 * Create a vnode to use to access the alternate FAT.
	 */
        node = OSMalloc(sizeof(*node), msdosfs_malloc_tag);
        if (node == NULL)
	{
	    error = ENOMEM;
	    goto exit;
	}
        node->pmp = pmp;
        node->start_block = pmp->pm_ResSectors * pmp->pm_BlocksPerSec + pmp->pm_fat_bytes / pmp->pm_BlockSize;
        node->file_size = (pmp->pm_FATs - 1) * pmp->pm_fat_bytes;
        vfsp.vnfs_mp = pmp->pm_mountp;
        vfsp.vnfs_vtype = VREG;
        vfsp.vnfs_str = "msdosfs";
        vfsp.vnfs_dvp = NULL;
        vfsp.vnfs_fsnode = node;
        vfsp.vnfs_vops = msdosfs_fat_vnodeop_p;
        vfsp.vnfs_markroot = 0;
        vfsp.vnfs_marksystem = 1;
        vfsp.vnfs_rdev = 0;
        vfsp.vnfs_filesize = node->file_size;
        vfsp.vnfs_cnp = NULL;
        vfsp.vnfs_flags = VNFS_CANTCACHE;
        error = vnode_create(VNCREATE_FLAVOR, VCREATESIZE, &vfsp, &pmp->pm_fat_mirror_vp);
        if (error)
        {
	    OSFree(node, sizeof(*node), msdosfs_malloc_tag);
	    goto exit;
        }
        vnode_ref(pmp->pm_fat_mirror_vp);
        vnode_put(pmp->pm_fat_mirror_vp);
    }
    
    /*
     * Initialize the cache for most recently used FAT block.
     */
    pmp->pm_fat_cache = OSMalloc(pmp->pm_fatblocksize, msdosfs_malloc_tag);
    if (pmp->pm_fat_cache == NULL)
    {
    	error = ENOMEM;
	goto exit;
    }
    pmp->pm_fat_cache_offset = -1;	/* invalid; no cached content */
    pmp->pm_fat_flags = 0;		/* Nothing is dirty yet */
    
    /*
     * Allocate space for a list of free extents, used when searching for
     * free space (especially when free space is highly fragmented).
     */
    pmp->pm_free_extents = OSMalloc(PAGE_SIZE, msdosfs_malloc_tag);
    if (pmp->pm_free_extents == NULL)
    {
	error = ENOMEM;
	goto exit;
    }
    pmp->pm_free_extent_count = 0;

    /*
     * We need to read through the FAT to determine the number of free clusters.
     */
    error = msdosfs_count_free_clusters(pmp);

exit:
    /*
     * NOTE: All memory allocated above gets freed in msdosfs_fat_uninit_vol.
     */
    lck_mtx_unlock(pmp->pm_fat_lock);
    return error;
}

void msdosfs_fat_uninit_vol(struct msdosfsmount *pmp)
{
    /*
     * Try one last time to flush the FAT before we get rid of the FAT vnodes.
     *
     * During a force unmount, the FAT may be dirty, and the device already
     * gone.  In that case, the flush will fail, but we must mark the FAT as
     * no longer dirty.  If we don't, releasing the vnodes below will again
     * try to flush the FAT, and will panic due to a NULL vnode pointer.
     */
    if (pmp->pm_fat_flags & FAT_CACHE_DIRTY)
    {
	int error;
	lck_mtx_lock(pmp->pm_fat_lock);
	error = msdosfs_fat_cache_flush(pmp, IO_SYNC);
	if (error)
	    printf("msdosfs_fat_uninit_vol: error %d from msdosfs_fat_cache_flush\n", error);
	pmp->pm_fat_flags = 0;	/* Ignore anything dirty or in need of update */
 	lck_mtx_unlock(pmp->pm_fat_lock);
    }

    if (pmp->pm_fat_active_vp)
    {
	/* Would it be better to push async, then wait for writes to complete? */
	cluster_push(pmp->pm_fat_active_vp, IO_SYNC);
	vnode_recycle(pmp->pm_fat_active_vp);
	vnode_rele(pmp->pm_fat_active_vp);
	pmp->pm_fat_active_vp = NULL;
    }
    
    if (pmp->pm_fat_mirror_vp)
    {
	cluster_push(pmp->pm_fat_mirror_vp, IO_SYNC);
	vnode_recycle(pmp->pm_fat_mirror_vp);
	vnode_rele(pmp->pm_fat_mirror_vp);
	pmp->pm_fat_mirror_vp = NULL;
    }
    
    if (pmp->pm_fat_cache)
    {
	OSFree(pmp->pm_fat_cache, pmp->pm_fatblocksize, msdosfs_malloc_tag);
	pmp->pm_fat_cache = NULL;
    }
    
    if (pmp->pm_free_extents)
    {
	OSFree(pmp->pm_free_extents, PAGE_SIZE, msdosfs_malloc_tag);
	pmp->pm_free_extents = NULL;
	pmp->pm_free_extent_count = 0;
    }
}



/*
 * Update the free cluster count and next free cluster number in the FSInfo
 * sector of a FAT32 volume.  This is called during VFS_SYNC.
 */
int msdosfs_update_fsinfo(struct msdosfsmount *pmp, int waitfor, vfs_context_t context)
{
    int error = 0;
    buf_t bp;
    struct fsinfo *fp;

    KERNEL_DEBUG_CONSTANT(MSDOSFS_UPDATE_FSINFO|DBG_FUNC_START, pmp, waitfor, 0, 0, 0);
    /*
     * If we don't have an FSInfo sector, or if there has been no change
     * to the FAT since the last FSInfo update, then there's nothing to do.
     */
    if (pmp->pm_fsinfo_size == 0 || !(pmp->pm_fat_flags & FSINFO_DIRTY))
    {
	goto done;
    }

    error = buf_meta_bread(pmp->pm_devvp, pmp->pm_fsinfo_sector, pmp->pm_fsinfo_size,
		vfs_context_ucred(context), &bp);
    if (error)
    {
	/*
	 * Turn off FSInfo update for the future.
	 */
	printf("msdosfs_update_fsinfo: error %d reading FSInfo\n", error);
	pmp->pm_fsinfo_size = 0;
	buf_brelse(bp);
    }
    else
    {
	fp = (struct fsinfo *) (buf_dataptr(bp) + pmp->pm_fsinfo_offset);
	
	putuint32(fp->fsinfree, pmp->pm_freeclustercount);
	putuint32(fp->fsinxtfree, pmp->pm_nxtfree);
	if (waitfor || pmp->pm_flags & MSDOSFSMNT_WAITONFAT)
	    error = buf_bwrite(bp);
	else
	    error = buf_bawrite(bp);
    }
    
    pmp->pm_fat_flags &= ~FSINFO_DIRTY;
done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_UPDATE_FSINFO|DBG_FUNC_END, error, 0, 0, 0, 0);
    return error;
}


/*
 * If the FAT cache is dirty, then write it back via Cluster I/O.
 *
 * NOTE: Cluster I/O will typically cache this write in UBC, so the write
 * will not necessarily go to disk right away.  You must explicitly fsync
 * the vnode(s) to force changes to the media.
 */
int msdosfs_fat_cache_flush(struct msdosfsmount *pmp, int ioflags)
{
    int error = 0;
    u_int32_t fat_file_size;
    u_int32_t block_size;
    uio_t uio;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FAT_CACHE_FLUSH|DBG_FUNC_START, pmp, ioflags, 0, 0, 0);

    if (DEBUG) lck_mtx_assert(pmp->pm_fat_lock, LCK_MTX_ASSERT_OWNED);

    if ((pmp->pm_fat_flags & FAT_CACHE_DIRTY) == 0)
	goto done;

    fat_file_size = pmp->pm_fat_bytes;
    block_size = pmp->pm_fatblocksize;
    if (pmp->pm_fat_cache_offset + block_size > pmp->pm_fat_bytes)
    	block_size = pmp->pm_fat_bytes - pmp->pm_fat_cache_offset;
    
    uio = uio_create(1, pmp->pm_fat_cache_offset, UIO_SYSSPACE, UIO_WRITE);
    if (uio == NULL)
    {
	error = ENOMEM;
	goto done;
    }
    uio_addiov(uio, CAST_USER_ADDR_T(pmp->pm_fat_cache), block_size);
    error = cluster_write(pmp->pm_fat_active_vp, uio, fat_file_size, fat_file_size, 0, 0, ioflags);
    if (!error)
	pmp->pm_fat_flags &= ~FAT_CACHE_DIRTY;

    /*
     * If we're mirroring the FAT, then write to all of the other copies now.
     * By definition, mirroring means that pmp->pm_curfat == 0, so we just
     * need to loop over 1 .. maximum here.
     */
    if (pmp->pm_flags & MSDOSFS_FATMIRROR)
    {
	int i;
	
	fat_file_size = (pmp->pm_FATs - 1) * pmp->pm_fat_bytes;
	for (i=0; i<pmp->pm_FATs-1; ++i)
	{
	    uio_reset(uio, pmp->pm_fat_cache_offset + i * pmp->pm_fat_bytes, UIO_SYSSPACE, UIO_WRITE);
	    uio_addiov(uio, CAST_USER_ADDR_T(pmp->pm_fat_cache), block_size);
	    error = cluster_write(pmp->pm_fat_mirror_vp, uio, fat_file_size, fat_file_size, 0, 0, ioflags);
	}
    }
    
    /* We're finally done with the uio */
    uio_free(uio);
    
done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FAT_CACHE_FLUSH|DBG_FUNC_END, error, 0, 0, 0, 0);
    return error;
}


/*
 * Given a cluster number, bring the corresponding block of the FAT into
 * the pm_fat_cache (after writing a dirty cache block, if needed).  Returns
 * a pointer to the given cluster's entry.
 *
 * Inputs:
 *	pmp
 *	cn
 *
 * Outputs (optional):
 *	offp	Offset of entryp from start of FAT block, in bytes
 *	sizep	Number of valid bytes in this block.  For the last FAT block,
 *		this may be less than the block size (it will be the offset
 *		following the entry for the last cluster).
 *
 * Return value: Pointer to cluster cn's entry in the FAT block
 */
void *msdosfs_fat_map(struct msdosfsmount *pmp, u_int32_t cn, u_int32_t *offp, u_int32_t *sizep, int *error_out)
{
    int error = 0;
    u_int32_t offset;
    u_int32_t block_offset;
    u_int32_t block_size;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FAT_MAP|DBG_FUNC_START, pmp, cn, 0, 0, 0);

    if (DEBUG) lck_mtx_assert(pmp->pm_fat_lock, LCK_MTX_ASSERT_OWNED);
    
    offset = cn * pmp->pm_fatmult / pmp->pm_fatdiv;
    block_offset = offset / pmp->pm_fatblocksize * pmp->pm_fatblocksize;	/* Round to start of page */
    block_size = pmp->pm_fatblocksize;
    if (block_offset + block_size > pmp->pm_fat_bytes)
    	block_size = pmp->pm_fat_bytes - block_offset;

    if (pmp->pm_fat_cache_offset != block_offset)
    {
	uio_t uio;
	
    	if (pmp->pm_fat_flags & FAT_CACHE_DIRTY)
	    error = msdosfs_fat_cache_flush(pmp, 0);
	
	if (!error)
	{
	    pmp->pm_fat_cache_offset = block_offset;
	    uio = uio_create(1, block_offset, UIO_SYSSPACE, UIO_READ);
	    if (uio == NULL)
	    {
		error = ENOMEM;
	    }
	    else
	    {
		uio_addiov(uio, CAST_USER_ADDR_T(pmp->pm_fat_cache), block_size);
		error = cluster_read(pmp->pm_fat_active_vp, uio, pmp->pm_fat_bytes, 0);
		uio_free(uio);
	    }
	}
    }
    
    offset -= block_offset;
    
    if (offp)
	*offp = offset;

    if (sizep)
    {
	u_int32_t last_offset;
	
	/*
	 * If this is the last block of the FAT, then constrain the size to the
	 * end of the entry for the last cluster.
	 */
	last_offset = (pmp->pm_maxcluster + 1) * pmp->pm_fatmult / pmp->pm_fatdiv;
	if (last_offset - block_offset < block_size)
	    block_size = last_offset - block_offset;
    	*sizep = block_size;
    }

    if (error_out)
    	*error_out = error;

    KERNEL_DEBUG_CONSTANT(MSDOSFS_FAT_MAP|DBG_FUNC_END, error, 0, 0, 0, 0);

    if (error)
    {
	/* Invalidate the state of the FAT cache */
	pmp->pm_fat_cache_offset = -1;
	pmp->pm_fat_flags &= ~FAT_CACHE_DIRTY;
	return NULL;
    }

    return (char *) pmp->pm_fat_cache + offset;
}


/*
 * Flush the primary/active copy of the FAT, and all directory blocks.
 * This skips the boot sector, FSInfo sector, and non-active copies
 * of the FAT.
 * 
 * If sync is non-zero, the data is flushed syncrhonously.
 */
void msdosfs_meta_flush_internal(struct msdosfsmount *pmp, int sync)
{
    int error;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_META_FLUSH_INTERNAL|DBG_FUNC_START, pmp, sync, 0, 0, 0);

    /*
     * Push out any changes to the FAT.  First, any dirty data in the FAT
     * cache gets written to the FAT vnodes, then the vnodes get pushed to
     * disk.
     *
     * We take the FAT lock here to delay other threads that may be trying
     * to use the FAT, and thus delay them from activating the flush timer.
     * We also pass IO_SYNC to cluster_push to make sure the push has completed
     * before we return.
     *
     * Otherwise, we get into a vicious cycle where one change is being flushed,
     * another change starts the flush timer, and a third change is blocked
     * waiting for the first change's writes to complete.  Meanwhile, the
     * flush timer fires, starting a new flush.  At this point, every change
     * results in its own flush, effectively becoming synchronous.
     */
    lck_mtx_lock(pmp->pm_fat_lock);
    if (pmp->pm_fat_flags & FAT_CACHE_DIRTY)
    {
    	error = msdosfs_fat_cache_flush(pmp, 0);
    	if (error)
    	{
	    printf("msdosfs_meta_flush_internal: error %d flushing FAT cache!\n", error);
	    pmp->pm_fat_flags &= ~FAT_CACHE_DIRTY;
	}
    }
    cluster_push(pmp->pm_fat_active_vp, IO_SYNC);
    if (pmp->pm_fat_mirror_vp)
	cluster_push(pmp->pm_fat_mirror_vp, IO_SYNC);
    lck_mtx_unlock(pmp->pm_fat_lock);
    
    /*
     * Push out any changes to directory blocks.
     */
    buf_flushdirtyblks(pmp->pm_devvp, sync, 0, "msdosfs_meta_flush");
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_META_FLUSH_INTERNAL|DBG_FUNC_END, 0, 0, 0, 0, 0);
}

void msdosfs_meta_flush(struct msdosfsmount *pmp, int sync)
{
    KERNEL_DEBUG_CONSTANT(MSDOSFS_META_FLUSH|DBG_FUNC_START, pmp, sync, 0, 0, 0);

    if (sync) goto flush_now;

    /*
     * If the volume was mounted async, and we're not being told to make
     * this flush synchronously, then ignore it.
     */
    if (vfs_flags(pmp->pm_mountp) & MNT_ASYNC)
    {
	KERNEL_DEBUG_CONSTANT(MSDOSFS_META_FLUSH|DBG_FUNC_END, 0, MNT_ASYNC, 0, 0, 0);
	return;
    }

    /*
     * If we have a timer, but it has not been scheduled yet, then schedule
     * it now.  It is possible that some or all of the metadata changed by the
     * caller has already been written out.  We're merely guaranteeing that
     * there will be a future metadata flush.
     *
     * If multiple threads are racing into this routine, they could all end
     * up (re)scheduling the timer.  The first thread to schedule the timer
     * ends up incrementing both pm_sync_scheduled and pm_sync_incomplete.
     * Any other thread will both increment and decrement pm_sync_scheduled,
     * leaving it unchanged in the end (but greater than 1 for a brief time).
     *
     * NOTE: We can't rely on just a single counter (pm_sync_incomplete) because
     * it doesn't get decremented until the callback has completed, so it
     * could be non-zero if the flush has already begun and the iterator has
     * already passed the newly dirtied blocks.  (And unmount definitely needs
     * to know when the last callback is *complete*, not just started.)
     */
    if (pmp->pm_sync_timer)
    {
    	if (pmp->pm_sync_scheduled == 0)
    	{
	    AbsoluteTime deadline;

	    clock_interval_to_deadline(msdosfs_meta_delay, kMillisecondScale, &deadline);

	    KERNEL_DEBUG_CONSTANT(MSDOSFS_META_FLUSH|DBG_FUNC_NONE, 0, 0, 0, 0, 0);

	    /*
	     * Increment pm_sync_scheduled on the assumption that we're the
	     * first thread to schedule the timer.  If some other thread beat
	     * us, then we'll decrement it.  If we *were* the first to
	     * schedule the timer, then we need to keep track that the
	     * callback is waiting to complete.
	     */
	    OSIncrementAtomic(&pmp->pm_sync_scheduled);
	    if (thread_call_enter_delayed(pmp->pm_sync_timer, deadline))
		OSDecrementAtomic(&pmp->pm_sync_scheduled);
	    else
		OSIncrementAtomic(&pmp->pm_sync_incomplete);
    	}

	return;
    }

flush_now:
    msdosfs_meta_flush_internal(pmp, sync);
    KERNEL_DEBUG_CONSTANT(MSDOSFS_META_FLUSH|DBG_FUNC_END, 0, 0, 0, 0, 0);
}


void msdosfs_meta_sync_callback(void *arg0, void *unused)
{
#pragma unused(unused)
    struct msdosfsmount *pmp = arg0;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_META_SYNC_CALLBACK|DBG_FUNC_START, pmp, 0, 0, 0, 0);

    OSDecrementAtomic(&pmp->pm_sync_scheduled);
    msdosfs_meta_flush_internal(pmp, 0);
    OSDecrementAtomic(&pmp->pm_sync_incomplete);
    wakeup(&pmp->pm_sync_incomplete);
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_META_SYNC_CALLBACK|DBG_FUNC_END, 0, 0, 0, 0, 0);
}



/*
 * Map the logical cluster number of a file into a physical disk sector
 * that is filesystem relative.
 *
 * dep	  - address of denode representing the file of interest
 * findcn - file relative cluster whose filesystem relative cluster number
 *	    and/or block number are/is to be found
 * bnp	  - address of where to place the file system relative block number.
 *	    If this pointer is null then don't return this quantity.
 * cnp	  - address of where to place the file system relative cluster number.
 *	    If this pointer is null then don't return this quantity.
 * sp	  - address of where to place the number of contiguous bytes.
 *		If this pointer is null then don't return this quantity.
 *
 * NOTE: Either bnp or cnp must be non-null.
 *
 * This function has one side effect.  If the requested file relative cluster
 * is beyond the end of file, then the actual number of clusters in the file
 * is returned in *cnp, and the file's last cluster number is returned in *sp.
 * This is useful for determining how long a directory is, or for initializing
 * the de_LastCluster field.
 */
int msdosfs_pcbmap_internal(
    struct denode *dep,
    uint32_t findcn,	    /* file relative cluster to get */
    uint32_t numclusters,	    /* maximum number of contiguous clusters to map */
    daddr64_t *bnp,	    /* returned filesys relative blk number	 */
    uint32_t *cnp,	    /* returned cluster number */
    uint32_t *sp)		    /* number of contiguous bytes */
{
    int error=0;
    uint32_t i;			/* A temporary */
    uint32_t cn;			/* The current cluster number being examined */
    uint32_t prevcn=0;		/* The cluster previous to cn */
    uint32_t cluster_logical;	/* First file-relative cluster in extent */
    uint32_t cluster_physical;	/* First volume-relative cluster in extent */
    uint32_t cluster_count;	/* Number of clusters in extent */
    struct msdosfsmount *pmp = dep->de_pmp;
    void *entry;
    daddr64_t result_block = 0;
    uint32_t result_cluster = 0;
    uint32_t result_contig = 0;
    
    cn = dep->de_StartCluster;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_PCBMAP_INTERNAL|DBG_FUNC_START, pmp, cn, findcn, numclusters, 0);

    if (numclusters == 0)
	panic("msdosfs_pcbmap: numclusters == 0");
    
    /*
     * If they don't give us someplace to return a value then don't
     * bother doing anything.
     */
    if (bnp == NULL && cnp == NULL && sp == NULL)
	goto done;

    /*
     * The "file" that makes up the root directory is contiguous,
     * permanently allocated, of fixed size, and is not made up of
     * clusters.  If the cluster number is beyond the end of the root
     * directory, then return the number of clusters in the file.
     */
    if (cn == MSDOSFSROOT) {
	if (dep->de_Attributes & ATTR_DIRECTORY) {
	    if (de_cn2off(pmp, findcn) >= dep->de_FileSize) {
		result_cluster = de_bn2cn(pmp, pmp->pm_rootdirsize);
		error = E2BIG;
		goto done;
	    }
	    result_block = pmp->pm_rootdirblk + de_cn2bn(pmp, findcn);
	    result_cluster = MSDOSFSROOT;
	    result_contig = min(pmp->pm_bpcluster, dep->de_FileSize - de_cn2off(pmp, findcn));
	    goto done;
	} else {		/* just an empty file */
	    result_cluster = 0;
	    error = E2BIG;
	    goto done;
	}
    }
    
    /*
     * If the requested position is within the currently cached extent,
     * we can return all of the information without walking the cluster
     * chain.
     */
    if (findcn >= dep->de_cluster_logical &&
	findcn < (dep->de_cluster_logical + dep->de_cluster_count))
    {
	i = findcn - dep->de_cluster_logical;	/* # clusters into cached extent */
	cn = dep->de_cluster_physical + i;	/* Starting cluster number */
	
	result_block = cntobn(pmp, cn);
	result_cluster = cn;
	result_contig = min(dep->de_cluster_count - i, numclusters) * pmp->pm_bpcluster;
	goto done;
    }
    
    /* Default to scanning from the beginning of the chain. */
    cluster_logical = 0;
    cluster_physical = 0;
    cluster_count = 0;
    /* From above: cn = dep->de_StartCluster */

    /*
     * If the requested position is after the cached extent, then start scanning
     * from the last cluster of the cached extent.  We set up the state
     * (cluster_xxx and cn) as if the loop below had just finished scanning
     * the cached extent, with cn being the first cluster of the next extent.
     */
    if (dep->de_cluster_count && findcn > dep->de_cluster_logical)
    {
    	cluster_logical = dep->de_cluster_logical;
    	cluster_physical = dep->de_cluster_physical;
    	cluster_count = dep->de_cluster_count;
    	prevcn = dep->de_cluster_physical+dep->de_cluster_count-1;
    	error = msdosfs_fatentry_internal(FAT_GET, pmp, prevcn, &cn, 0);
	if (error)
	    goto done;
    }

    /*
     * Iterate through the extents (runs of contiguous clusters) in the file
     * until we find the extent containing findcn, or we hit end of file.
     */
    while ((cluster_logical + cluster_count) <= findcn)
    {
	/*
	 * Stop with all reserved clusters, not just with EOF.
	 */
	if ((cn | ~pmp->pm_fatmask) >= CLUST_RSRVD)
	    goto hiteof;
	
	/*
	 * Detect infinite cycles in the cluster chain.  (Generally only useful
	 * for directories, where findcn is (-1).)
	 */
	if (cluster_logical > pmp->pm_maxcluster)
	{
	    printf("msdosfs: msdosfs_pcbmap: Corrupt cluster chain detected\n");
	    pmp->pm_flags |= MSDOSFS_CORRUPT;
	    error = EIO;
	    goto done;
	}
	
	/*
	 * Stop and return an error if we hit an out-of-range cluster number.
	 */
	if (cn < CLUST_FIRST || cn > pmp->pm_maxcluster)
	{
	    if (DEBUG)
		panic("msdosfs_pcbmap_internal: invalid cluster: cn=%u, name='%11.11s'", cn, dep->de_Name);
	    error = EIO;
	    goto done;
	}
	
	/*
	 * Keep track of the start of the extent.
	 *
	 * The count is initialized to zero because it will be incremented to
	 * account for cluster "cn" at the end of the do ... while loop.
	 */
	cluster_logical += cluster_count;   /* Skip past previous extent */
	cluster_physical = cn;
	cluster_count = 0;
	
	/*
	 * Loop over contiguous clusters starting with #cn.
	 */
	do {
	    /*
	     * Make sure we have the block containing the cn'th entry in the FAT.
	     */
	    entry = msdosfs_fat_map(pmp, cn, NULL, NULL, &error);
	    if (!entry)
		goto done;
	    
	    /*
	     * Fetch the next cluster in the chain.
	     */
	    prevcn = cn;
	    if (FAT32(pmp))
		cn = getuint32(entry);
	    else
		cn = getuint16(entry);
	    if (FAT12(pmp) && (prevcn & 1))
		cn >>= 4;
	    cn &= pmp->pm_fatmask;
	    
	    /*
	     * Force the special cluster numbers
	     * to be the same for all cluster sizes
	     * to let the rest of msdosfs handle
	     * all cases the same.
	     */
	    if ((cn | ~pmp->pm_fatmask) >= CLUST_RSRVD)
		cn |= ~pmp->pm_fatmask;
	    
	    cluster_count++;
	} while (cn == prevcn + 1);
	
	/*
	 * Falling out of the do..while loop means that cn is the start of a
	 * new extent (or a reserved/EOF value).
	 */
    }
    
    /*
     * We have found the extent containing findcn.  Remember it, and return
     * the mapping information.
     */
    dep->de_cluster_logical = cluster_logical;
    dep->de_cluster_physical = cluster_physical;
    dep->de_cluster_count = cluster_count;
    
    i = findcn - cluster_logical;	/* # of clusters into found extent */
    cn = cluster_physical + i;	/* findcn'th physical (volume-relative) cluster */
    
    result_block = cntobn(pmp, cn);
    result_cluster = cn;
    result_contig = min(cluster_count-i, numclusters) * pmp->pm_bpcluster;

done:
    if (bnp)
	*bnp = result_block;
    if (cnp)
	*cnp = result_cluster;
    if (sp)
	*sp = result_contig;
    KERNEL_DEBUG_CONSTANT(MSDOSFS_PCBMAP_INTERNAL|DBG_FUNC_END, error, result_block, result_cluster, result_contig, 0);
    return error;
    
hiteof:
    result_cluster = cluster_logical + cluster_count;
    result_contig = prevcn;    
    error = E2BIG;
    goto done;
}


int msdosfs_pcbmap(
    struct denode *dep,
    uint32_t findcn,	    /* file relative cluster to get */
    uint32_t numclusters,	    /* maximum number of contiguous clusters to map */
    daddr64_t *bnp,	    /* returned filesys relative blk number	 */
    uint32_t *cnp,	    /* returned cluster number */
    uint32_t *sp)		    /* number of contiguous bytes */
{
    int error;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_PCBMAP|DBG_FUNC_START, dep->de_pmp, dep->de_StartCluster, findcn, numclusters, 0);

    lck_mtx_lock(dep->de_cluster_lock);
    lck_mtx_lock(dep->de_pmp->pm_fat_lock);
    error = msdosfs_pcbmap_internal(dep, findcn, numclusters, bnp, cnp, sp);
    lck_mtx_unlock(dep->de_pmp->pm_fat_lock);
    lck_mtx_unlock(dep->de_cluster_lock);
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_PCBMAP|DBG_FUNC_END, error, 0, 0, 0, 0);

    return error;
}


/*
 * Get or Set or 'Get and Set' the cluster'th entry in the fat.
 *
 * function	- whether to get or set a fat entry
 * pmp		- address of the msdosfsmount structure for the filesystem
 *		  whose fat is to be manipulated.
 * cn		- which cluster is of interest
 * oldcontents	- address of a word that is to receive the contents of the
 *		  cluster'th entry if this is a get function
 * newcontents	- the new value to be written into the cluster'th element of
 *		  the fat if this is a set function.
 *
 * This function can also be used to free a cluster by setting the fat entry
 * for a cluster to 0.
 *
 * All copies of the fat are updated if this is a set function. NOTE: If
 * msdosfs_fatentry() marks a cluster as free it does not update the inusemap in
 * the msdosfsmount structure. This is left to the caller.
 *
 * Updating entries in 12 bit fats is a pain in the butt.
 *
 * The following picture shows where nibbles go when moving from a 12 bit
 * cluster number into the appropriate bytes in the FAT.
 *
 *	byte m        byte m+1      byte m+2
 *	+----+----+   +----+----+   +----+----+
 *	|  0    1 |   |  2    3 |   |  4    5 |   FAT bytes
 *	+----+----+   +----+----+   +----+----+
 *
 *	+----+----+----+   +----+----+----+
 *	|  3    0    1 |   |  4    5    2 |
 *	+----+----+----+   +----+----+----+
 *	cluster n  	   cluster n+1
 *
 * Where n is even. m = n + (n >> 2)
 *
 */
int msdosfs_fatentry_internal(int function, struct msdosfsmount *pmp, uint32_t cn, uint32_t *oldcontents, uint32_t newcontents)
{
    int error = 0;
    uint32_t readcn;
    void *entry;
    uint32_t result_old = 0;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FATENTRY_INTERNAL|DBG_FUNC_START, pmp, cn, function, newcontents, 0);

    /*
     * Be sure the requested cluster is in the filesystem.
     */
    if (cn < CLUST_FIRST || cn > pmp->pm_maxcluster)
    {
	if (DEBUG)
	{
	    printf("msdosfs: msdosfs_fatentry_internal: cn=%u, function=%d, new=%u\n", cn, function, newcontents);
	}
	error = EIO;
	goto done;
    }
    
    entry = msdosfs_fat_map(pmp, cn, NULL, NULL, &error);
    if (!entry)
	goto done;
    
    if (function & FAT_GET) {
	if (FAT32(pmp))
	    readcn = getuint32(entry);
	else
	    readcn = getuint16(entry);
	if (FAT12(pmp) & (cn & 1))
	    readcn >>= 4;
	readcn &= pmp->pm_fatmask;
	/* map reserved fat entries to same values for all fats */
	if ((readcn | ~pmp->pm_fatmask) >= CLUST_RSRVD)
	    readcn |= ~pmp->pm_fatmask;
	*oldcontents = readcn;
	result_old = readcn;
    }
    if (function & FAT_SET) {
	switch (pmp->pm_fatmask) {
	    case FAT12_MASK:
		readcn = getuint16(entry);
		if (cn & 1) {
		    readcn &= 0x000f;
		    readcn |= newcontents << 4;
		} else {
		    readcn &= 0xf000;
		    readcn |= newcontents & 0xfff;
		}
		putuint16(entry, readcn);
		break;
	    case FAT16_MASK:
		putuint16(entry, newcontents);
		break;
	    case FAT32_MASK:
		/*
		 * According to spec we have to retain the
		 * high order bits of the fat entry.
		 */
		readcn = getuint32(entry);
		readcn &= ~FAT32_MASK;
		readcn |= newcontents & FAT32_MASK;
		putuint32(entry, readcn);
		break;
	}
	pmp->pm_fat_flags |= FAT_CACHE_DIRTY | FSINFO_DIRTY;
    }

done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FATENTRY_INTERNAL|DBG_FUNC_END, error, result_old, 0, 0, 0);
    return 0;
}

int msdosfs_fatentry(int function, struct msdosfsmount *pmp, uint32_t cn, uint32_t *oldcontents, uint32_t newcontents)
{
    int error;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FATENTRY|DBG_FUNC_START, pmp, cn, function, newcontents, 0);

    lck_mtx_lock(pmp->pm_fat_lock);
    error = msdosfs_fatentry_internal(function, pmp, cn, oldcontents, newcontents);
    lck_mtx_unlock(pmp->pm_fat_lock);
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FATENTRY|DBG_FUNC_END, error, 0, 0, 0, 0);

    return error;
}


/*
 * Update a contiguous cluster chain
 *
 * pmp	    - mount point
 * start    - first cluster of chain
 * count    - number of clusters in chain
 * fillwith - what to write into fat entry of last cluster
 */
int msdosfs_fatchain(struct msdosfsmount *pmp, uint32_t start, uint32_t count, uint32_t fillwith)
{
    int error = 0;
    u_int32_t bo, bsize;
    uint32_t readcn, newc;
    char *entry;
    char *block_end;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FATCHAIN|DBG_FUNC_START, pmp, start, count, fillwith, 0);

    /*
     * Be sure the clusters are in the filesystem.
     */
    if (start < CLUST_FIRST || start + count - 1 > pmp->pm_maxcluster)
    {
	printf("msdosfs: msdosfs_fatchain: start=%u, count=%u, fill=%u\n", start, count, fillwith);
	error = EIO;
	goto done;
    }
    
    /* Loop over all clusters in the chain */
    while (count > 0) {
	entry = msdosfs_fat_map(pmp, start, &bo, &bsize, &error);
	if (!entry)
	    break;
	
	block_end = entry - bo + bsize;
	
	/* Loop over all clusters in this FAT block */
	while (count > 0) {
	    start++;
	    newc = --count > 0 ? start : fillwith;
	    switch (pmp->pm_fatmask) {
		case FAT12_MASK:
		    readcn = getuint16(entry);
		    if (start & 1) {
			readcn &= 0xf000;
			readcn |= newc & 0xfff;
		    } else {
			readcn &= 0x000f;
			readcn |= newc << 4;
		    }
		    putuint16(entry, readcn);
		    entry++;
		    if (!(start & 1))
			entry++;
		    break;
		case FAT16_MASK:
		    putuint16(entry, newc);
		    entry += 2;
		    break;
		case FAT32_MASK:
		    readcn = getuint32(entry);
		    readcn &= ~pmp->pm_fatmask;
		    readcn |= newc & pmp->pm_fatmask;
		    putuint32(entry, readcn);
		    entry += 4;
		    break;
	    }
	    if (entry >= block_end)
		break;
	}
	pmp->pm_fat_flags |= FAT_CACHE_DIRTY | FSINFO_DIRTY;
    }

done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FATCHAIN|DBG_FUNC_END, error, 0, 0, 0, 0);
    return error;
}

/*
 * Check the length of a free cluster chain starting at start.
 *
 * pmp	 - mount point
 * start - start of chain
 * count - maximum interesting length
 */
uint32_t msdosfs_chainlength(struct msdosfsmount *pmp, uint32_t start, uint32_t count)
{
    uint32_t found = 0;	/* Number of contiguous free clusters found so far */
    uint32_t readcn = 0;    /* A cluster number read from the FAT */
    char *entry;	/* Current FAT entry (points into FAT cache block) */
    char *block_end;	/* End of current entry's FAT cache block */
    uint32_t bo, bsize; /* Offset into, and size of, FAT cache block */
    int error = 0;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_CHAINLENGTH|DBG_FUNC_START, pmp, start, count, 0, 0);

    /* Loop over all clusters from "start" to end of volume. */
    while (found < count && start <= pmp->pm_maxcluster)
    {
	/* Find the entry for the start'th cluster in the FAT */
	entry = msdosfs_fat_map(pmp, start, &bo, &bsize, &error);
	if (!entry)
	{
	    printf("msdosfs chainlength: error %d reading FAT for cluster %u\n", error, start);
	    break;
	}
	
	/* Find the end of the current FAT block. */
	block_end = entry - bo + bsize;
	
	/* Loop over all entries in this FAT block. */
	while (found < count && entry < block_end)
	{
	    /* Extract the value from the current FAT entry into "readcn" */
	    switch (pmp->pm_fatmask)
	    {
	    case FAT12_MASK:
		readcn = getuint16(entry);
		if (start & 1)
		{
		    readcn >>= 4;
		    entry += 2;
		}
		else
		{
		    readcn &= FAT12_MASK;
		    entry++;
		}
		break;
	    case FAT16_MASK:
		readcn = getuint16(entry);
		entry += 2;
		break;
	    case FAT32_MASK:
		readcn = getuint32(entry);
		entry += 4;
		readcn &= FAT32_MASK;
		break;
	    }
	    
	    if (readcn == 0)
		found++;
	    else
		goto done;
	    
	    start++;
	}
    }

done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_CHAINLENGTH|DBG_FUNC_END, error, found, 0, 0, 0);
    return found;
}

/*
 * Allocate contigous free clusters.
 *
 * pmp	      - mount point.
 * start      - start of cluster chain.
 * count      - number of clusters to allocate.
 * fillwith   - put this value into the fat entry for the
 *		last allocated cluster.
 * retcluster - put the first allocated cluster's number here.
 * got	      - how many clusters were actually allocated.
 */
int msdosfs_chainalloc(
	struct msdosfsmount *pmp,
	uint32_t start,
	uint32_t count,
	uint32_t fillwith,
	uint32_t *retcluster,
	uint32_t *got)
{
    int error = 0;

    KERNEL_DEBUG_CONSTANT(MSDOSFS_CHAINALLOC|DBG_FUNC_START, pmp, start, count, fillwith, 0);
    pmp->pm_freeclustercount -= count;
    
    error = msdosfs_fatchain(pmp, start, count, fillwith);
    if (error != 0)
	goto done;
    
    if (retcluster)
	*retcluster = start;
    if (got)
	*got = count;
done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FREESPACE, pmp, pmp->pm_freeclustercount, 0, 0, 0);
    KERNEL_DEBUG_CONSTANT(MSDOSFS_CHAINALLOC|DBG_FUNC_END, error, start, count, 0, 0);
    return error;
}

/*
 * Allocate contiguous free clusters.
 *
 * pmp	      - mount point.
 * start      - preferred start of cluster chain.
 * count      - number of clusters requested.
 * fillwith   - put this value into the fat entry for the
 *		last allocated cluster.
 * retcluster - put the first allocated cluster's number here.
 * got	      - how many clusters were actually allocated.
 */
int msdosfs_clusteralloc_internal(
	struct msdosfsmount *pmp,
	uint32_t start,
	uint32_t count,
	uint32_t fillwith,
	uint32_t *retcluster,
	uint32_t *got)
{
    int error = 0;
    uint32_t len;	/* The number of free clusters at "start" */
    uint32_t cn;	/* A cluster number (loop index) */
    uint32_t l;		/* The number of contiguous free clusters at "cn" */
    uint32_t foundl;	/* The largest contiguous run of free clusters found so far */
    uint32_t foundcn;	/* The starting cluster of largest run found */
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_CLUSTERALLOC_INTERNAL|DBG_FUNC_START, pmp, start, count, fillwith, 0);

    if (start) {
	/*
	 * If the caller had a specific starting cluster, then look there
	 * first.  (This happens when extending an existing file or directory.)
	 *
	 * If there are enough contiguous free clusters at "start", just use them.
	 */
	if ((len = msdosfs_chainlength(pmp, start, count)) >= count)
	{
	    error = msdosfs_chainalloc(pmp, start, count, fillwith, retcluster, got);
	    goto done;
	}
	
	/*
	 * If we fall through here, "len" = the number of contiguous free clusters
	 * starting at "start".
	 */
    } else {
	/* No specific starting point requested, so use the "next free" pointer. */
	start = pmp->pm_nxtfree;
	len = 0;
    }

    /*
     * "foundl" is the largest number of contiguous free clusters found so far.
     * "foundcn" is the corresponding starting cluster.
     */
    foundl = 0;
    foundcn = 0;
    
    /*
     * Scan through the FAT for contiguous free clusters.
     *
     * TODO: Should we just return any free clusters starting at "start"
     * (if it was non-zero), or else the first free clusters at or after
     * pmp->pm_nxtfree (with wrap-around)?
     */
    for (cn = start; cn <= pmp->pm_maxcluster; cn += l+1) {
	/*
	 * TODO: This is wasteful when skipping over a large range of clusters
	 * that are allocated.  Perhaps the routine should return how many
	 * contiguous clusters and whether they were free or allocated?
	 * Or should this routine examine the FAT directly?  Or just have a
	 * "skip used clusters and return the count" routine?
	 */
	l = msdosfs_chainlength(pmp, cn, count);
	if (l >= count)
	{
	    /* We found enough free space, so look here next time. */
	    pmp->pm_nxtfree = cn + count;
	    error = msdosfs_chainalloc(pmp, cn, count, fillwith, retcluster, got);
	    goto done;
	}
	
	/* Keep track of longest free extent found */
	if (l > foundl)
	{
	    foundcn = cn;
	    foundl = l;
	}
    }
    for (cn = CLUST_FIRST; cn < start; cn += l+1) {
	l = msdosfs_chainlength(pmp, cn, count);
	if (l >= count)
	{
	    /* We found enough free space, so look here next time. */
	    pmp->pm_nxtfree = cn + count;
	    error = msdosfs_chainalloc(pmp, cn, count, fillwith, retcluster, got);
	    goto done;
	}
	
	/* Keep track of longest free extent found */
	if (l > foundl)
	{
	    foundcn = cn;
	    foundl = l;
	}
    }

    /*
     * If we get here, there was no single contiguous chain as long as we
     * wanted.  Check to see if we found *any* free chains.
     */
    if (!foundl)
    {
	error = ENOSPC;
	goto done;
    }

    /*
     * There was no single contiguous chain long enough.  If the caller passed
     * a specific starting cluster, and there were free clusters there, then
     * return that chain (under the assumption they're at least contiguous
     * with the previous bit of the file -- which might result in fewer
     * total extents).
     *
     * Otherwise, just return the largest free chain we found.
     *
     * TODO: Should we instead try to use the first free chain at or after
     * pmp->pm_nxtfree?
     */
    if (len)
    {
	/* Don't update pm_nxtfree since we didn't allocate from there. */
	error = msdosfs_chainalloc(pmp, start, len, fillwith, retcluster, got);
    }
    else
    {
	/*
	 * TODO: Is updating pm_nxtfree really correct?  Perhaps only if
	 * foundcn == pmp->pm_nxtfree?  
	 */
	pmp->pm_nxtfree = foundcn + foundl;
	error = msdosfs_chainalloc(pmp, foundcn, foundl, fillwith, retcluster, got);
    }
    
done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_CLUSTERALLOC_INTERNAL|DBG_FUNC_END, error, 0, 0, 0, 0);
    return error;
}

int msdosfs_clusteralloc(
	struct msdosfsmount *pmp,
	uint32_t start,
	uint32_t count,
	uint32_t fillwith,
	uint32_t *retcluster,
	uint32_t *got)
{
    int error;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_CLUSTERALLOC|DBG_FUNC_START, pmp, start, count, fillwith, 0);

    lck_mtx_lock(pmp->pm_fat_lock);
    error = msdosfs_clusteralloc_internal(pmp, start, count, fillwith, retcluster, got);
    lck_mtx_unlock(pmp->pm_fat_lock);
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_CLUSTERALLOC|DBG_FUNC_END, error, 0, 0, 0, 0);

    return error;
}


/*
 * Free a chain of clusters.
 *
 * pmp		- address of the msdosfs mount structure for the filesystem
 *		  containing the cluster chain to be freed.
 * startcluster - number of the 1st cluster in the chain of clusters to be
 *		  freed.
 */
int msdosfs_freeclusterchain(struct msdosfsmount *pmp, uint32_t cluster)
{
    int error = 0;
    uint32_t readcn;
    void *entry;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FREECLUSTERCHAIN|DBG_FUNC_START, pmp, cluster, 0, 0, 0);

    lck_mtx_lock(pmp->pm_fat_lock);
    
    while (cluster >= CLUST_FIRST && cluster <= pmp->pm_maxcluster) {
	entry = msdosfs_fat_map(pmp, cluster, NULL, NULL, &error);
	if (!entry)
	    break;
	
	pmp->pm_freeclustercount++;
	switch (pmp->pm_fatmask) {
	    case FAT12_MASK:
		readcn = getuint16(entry);
		if (cluster & 1) {
		    cluster = readcn >> 4;
		    readcn &= 0x000f;
		    readcn |= MSDOSFSFREE << 4;
		} else {
		    cluster = readcn;
		    readcn &= 0xf000;
		    readcn |= MSDOSFSFREE & 0xfff;
		}
		putuint16(entry, readcn);
		break;
	    case FAT16_MASK:
		cluster = getuint16(entry);
		putuint16(entry, MSDOSFSFREE);
		break;
	    case FAT32_MASK:
		cluster = getuint32(entry);
		putuint32(entry, (MSDOSFSFREE & FAT32_MASK) | (cluster & ~FAT32_MASK));
		break;
	}
	pmp->pm_fat_flags |= FAT_CACHE_DIRTY | FSINFO_DIRTY;
	cluster &= pmp->pm_fatmask;
	if ((cluster | ~pmp->pm_fatmask) >= CLUST_RSRVD)
	    cluster |= ~pmp->pm_fatmask;
    }
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FREESPACE, pmp, pmp->pm_freeclustercount, 0, 0, 0);
    lck_mtx_unlock(pmp->pm_fat_lock);
    
    if (cluster < CLUST_RSRVD)
        printf("msdosfs_freeclusterchain: found out-of-range cluster (%u)\n", cluster);
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FREECLUSTERCHAIN|DBG_FUNC_END, error, cluster, 0, 0, 0);
    
    return error;
}

/*
 * Read in fat blocks to count the number of free clusters.
 */
int msdosfs_count_free_clusters(struct msdosfsmount *pmp)
{
    uint32_t cn, readcn=0;
    int error = 0;
    char *entry, *last_entry;
    u_int32_t offset, block_size;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_COUNT_FREE_CLUSTERS|DBG_FUNC_START, pmp, 0, 0, 0, 0);
    
    /*
     * We're going to be reading the entire FAT, so advise Cluster I/O
     * that it should begin the read now.
     */
    (void) advisory_read(pmp->pm_fat_active_vp, pmp->pm_fat_bytes, 0, pmp->pm_fat_bytes);
    
    /*
     * Figure how many free clusters are in the filesystem by ripping
     * through the fat counting the number of entries whose content is
     * zero.  These represent free clusters.
     */
    pmp->pm_freeclustercount = 0;
    for (cn = CLUST_FIRST; cn <= pmp->pm_maxcluster; ) {
	entry = msdosfs_fat_map(pmp, cn, &offset, &block_size, &error);
	if (!entry)
	    break;
	
	/* Get a pointer past the last valid entry in the block */
	last_entry = entry - offset + block_size;
	
	while (entry < last_entry && cn <= pmp->pm_maxcluster)
	{
	    switch (pmp->pm_fatmask)
	    {
		case FAT12_MASK:
		    readcn = getuint16(entry);
		    if (cn & 1)
		    {
			readcn >>= 4;
			entry += 2;
		    }
		    else
		    {
			readcn &= FAT12_MASK;
			entry++;
		    }
		    break;
		case FAT16_MASK:
		    readcn = getuint16(entry);
		    entry += 2;
		    break;
		case FAT32_MASK:
		    readcn = getuint32(entry);
		    entry += 4;
		    readcn &= FAT32_MASK;
		    break;
	    }
	    
	    if (readcn == 0)
		pmp->pm_freeclustercount++;
	    
	    cn++;
	}
    }

    KERNEL_DEBUG_CONSTANT(MSDOSFS_FREESPACE, pmp, pmp->pm_freeclustercount, 0, 0, 0);
    KERNEL_DEBUG_CONSTANT(MSDOSFS_COUNT_FREE_CLUSTERS|DBG_FUNC_END, error, pmp->pm_freeclustercount, 0, 0, 0);
    return error;
}

struct extent {
    uint32_t	start;
    uint32_t	count;
};

int msdosfs_find_free_extents(struct msdosfsmount *pmp, uint32_t start, uint32_t count);
int msdosfs_find_next_free(struct msdosfsmount *pmp, uint32_t start, uint32_t end, uint32_t maxCount,
			   uint32_t *foundStart, uint32_t *foundCount);
void msdosfs_insert_free_extent(struct msdosfsmount *pmp, uint32_t start, uint32_t count);

/*
 * msdosfs_find_next_free - Search through the FAT looking for a single contiguous
 * extent of free clusters.
 *
 * Searches clusters start through end-1, inclusive.  Exits immediately if it finds
 * at least maxCount contiguous clusters.  The found extent is returned in
 * *foundStart and *foundCount.
 */
int msdosfs_find_next_free(struct msdosfsmount *pmp, uint32_t start, uint32_t end, uint32_t maxCount,
			   uint32_t *foundStart, uint32_t *foundCount)
{
    uint32_t cn = start;    /* Current cluster number being examined. */
    uint32_t found = 0;	    /* Number of contiguous free extents found so far. */
    uint32_t readcn = 0;    /* A cluster number read from the FAT */
    char *entry;	    /* Current FAT entry (points into FAT cache block) */
    char *block_end;	    /* End of current entry's FAT cache block */
    u_int32_t bo, bsize;    /* Offset into, and size of, FAT cache block */
    int error = 0;

    KERNEL_DEBUG_CONSTANT(MSDOSFS_FIND_NEXT_FREE|DBG_FUNC_START, pmp, start, end, maxCount, 0);

    while (found < maxCount && cn < end)
    {
	/* Find the entry for the cn'th cluster in the FAT */
	entry = msdosfs_fat_map(pmp, cn, &bo, &bsize, &error);
	if (!entry)
	{
	    printf("msdosfs_find_next_free: error %d reading FAT for cluster %u\n", error, cn);
	    break;
	}
	
	/* Find the end of the current FAT block. */
	block_end = entry - bo + bsize;
	
	/* Loop over all entries in this FAT block. */
	while (found < maxCount && cn < end && entry < block_end)
	{
	    /* Extract the value from the current FAT entry into "readcn" */
	    switch (pmp->pm_fatmask)
	    {
		case FAT12_MASK:
		    readcn = getuint16(entry);
		    if (cn & 1)
		    {
			readcn >>= 4;
			entry += 2;
		    }
		    else
		    {
			readcn &= FAT12_MASK;
			entry++;
		    }
		    break;
		case FAT16_MASK:
		    readcn = getuint16(entry);
		    entry += 2;
		    break;
		case FAT32_MASK:
		    readcn = getuint32(entry);
		    entry += 4;
		    readcn &= FAT32_MASK;
		    break;
	    }
	    
	    if (readcn == 0)
	    {
		/*
		 * Found a free cluster.  If this was the first one, then
		 * return the start of the extent.
		 */
		if (found == 0)
		{
		    *foundStart = cn;
		}
		++ found;
	    }
	    else
	    {
		/*
		 * Found a used cluster.  If we already found some free
		 * clusters, then we've found the end of a free exent,
		 * and we're done.
		 */
		if (found != 0)
		    goto done;
	    }

	    cn++;
	}
    }
    
done:
    *foundCount = found;
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FIND_NEXT_FREE|DBG_FUNC_END, error, *foundStart, *foundCount, 0, 0);
    return error;
}

void msdosfs_insert_free_extent(struct msdosfsmount *pmp, uint32_t start, uint32_t count)
{
    uint32_t maxExtents = PAGE_SIZE / sizeof(struct extent);
    struct extent *extents = (struct extent *) pmp->pm_free_extents;
    
    /*
     * If the free extent list is full, and the given extent is no better than the
     * worst we already know about, then ignore it.
     */
    if (pmp->pm_free_extent_count == maxExtents && count <= extents[maxExtents-1].count)
	return;

    /*
     * Find the index in the free extent table to insert the given extent.
     * When we're done with the loop, the given extent should be inserted
     * before extents[i].  NOTE: i < maxExtents.
     *
     * TODO: Use a binary search to find the right spot for the insertion.
     */
    uint32_t i = pmp->pm_free_extent_count;
    while (i > 0 && extents[i-1].count < count)
	--i;
    if (i >= maxExtents)
	panic("msdosfs_insert_free_extent: invalid insertion index");
    
    /*
     * Make room for the new extent to be inserted.  Grow the array if it isn't
     * full yet.  Shift any worse extents (index i+1 or larger) down in the array.
     */
    if (pmp->pm_free_extent_count < maxExtents)
	pmp->pm_free_extent_count++;
    size_t bytes = (pmp->pm_free_extent_count - (i + 1)) * sizeof(struct extent);
    if (bytes)
	memmove(&extents[i+1], &extents[i], bytes);
    
    extents[i].start = start;
    extents[i].count = count;
}

/*
 * msdosfs_find_free_extents - Search through the FAT looking for contiguous free clusters.
 * It populates the free extent cache (pm_free_extents).
 *
 * start    - Desired cluster number for starting the search, or zero for anywhere.
 * count    - Maximum number of contiguous clusters needed.
 */
int msdosfs_find_free_extents(struct msdosfsmount *pmp, uint32_t start, uint32_t count)
{
    int error = 0;
    uint32_t next;	/* Cluster number for next search. */
    uint32_t foundStart, foundCount;
    struct extent *extents = pmp->pm_free_extents;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FIND_FREE_EXTENTS|DBG_FUNC_START, pmp, start, count, 0, 0);

    if (start < CLUST_FIRST || start > pmp->pm_maxcluster)
	start = CLUST_FIRST;
    
    /* Forget about extents we found previously. */
    pmp->pm_free_extent_count = 0;
    
    /* Look for free extents from "start" to end of FAT. */
    next = start;
    while (next <= pmp->pm_maxcluster)
    {
	error = msdosfs_find_next_free(pmp, next, pmp->pm_maxcluster+1, count, &foundStart, &foundCount);
	if (error != 0) goto done;
	if (foundCount == count)
	{
	    extents[0].start = foundStart;
	    extents[0].count = foundCount;
	    pmp->pm_free_extent_count = 1;
	    goto done;
	}
	if (foundCount == 0) break;
	msdosfs_insert_free_extent(pmp, foundStart, foundCount);
	next = foundStart + foundCount;
    }

    /* Wrap around.  Look for extents from start of FAT to "start". */
    next = CLUST_FIRST;
    while (next < start)
    {
	error = msdosfs_find_next_free(pmp, next, start, count, &foundStart, &foundCount);
	if (error != 0) goto done;
	if (foundCount == count)
	{
	    extents[0].start = foundStart;
	    extents[0].count = foundCount;
	    pmp->pm_free_extent_count = 1;
	    goto done;
	}
	if (foundCount == 0) break;
	msdosfs_insert_free_extent(pmp, foundStart, foundCount);
	next = foundStart + foundCount;
    }
    
done:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_FIND_FREE_EXTENTS|DBG_FUNC_END, error, pmp->pm_free_extent_count, extents[0].start, extents[0].count, 0);
    return error;
}


/*
 * Allocate new clusters and chain them onto the end of the file.
 *
 * dep	 - the file to extend
 * count - maximum number of clusters to allocate
 * numAllocated - actual number of clusters allocated
 *
 * Allocates up to "count" clusters and appends them to the end of the
 * file's cluster chain.  If fewer than "count" clusters are free, then
 * allocate and append all remaining clusters.
 *
 * NOTE: This function is not responsible for turning on the DE_UPDATE bit of
 * the de_flag field of the denode and it does not change the de_FileSize
 * field.  This is left for the caller to do.
 */
int msdosfs_extendfile(struct denode *dep, uint32_t count, uint32_t *numAllocated)
{
    int error=0;
    uint32_t cn, got, totalAllocated;
    uint32_t i;
    struct msdosfsmount *pmp = dep->de_pmp;
    struct buf *bp = NULL;
    struct extent *extent = NULL;
    
    KERNEL_DEBUG_CONSTANT(MSDOSFS_EXTENDFILE|DBG_FUNC_START, pmp, dep->de_StartCluster, count, 0, 0);
    
    totalAllocated = 0;

    lck_mtx_lock(dep->de_cluster_lock);
    lck_mtx_lock(pmp->pm_fat_lock);
    
    /*
     * Don't try to extend the root directory on FAT12 or FAT16.
     */
    if (dep->de_StartCluster == MSDOSFSROOT
        && (dep->de_Attributes & ATTR_DIRECTORY))
    {
        printf("msdosfs_extendfile: Cannot grow the root directory on FAT12 or FAT16; returning ENOSPC.\n");
    	error = ENOSPC;
    	goto done;
    }
    
    /*
     * If the "file's last cluster" is uninitialized, and the file
     * is not empty, then calculate the last cluster.
     */
    if (dep->de_LastCluster == 0 &&
        dep->de_StartCluster != 0)
    {
        error = msdosfs_pcbmap_internal(dep, 0xFFFFFFFF, 1, NULL, NULL, &dep->de_LastCluster);
        /* we expect it to return E2BIG */
        if (error != E2BIG)
            goto done;
	error = 0;
	
	if (dep->de_LastCluster == 0)
	{
	    printf("msdosfs: msdosfs_extendfile: dep->de_LastCluster == 0!\n");
	    error = EIO;
	    goto done;
	}
    }

    /*
     * First look for free space contiguous with the end of file.
     */
    if (dep->de_StartCluster != 0)
    {
	cn = dep->de_LastCluster + 1;
	
	got = msdosfs_chainlength(pmp, cn, count);
	if (got != 0)
	{
	    error = msdosfs_chainalloc(pmp, cn, got, CLUST_EOFE, NULL, NULL);
	    if (error) goto done;
	    
	    /* See if we need to update the cluster extent cache. */
	    if (cn == (dep->de_LastCluster+1) &&
		cn == (dep->de_cluster_physical + dep->de_cluster_count))
	    {
		/*
		 * We extended the file's last extent, and it was cached, so
		 * update its length to include the new allocation.
		 */
		dep->de_cluster_count += got;
	    }
	    
	    /* Point the old end of file to the newly allocated extent. */
	    error = msdosfs_fatentry_internal(FAT_SET, pmp, dep->de_LastCluster, NULL, cn);
	    if (error)
	    {
		msdosfs_freeclusterchain(pmp, cn);
		goto done;
	    }
	    
	    /*
	     * Clear directory clusters here, file clusters are cleared by the caller
	     */
	    if (dep->de_Attributes & ATTR_DIRECTORY) {
		for (i = 0; i < got; ++i)
		{
		    bp = buf_getblk(pmp->pm_devvp, cntobn(pmp, cn + i), pmp->pm_bpcluster, 0, 0, BLK_META);
		    buf_clear(bp);
		    buf_bdwrite(bp);
		}
	    }

	    count -= got;
	    dep->de_LastCluster += got;
	    totalAllocated += got;
	}
    }
    
    if (count == 0)
	goto done;
    
    /*
     * Look for contiguous free space, populating the free extent cache.
     */
    error = msdosfs_find_free_extents(pmp, pmp->pm_nxtfree, count);
    if (error)
	goto done;
    
    /* Start by using the known free extents found above. */
    for (i = 0, extent = (struct extent *) pmp->pm_free_extents;
	 count > 0 && i < pmp->pm_free_extent_count;
	 ++i, ++extent)
    {
	/* Grab the next largest free extent. */
	cn = extent->start;
	got = extent->count;
	if (got > count)
	    got = count;

	/* Allocate it in the FAT. */
	error = msdosfs_chainalloc(pmp, cn, got, CLUST_EOFE, NULL, NULL);
	if (error) goto done;
	
	/* Point the old end of file to the newly allocated extent. */
	if (dep->de_LastCluster)
	    error = msdosfs_fatentry_internal(FAT_SET, pmp, dep->de_LastCluster, NULL, cn);
	if (error)
	{
	    msdosfs_freeclusterchain(pmp, cn);
	    goto done;
	}
	
	/*
	 * Clear directory clusters here, file clusters are cleared by the caller
	 */
	if (dep->de_Attributes & ATTR_DIRECTORY) {
	    for (i = 0; i < got; ++i)
	    {
		bp = buf_getblk(pmp->pm_devvp, cntobn(pmp, cn + i), pmp->pm_bpcluster, 0, 0, BLK_META);
		buf_clear(bp);
		buf_bdwrite(bp);
	    }
	}

	/* If the file was empty, set the starting cluster. */
	if (dep->de_StartCluster == 0)
	    dep->de_StartCluster = cn;
	/* Update the file's last cluster. */
	dep->de_LastCluster = cn + got - 1;

	count -= got;
	totalAllocated += got;
    }
    
    /*
     * If we used everything in the free extent cache, and it wasn't full, then
     * there is no more free space.
     */
    if (count > 0 && pmp->pm_free_extent_count < (PAGE_SIZE / sizeof(struct extent)))
    {
	error = ENOSPC;
	goto done;
    }
    
    /* Just use whatever free space we can find. */
    while (count > 0)
    {
	/* Find the next free extent and use it. */
	error = msdosfs_find_next_free(pmp, pmp->pm_nxtfree, pmp->pm_maxcluster+1, count, &cn, &got);
	if (error) break;
	if (got == 0)
	{
	    error = ENOSPC;
	    break;
	}
	
	/* Allocate it in the FAT. */
	error = msdosfs_chainalloc(pmp, cn, got, CLUST_EOFE, NULL, NULL);
	if (error) goto done;
	
	/*
	 * Point the old end of file to the newly allocated extent.
	 * NOTE: dep->de_LastCluster must be non-zero by now.
	 */
	error = msdosfs_fatentry_internal(FAT_SET, pmp, dep->de_LastCluster, NULL, cn);
	if (error)
	{
	    msdosfs_freeclusterchain(pmp, cn);
	    goto done;
	}
	
	/*
	 * Clear directory clusters here, file clusters are cleared by the caller
	 */
	if (dep->de_Attributes & ATTR_DIRECTORY) {
	    for (i = 0; i < got; ++i)
	    {
		bp = buf_getblk(pmp->pm_devvp, cntobn(pmp, cn + i), pmp->pm_bpcluster, 0, 0, BLK_META);
		buf_clear(bp);
		buf_bdwrite(bp);
	    }
	}
	
	/*
	 * NOTE: The file couldn't have been empty, so no need to set de_StartCluster.
	 * The file must have had extents allocated via the free extent cache, above.
	 */
	
	/* Update the file's last cluster. */
	dep->de_LastCluster = cn + got - 1;
	
	count -= got;
	totalAllocated += got;
	pmp->pm_nxtfree = cn + got;
    }
    
done:
    lck_mtx_unlock(pmp->pm_fat_lock);
    lck_mtx_unlock(dep->de_cluster_lock);
    if (numAllocated)
	*numAllocated = totalAllocated;
    KERNEL_DEBUG_CONSTANT(MSDOSFS_EXTENDFILE|DBG_FUNC_END, error, totalAllocated, 0, 0, 0);
    return error;
}


/* [2753891]
 * Routine to mark a FAT16 or FAT32 volume as "clean" or "dirty" by manipulating the upper bit
 * of the FAT entry for cluster 1.  Note that this bit is not defined for FAT12 volumes, which
 * are always assumed to be dirty.
 *
 * The msdosfs_fatentry() routine only works on cluster numbers that a file could occupy, so it won't
 * manipulate the entry for cluster 1.  So we have to do it here.  The code is ripped from
 * msdosfs_fatentry(), and tailored for cluster 1.
 * 
 * Inputs:
 *	pmp	The MS-DOS volume to mark
 *	dirty	Non-zero if the volume should be marked dirty; zero if it should be marked clean.
 *
 * Result:
 *	0	Success
 *	EROFS	Volume is read-only
 *	?	(other errors from called routines)
 */
int msdosfs_markvoldirty(struct msdosfsmount *pmp, int dirty)
{
    int error = 0;
    uint32_t fatval;
    void *entry;

    KERNEL_DEBUG_CONSTANT(MSDOSFS_MARKVOLDIRTY|DBG_FUNC_START, pmp, dirty, 0, 0, 0);

    /* FAT12 does not support a "clean" bit, so don't do anything */
    if (FAT12(pmp))
        goto done2;

    /* Can't change the bit on a read-only filesystem */
    if (pmp->pm_flags & MSDOSFSMNT_RONLY)
    {
        error = EROFS;
	goto done2;
    }

    lck_mtx_lock(pmp->pm_fat_lock);

    /* Fetch the block containing the FAT entry */
    entry = msdosfs_fat_map(pmp, 1, NULL, NULL, &error);
    if (!entry)
	goto done;
    
    /* Get the current value of the FAT entry and set/clear the high bit */
    if (FAT32(pmp)) {
        /* FAT32 uses bit 27 */
        fatval = getuint32(entry);
        if (dirty)
            fatval &= 0xF7FFFFFF;	/* dirty means clear the "clean" bit */
        else
            fatval |= 0x08000000;	/* clean means set the "clean" bit */
        putuint32(entry, fatval);
    }
    else {
        /* Must be FAT16; use bit 15 */
        fatval = getuint16(entry);
        if (dirty)
            fatval &= 0x7FFF;		/* dirty means clear the "clean" bit */
        else
            fatval |= 0x8000;		/* clean means set the "clean" bit */
        putuint16(entry, fatval);
    }
    
    /* Write out the modified FAT block immediately */
    pmp->pm_fat_flags |= FAT_CACHE_DIRTY;
    error = msdosfs_fat_cache_flush(pmp, IO_SYNC);

done:
    lck_mtx_unlock(pmp->pm_fat_lock);
done2:
    KERNEL_DEBUG_CONSTANT(MSDOSFS_MARKVOLDIRTY|DBG_FUNC_END, error, 0, 0, 0, 0);
    return error;
}

