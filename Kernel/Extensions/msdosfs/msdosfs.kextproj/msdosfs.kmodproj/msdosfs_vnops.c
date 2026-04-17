/*
 * Copyright (c) 2000-2013 Apple Inc. All rights reserved.
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
/* $FreeBSD: src/sys/msdosfs/msdosfs_vnops.c,v 1.99 2000/05/05 09:58:36 phk Exp $ */
/*	$NetBSD: msdosfs_vnops.c,v 1.68 1998/02/10 14:10:04 mrg Exp $	*/

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
#include <sys/vnode.h>
#include <sys/kernel.h>
#include <sys/stat.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/unistd.h>
#include <sys/vnode.h>
#include <miscfs/specfs/specdev.h>
#include <sys/malloc.h>
#include <sys/dirent.h>
#include <sys/signalvar.h>
#include <sys/ubc.h>
#include <sys/utfconv.h>
#include <sys/attr.h>
#include <sys/namei.h>
#include <libkern/crypto/md5.h>
#include <sys/disk.h>
#include <mach/boolean.h>
#include <libkern/OSMalloc.h>

#include "bpb.h"
#include "direntry.h"
#include "denode.h"
#include "msdosfsmount.h"
#include "fat.h"
#include "msdosfs_kdebug.h"

#ifndef DEBUG
#define DEBUG 0
#endif

/*
 * The maximum file size on FAT is 4GB-1, which is the largest value that fits
 * in an unsigned 32-bit integer.
 */
#define	DOS_FILESIZE_MAX	0xffffffff

union msdosfs_dirbuf {
	struct dirent   dirent;
	struct direntry direntry;
};

/*
 * Prototypes for MSDOSFS vnode operations
 */
int msdosfs_vnop_create(struct vnop_create_args *);
int msdosfs_vnop_mknod(struct vnop_mknod_args *);
int msdosfs_vnop_open(struct vnop_open_args *ap);
int msdosfs_vnop_close(struct vnop_close_args *);
int msdosfs_vnop_getattr(struct vnop_getattr_args *);
int msdosfs_vnop_setattr(struct vnop_setattr_args *);
int msdosfs_vnop_getxattr(struct vnop_getxattr_args *ap);
int msdosfs_vnop_setxattr(struct vnop_setxattr_args *ap);
int msdosfs_vnop_removexattr(struct vnop_removexattr_args *ap);
int msdosfs_vnop_listxattr(struct vnop_listxattr_args *ap);
int msdosfs_vnop_read(struct vnop_read_args *);
int msdosfs_vnop_write(struct vnop_write_args *);
int msdosfs_vnop_pagein(struct vnop_pagein_args *);
int msdosfs_vnop_fsync(struct vnop_fsync_args *);
int msdosfs_vnop_remove(struct vnop_remove_args *);
int msdosfs_vnop_rename(struct vnop_rename_args *);
int msdosfs_vnop_mkdir(struct vnop_mkdir_args *);
int msdosfs_vnop_rmdir(struct vnop_rmdir_args *);
int msdosfs_vnop_readdir(struct vnop_readdir_args *);
int msdosfs_vnop_strategy(struct vnop_strategy_args *);
int msdosfs_vnop_pathconf(struct vnop_pathconf_args *ap);
int msdosfs_vnop_symlink(struct vnop_symlink_args *ap);
int msdosfs_vnop_readlink(struct vnop_readlink_args *ap);
int msdosfs_vnop_ioctl(struct vnop_ioctl_args *ap);
int msdosfs_vnop_pageout(struct vnop_pageout_args *ap);

/* Other prototypes */
void msdosfs_lock_two(struct denode *dep1, struct denode *dep2);
void msdosfs_sort_denodes(struct denode *deps[4]);
void msdosfs_lock_four(struct denode *dep0, struct denode *dep1, struct denode *dep2, struct denode *dep3);
void msdosfs_unlock_four(struct denode *dep0, struct denode *dep1, struct denode *dep2, struct denode *dep3);
void msdosfs_md5_digest(void *text, size_t length, char digest[33]);
ssize_t msdosfs_dirbuf_size(union msdosfs_dirbuf *buf, size_t name_length, int flags);


/*
 * Some general notes:
 *
 * In the ufs filesystem the inodes, superblocks, and indirect blocks are
 * read/written using the vnode for the filesystem. Blocks that represent
 * the contents of a file are read/written using the vnode for the file
 * (including directories when they are read/written as files). This
 * presents problems for the dos filesystem because data that should be in
 * an inode (if dos had them) resides in the directory itself.  Since we
 * must update directory entries without the benefit of having the vnode
 * for the directory we must use the vnode for the filesystem.  This means
 * that when a directory is actually read/written (via read, write, or
 * readdir, or seek) we must use the vnode for the filesystem instead of
 * the vnode for the directory as would happen in ufs. This is to insure we
 * retreive the correct block from the buffer cache since the hash value is
 * based upon the vnode address and the desired block number.
 */


/*
 * msdosfs_lock_two
 *
 * Acquire the denode locks for two denodes.  The locks are always
 * acquired in order of increasing address of the denode.
 */
void msdosfs_lock_two(struct denode *dep1, struct denode *dep2)
{
	if (dep1 == NULL)
		panic("msdosfs_lock_two: dep1 == NULL\n");
	if (dep2 == NULL)
		panic("msdosfs_lock_two: dep2 == NULL\n");
	if (dep1 == dep2)
		panic("msdosfs_lock_two: dep1 == dep2\n");
	
	if (dep1 < dep2)
	{
		lck_mtx_lock(dep1->de_lock);
		lck_mtx_lock(dep2->de_lock);
	}
	else
	{
		lck_mtx_lock(dep2->de_lock);
		lck_mtx_lock(dep1->de_lock);
	}
}

/*
 * Sort a list of denodes into increasing address order.  Remove duplicate addresses.
 * Any unused entries will be NULL.  Some of the entries may be NULL on input.
 */
void msdosfs_sort_denodes(struct denode *deps[4])
{
	int i, j;
	struct denode *temp;
	
	/* A simple bubble sort */
	for (j=3; j>0; --j)
		for (i=0; i<j; ++i)
			if (deps[i] > deps[i+1])
			{
				temp = deps[i];
				deps[i] = deps[i+1];
				deps[i+1] = temp;
			}
	
	/* Remove duplicates */
	for (i=0; i<3; ++i)
		if (deps[i] == deps[i+1])
			deps[i] = NULL;
}

void msdosfs_lock_four(struct denode *dep0, struct denode *dep1, struct denode *dep2, struct denode *dep3)
{
	int i;
	struct denode *deps[4] = {dep0, dep1, dep2, dep3};

	msdosfs_sort_denodes(deps);
	
	for (i=0; i<4; ++i)
		if (deps[i] != NULL)
			lck_mtx_lock(deps[i]->de_lock);
}

void msdosfs_unlock_four(struct denode *dep0, struct denode *dep1, struct denode *dep2, struct denode *dep3)
{
	int i;
	struct denode *deps[4] = {dep0, dep1, dep2, dep3};

	/*
	 * We don't actually care about the order of the denodes when unlocking.
	 * But we do care about removing duplicates.
	 */
	msdosfs_sort_denodes(deps);
	
	for (i=0; i<4; ++i)
		if (deps[i] != NULL)
			lck_mtx_unlock(deps[i]->de_lock);
}

/*
 * Create a regular file.
 */
int msdosfs_vnop_create(struct vnop_create_args *ap)
/* {
		vnode_t a_dvp;
		vnode_t *a_vpp;
		struct componentname *a_cnp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} */
{
	vnode_t dvp = ap->a_dvp;
	struct denode *pdep = VTODE(dvp);
	struct componentname *cnp = ap->a_cnp;
	vfs_context_t context = ap->a_context;
	struct vnode_attr *vap = ap->a_vap;
	struct denode ndirent;
	struct denode *dep = NULL;
	struct timespec ts;
	int error;
	uint32_t offset = 0;		/* byte offset in directory for new entry */
	uint32_t long_count = 0;	/* number of long name entries needed */
	int needs_generation;
	
    KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_CREATE|DBG_FUNC_START, pdep->de_pmp, pdep, 0, 0, 0);
	
	lck_mtx_lock(pdep->de_lock);

	/*
	 * Make sure the parent directory hasn't been deleted.
	 */
	if (pdep->de_refcnt <= 0)
	{
		cache_purge(dvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the name does not exist in the parent directory.  (It didn't
	 * exist during VNOP_LOOKUP, but another thread may have created the name
	 * before we got the lock on the parent.)
	 */
	error = msdosfs_lookup_name(pdep, cnp, NULL, NULL, NULL, NULL, NULL, NULL, context);
	if (error != ENOENT)
	{
		error = EEXIST;
		goto exit;
	}
	
	/*
	 * Find space in the directory to place the new name.
	 */
	bzero(&ndirent, sizeof(ndirent));
	error = msdosfs_findslots(pdep, cnp, ndirent.de_Name, &needs_generation, &ndirent.de_LowerCase, &offset, &long_count, context);
	if (error)
		goto exit;

	/*
	 * If this is the root directory and there is no space left we
	 * can't do anything.  This is because the root directory can not
	 * change size.  (FAT12 and FAT16 only)
	 *
	 * On FAT32, we can grow the root directory, and de_StartCluster
	 * will be the actual cluster number of the root directory.
	 */
	if (pdep->de_StartCluster == MSDOSFSROOT && offset >= pdep->de_FileSize)
	{
        printf("msdosfs_vnop_create: Cannot grow the root directory on FAT12 or FAT16; returning ENOSPC.\n");
		error = ENOSPC;
		goto exit;
	}

	/*
	 * Create a directory entry for the file, then call msdosfs_createde() to
	 * have it installed. NOTE: DOS files are always executable.
	 * The supplied mode is ignored (DOS doesn't store mode, so
	 * all files on a volume have a constant mode).  The immutable
	 * flag is used to set DOS's read-only attribute.
	 */
	error = msdosfs_uniqdosname(pdep, ndirent.de_Name, needs_generation ? offset - long_count * sizeof(struct dosdirentry) : 1, context);
	if (error)
	{
		if (DEBUG) panic("msdosfs_vnop_create: msdosfs_uniqdosname returned %d\n", error);
		goto exit;
	}

	// Set read-only attribute if one of the immutable bits is set.
	// Always set the "needs archive" attribute on newly created files.
	ndirent.de_Attributes = ATTR_ARCHIVE;
	if (VATTR_IS_ACTIVE(vap, va_flags))
	{
		if ((vap->va_flags & (SF_IMMUTABLE | UF_IMMUTABLE)) != 0)
			ndirent.de_Attributes |= ATTR_READONLY;
		VATTR_SET_SUPPORTED(vap, va_flags);
	}
	
	
	/*
	 * If the file name starts with ".", make it invisible on Windows.
	 */
	if (cnp->cn_nameptr[0] == '.')
		ndirent.de_Attributes |= ATTR_HIDDEN;

	ndirent.de_StartCluster = 0;
	ndirent.de_FileSize = 0;
	ndirent.de_dev = pdep->de_dev;
	ndirent.de_devvp = pdep->de_devvp;
	ndirent.de_pmp = pdep->de_pmp;
	ndirent.de_flag = DE_ACCESS | DE_CREATE | DE_UPDATE;
	getnanotime(&ts);
	DETIMES(&ndirent, &ts, &ts, &ts);
	error = msdosfs_createde(&ndirent, pdep, &dep, cnp, offset, long_count, context);
	if (error)
	{
		/*
		 * ENOSPC is a common and expected failure.  Anything else is
		 * unexpected, and I want a chance to debug it.
		 */
		if (DEBUG && error != ENOSPC) panic("msdosfs_vnop_create: msdosfs_createde returned %d\n", error);
		goto exit;
	}
	*ap->a_vpp = DETOV(dep);
	cache_purge_negatives(dvp);

exit:
	msdosfs_meta_flush(pdep->de_pmp, FALSE);
	lck_mtx_unlock(pdep->de_lock);
    KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_CREATE|DBG_FUNC_END, error, dep, offset, long_count, 0);
	return error;
}

int msdosfs_vnop_mknod(struct vnop_mknod_args *ap)
/* {
		vnode_t a_dvp;
		vnode_t *a_vpp;
		struct componentname *a_cnp;
		struct vattr *a_vap;
		vfs_context_t a_context;
	} */
{
#pragma unused (ap)
	/* We don't support special files */
	return EINVAL;
}

int msdosfs_vnop_open(struct vnop_open_args *ap)
/* {
		vnode_t a_vp;
		int a_mode;
		vfs_context_t a_context;
	} */
{
#pragma unused (ap)
	return 0;
}

int msdosfs_vnop_close(struct vnop_close_args *ap)
/* {
		vnode_t a_vp;
		int a_fflag;
		vfs_context_t a_context;
	} */
{
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_CLOSE|DBG_FUNC_START, dep, 0, 0, 0, 0);
	lck_mtx_lock(dep->de_lock);
	
	cluster_push(vp, IO_CLOSE);
	msdosfs_deupdat(dep, 0, ap->a_context);
	msdosfs_meta_flush(dep->de_pmp, FALSE);

	lck_mtx_unlock(dep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_CLOSE|DBG_FUNC_END, 0, 0, 0, 0, 0);
	
	return 0;
}

int msdosfs_vnop_getattr(struct vnop_getattr_args *ap)
/* {
		vnode_t a_vp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} */
{
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);
	struct msdosfsmount *pmp = dep->de_pmp;
	struct vnode_attr *vap = ap->a_vap;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_GETATTR|DBG_FUNC_START, dep, vap->va_active, 0, 0, 0);
	lck_mtx_lock(dep->de_lock);
	
	VATTR_RETURN(vap, va_rdev, 0);
	VATTR_RETURN(vap, va_nlink, 1);
	VATTR_RETURN(vap, va_total_size, dep->de_FileSize);
	/* va_total_alloc is wrong for symlinks */
	VATTR_RETURN(vap, va_total_alloc, ((off_t)dep->de_FileSize + pmp->pm_crbomask) & ~((off_t)pmp->pm_crbomask));
	VATTR_RETURN(vap, va_data_size, dep->de_FileSize);
	VATTR_RETURN(vap, va_data_alloc, vap->va_total_alloc);
	VATTR_RETURN(vap, va_iosize, pmp->pm_iosize);
	VATTR_RETURN(vap, va_uid, 99);
	VATTR_RETURN(vap, va_gid, 99);
	VATTR_RETURN(vap, va_mode, ALLPERMS & pmp->pm_mask);
	if (VATTR_IS_ACTIVE(vap, va_flags)) {
		vap->va_flags = 0;
		/* MSDOS does not set ATTR_ARCHIVE or ATTR_READONLY bits for directories. */
		if ((dep->de_Attributes & (ATTR_ARCHIVE | ATTR_DIRECTORY)) == 0)	// DOS: flag set means "needs to be archived"
			vap->va_flags |= SF_ARCHIVED;				// BSD: flag set means "has been archived"
		if ((dep->de_Attributes & (ATTR_READONLY | ATTR_DIRECTORY)) == ATTR_READONLY)
			vap->va_flags |= UF_IMMUTABLE;				// DOS read-only becomes BSD user immutable
		if (dep->de_Attributes & ATTR_HIDDEN)
			vap->va_flags |= UF_HIDDEN;
		VATTR_SET_SUPPORTED(vap, va_flags);
	}
	
	/* FAT doesn't support extended security data */
	
	if (vap->va_active & (VNODE_ATTR_va_create_time |
		VNODE_ATTR_va_access_time | VNODE_ATTR_va_modify_time |
		VNODE_ATTR_va_change_time))
	{
		struct timespec ts;
		getnanotime(&ts);
		DETIMES(dep, &ts, &ts, &ts);
		
		msdosfs_dos2unixtime(dep->de_CDate, dep->de_CTime, 0, &vap->va_create_time);
		msdosfs_dos2unixtime(dep->de_ADate, 0, 0, &vap->va_access_time);
		msdosfs_dos2unixtime(dep->de_MDate, dep->de_MTime, 0, &vap->va_modify_time);
		vap->va_change_time = vap->va_modify_time;
		/* FAT doesn't have a backup date/time */
		
		vap->va_supported |= VNODE_ATTR_va_create_time |
			VNODE_ATTR_va_access_time |
			VNODE_ATTR_va_modify_time |
			VNODE_ATTR_va_change_time;
	}
	
	if (VATTR_IS_ACTIVE(vap, va_fileid))
		VATTR_RETURN(vap, va_fileid, msdosfs_defileid(dep));
	/* FAT has no va_linkid, and no easy access to va_parentid */

	VATTR_RETURN(vap, va_fsid, dep->de_dev);
	VATTR_RETURN(vap, va_filerev, dep->de_modrev);
	VATTR_RETURN(vap, va_gen, 0);
	
	lck_mtx_unlock(dep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_GETATTR|DBG_FUNC_END, 0, vap->va_supported, 0, 0, 0);
	
	return 0;
}

int msdosfs_vnop_setattr(struct vnop_setattr_args *ap)
	/* {
		vnode_t a_vp;
		struct vnode_attr *a_vap;
		vfs_context_t a_context;
	} */
{
	struct denode *dep = VTODE(ap->a_vp);
	struct vnode_attr *vap = ap->a_vap;
	int error = 0;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_SETATTR|DBG_FUNC_START, dep, vap->va_active, vap->va_data_size, vap->va_flags, 0);
	lck_mtx_lock(dep->de_lock);

	int isDir = dep->de_Attributes & ATTR_DIRECTORY;
	
	if (VATTR_IS_ACTIVE(vap, va_data_size)) {
		if (isDir)
		{
			error = EPERM;	/* Cannot change size of a directory or symlink! */
			goto exit;
		}
		if (dep->de_FileSize != vap->va_data_size) {
			/* msdosfs_detrunc internally updates dep->de_FileSize and calls ubc_setsize. */
			if (vap->va_data_size > DOS_FILESIZE_MAX)
				error = EFBIG;
			else
				error = msdosfs_detrunc(dep, (uint32_t)vap->va_data_size, vap->va_vaflags, ap->a_context);
			if (error)
				goto exit;
		}
		VATTR_SET_SUPPORTED(vap, va_data_size);
	}
	
	/* FAT does not support setting uid, gid or mode */
	
	if (VATTR_IS_ACTIVE(vap, va_flags)) {
		/*
		 * Here we are strict, stricter than ufs in not allowing
		 * users to attempt to set SF_SETTABLE bits or anyone to
		 * set unsupported bits.  However, we ignore attempts to
		 * set ATTR_ARCHIVE for directories `cp -pr' from a more
		 * sensible file system attempts it a lot.
		 */
        
		if (vap->va_flags & ~(SF_ARCHIVED | SF_IMMUTABLE | UF_IMMUTABLE | UF_HIDDEN))
		{
			error = EINVAL;
			goto exit;
		}
		
		uint8_t originalAttributes = dep->de_Attributes;
		
		if (vap->va_flags & SF_ARCHIVED)
			dep->de_Attributes &= ~ATTR_ARCHIVE;
		else if (!isDir)
			dep->de_Attributes |= ATTR_ARCHIVE;

		/* For files, copy the immutable flag to read-only attribute. */
		/* Ignore immutable bit for directories. */
		if (!isDir)
		{
			if (vap->va_flags & (SF_IMMUTABLE | UF_IMMUTABLE))
				dep->de_Attributes |= ATTR_READONLY;
			else
				dep->de_Attributes &= ~ATTR_READONLY;
		}
        
        if (vap->va_flags & UF_HIDDEN)
        	dep->de_Attributes |= ATTR_HIDDEN;
        else
        	dep->de_Attributes &= ~ATTR_HIDDEN;

		if (dep->de_Attributes != originalAttributes)
			dep->de_flag |= DE_MODIFIED | DE_ATTR_MOD;
		
		VATTR_SET_SUPPORTED(vap, va_flags);
	}

	/*
	 * Update times.  Since we don't explicitly store a change time, we
	 * don't let you set it here.  (An alternative behavior would be to
	 * set the denode's mod time to the greater of va_modify_time and
	 * va_change_time.)
	 */
	if (VATTR_IS_ACTIVE(vap, va_create_time) |
		VATTR_IS_ACTIVE(vap, va_access_time) |
		VATTR_IS_ACTIVE(vap, va_modify_time))
	{
		if (VATTR_IS_ACTIVE(vap, va_create_time)) {
			msdosfs_unix2dostime(&vap->va_create_time, &dep->de_CDate, &dep->de_CTime, NULL);
			VATTR_SET_SUPPORTED(vap, va_create_time);
		}
		if (VATTR_IS_ACTIVE(vap, va_access_time)) {
			msdosfs_unix2dostime(&vap->va_access_time, &dep->de_ADate, NULL, NULL);
			VATTR_SET_SUPPORTED(vap, va_access_time);
		}
		if (VATTR_IS_ACTIVE(vap, va_modify_time)) {
			msdosfs_unix2dostime(&vap->va_modify_time, &dep->de_MDate, &dep->de_MTime, NULL);
			VATTR_SET_SUPPORTED(vap, va_modify_time);
		}
		dep->de_Attributes |= ATTR_ARCHIVE;
		dep->de_flag |= DE_MODIFIED;
	}

	error = msdosfs_deupdat(dep, 1, ap->a_context);
	msdosfs_meta_flush(dep->de_pmp, FALSE);

exit:
	lck_mtx_unlock(dep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_SETATTR|DBG_FUNC_END, error, vap->va_supported, 0, 0, 0);
	return error;
}

static char MSDOSFS_XATTR_VOLUME_ID_NAME[] = "com.apple.filesystems.msdosfs.volume_id";

static bool is_xattr_volume_id_name(const char* name)
{
    return !strcmp(name, MSDOSFS_XATTR_VOLUME_ID_NAME);
}

int msdosfs_vnop_getxattr(struct vnop_getxattr_args *ap)
/* {
 struct vnodeop_desc *a_desc;
 vnode_t a_vp;
 const char * a_name;
 uio_t a_uio;
 size_t *a_size;
 int a_options;
 vfs_context_t a_context;
 } */
{
	if (vnode_isvroot(ap->a_vp) && is_xattr_volume_id_name(ap->a_name))
	{
		struct denode *dep = VTODE(ap->a_vp);
		struct msdosfsmount *pmp = dep->de_pmp;
		uio_t uio = ap->a_uio;
		
		/* Make sure the volume actually has a serial number. */
		if (!(pmp->pm_flags & MSDOSFS_HAS_EXT_BOOT))
			return ENOATTR;
		
		/* Return the volume serial number */
		if (uio == NULL)
		{
			*ap->a_size = sizeof(pmp->pm_volume_serial_num);
			return 0;
		}
		else if ((user_size_t)uio_resid(uio) < sizeof(pmp->pm_volume_serial_num))
		{
			return ERANGE;
		}
		else
		{
			return uiomove((char *)pmp->pm_volume_serial_num, sizeof(pmp->pm_volume_serial_num), ap->a_uio);
		}
	}

	/* Let VFS handle all other extended attributes. */
	return ENOTSUP;
}


int msdosfs_vnop_setxattr(struct vnop_setxattr_args *ap)
/* {
 struct vnodeop_desc *a_desc;
 vnode_t a_vp;
 const char * a_name;
 uio_t a_uio;
 int a_options;
 vfs_context_t a_context;
 } */
{
	if (vnode_isvroot(ap->a_vp) && is_xattr_volume_id_name(ap->a_name))
	{
		return EPERM;
	}
	
	/* Let VFS handle all other extended attributes. */
	return ENOTSUP;
}


int msdosfs_vnop_removexattr(struct vnop_removexattr_args *ap)
/* {
 struct vnodeop_desc *a_desc;
 vnode_t a_vp;
 const char * a_name;
 int a_options;
 vfs_context_t a_context;
 } */
{
	if (vnode_isvroot(ap->a_vp) && is_xattr_volume_id_name(ap->a_name))
	{
		return EPERM;
	}
	
	/* Let VFS handle all other extended attributes. */
	return ENOTSUP;
}


int msdosfs_vnop_listxattr(struct vnop_listxattr_args *ap)
/* {
 struct vnodeop_desc *a_desc;
 vnode_t a_vp;
 uio_t a_uio;
 size_t *a_size;
 int a_options;
 vfs_context_t a_context;
 } */
{
	/* Return our xattrs, then return ENOTSUP to let VFS add the rest from AppleDouble files. */
	if (vnode_isvroot(ap->a_vp))
	{
		struct denode *dep = VTODE(ap->a_vp);
		struct msdosfsmount *pmp = dep->de_pmp;

		if (pmp->pm_flags & MSDOSFS_HAS_EXT_BOOT)
		{
			/*
			 * The volume has a serial number, so return its name.
			 */
			
			uio_t uio = ap->a_uio;

			if (uio == NULL)
			{
				/* Just update the size */
				*ap->a_size += sizeof(MSDOSFS_XATTR_VOLUME_ID_NAME);
			}
			else if (uio_resid(uio) < (user_ssize_t)sizeof(MSDOSFS_XATTR_VOLUME_ID_NAME))
			{
				return ERANGE;
			}
			else
			{
				int error = uiomove(MSDOSFS_XATTR_VOLUME_ID_NAME, sizeof(MSDOSFS_XATTR_VOLUME_ID_NAME), uio);
				if (error)
					return ERANGE;
			}
		}
	}

	return ENOTSUP;
}


int msdosfs_vnop_read(struct vnop_read_args *ap)
/* {
		vnode_t a_vp;
		struct uio *a_uio;
		int a_ioflag;
		vfs_context_t a_context;
	} */
{
	int error = 0;
	user_ssize_t orig_resid;
	vnode_t vp = ap->a_vp;
	struct uio *uio = ap->a_uio;
	vfs_context_t context = ap->a_context;
	struct denode *dep = VTODE(vp);
	struct msdosfsmount *pmp = dep->de_pmp;

	if (uio_offset(uio) < 0)
		return EINVAL;

	if (uio_offset(uio) > DOS_FILESIZE_MAX)
		return 0;

	/* If they didn't ask for any data, then we are done. */
	orig_resid = uio_resid(uio);
	if (orig_resid <= 0)
		return 0;

	lck_mtx_lock(dep->de_lock);
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_READ|DBG_FUNC_START, dep, uio_offset(uio), uio_resid(uio), dep->de_FileSize, 0);

	if (vnode_isreg(vp)) {
		error = cluster_read(vp, uio, (off_t)dep->de_FileSize, ap->a_ioflag);
		if (error == 0 && (vfs_flags(pmp->pm_mountp) & (MNT_RDONLY | MNT_NOATIME)) == 0)
			dep->de_flag |= DE_ACCESS;
	}
	else
	{
		uint32_t blsize;
		u_int n;
		uint32_t diff;
		uint32_t on;
		daddr64_t lbn;
		buf_t bp;

		/* The following code is only used for reading directories */
		
		do {
			if (uio_offset(uio) >= dep->de_FileSize)
				break;
			lbn = de_cluster(pmp, uio_offset(uio));
			/*
			 * If we are operating on a directory file then be sure to
			 * do i/o with the vnode for the filesystem instead of the
			 * vnode for the directory.
			 */
			/* convert cluster # to block # */
			error = msdosfs_pcbmap(dep, (uint32_t)lbn, 1, &lbn, NULL, &blsize);
			if (error == E2BIG) {
				error = EINVAL;
				break;
			} else if (error)
				break;
			error = (int)buf_meta_bread(pmp->pm_devvp, lbn, blsize, vfs_context_ucred(context), &bp);
			if (error) {
				buf_brelse(bp);
				break;
			}
			if (ISSET(ap->a_ioflag, IO_NOCACHE) && buf_fromcache(bp) == 0)
				buf_markaged(bp);
			on = uio_offset(uio) & pmp->pm_crbomask;
			diff = pmp->pm_bpcluster - on;
			n = diff > (uint32_t)uio_resid(uio) ? (uint32_t)uio_resid(uio) : diff;
			diff = (uint32_t)(dep->de_FileSize - uio_offset(uio));
			if (diff < n)
				n = diff;
			diff = blsize - buf_resid(bp);
			if (diff < n)
				n = diff;
			error = uiomove((char *)buf_dataptr(bp) + on, (int) n, uio);
			buf_brelse(bp);
		} while (error == 0 && uio_resid(uio) > 0 && n != 0);
	}

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_READ|DBG_FUNC_END, error, uio_offset(uio), uio_resid(uio), dep->de_FileSize, 0);	
	lck_mtx_unlock(dep->de_lock);
	
	return error;
}

/*
 * Write data to a file.
 */
int msdosfs_vnop_write(struct vnop_write_args *ap)
/* {
		vnode_t a_vp;
		struct uio *a_uio;
		int a_ioflag;
		vfs_context_t a_context;
	} */
{
	int error = 0;
	vnode_t vp = ap->a_vp;
	struct uio *uio = ap->a_uio;
	int ioflag = ap->a_ioflag;
	vfs_context_t context = ap->a_context;
	struct denode *dep = VTODE(vp);
	struct msdosfsmount *pmp = dep->de_pmp;
	off_t zero_off;
	uint32_t original_size;
    uint32_t original_clusters;
	uint32_t filesize;
	int   lflag;
	user_ssize_t original_resid;
	user_ssize_t partial_resid;
	off_t original_offset;
	off_t offset;

	switch (vnode_vtype(vp)) {
	case VREG:
		break;
	case VDIR:
		return EISDIR;
	default:
		panic("msdosfs_vnop_write: bad file type");
		return EINVAL;
	}
	
	lck_mtx_lock(dep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_WRITE|DBG_FUNC_START, dep, uio_offset(uio), uio_resid(uio), dep->de_FileSize, 0);
	
	/*
	 * Remember some values in case the write fails.
	 */
	partial_resid = 0;
	original_resid = uio_resid(uio);
	original_size = filesize = dep->de_FileSize;
    original_clusters = de_clcount(pmp, original_size);
	original_offset = uio_offset(uio);
	offset = original_offset;

	if (ioflag & IO_APPEND) {
		uio_setoffset(uio, dep->de_FileSize);
		offset = dep->de_FileSize;
	}

	if (offset < 0)
	{
		error = EFBIG;
		goto exit;
	}

	if (original_resid == 0)
	{
		goto exit;
	}

	if (offset + original_resid > DOS_FILESIZE_MAX)
	{
		error = EFBIG;
		goto exit;
	}

	/*
	 * If the end of the write will be beyond the current end of file,
	 * then grow the file now to accommodate the bytes we want to write.
	 */
    if (offset + original_resid > original_size) {
		uint32_t clusters_needed, clusters_got;
		filesize = (uint32_t)(offset + original_resid);
        clusters_needed = de_clcount(pmp, filesize) - original_clusters;
		if (clusters_needed)
			error = msdosfs_extendfile(dep, clusters_needed, &clusters_got);

		if (error == ENOSPC)
		{
			/*
			 * We got fewer clusters than we wanted.  Set the file size
			 * to the end of the last cluster we now have.
			 */
			filesize = de_cn2off(pmp, original_clusters + clusters_got);
			if (filesize > offset)
			{
				/*
				 * We allocated enough to be able to do a partial write.
				 * Limit the I/O amount to not exceed the new end of file.
				 * Keep track of the number of bytes beyond the new end
				 * of file that we're unable to write, so we can add them
				 * back in later.
				 */
				uio_setresid(uio, filesize-offset);
				partial_resid = offset + original_resid - filesize;
				error = 0;
			}
		}
		if (error)
			goto errexit;
    }
	
	lflag = ioflag;

	/*
	 * If the offset we are starting the write at is beyond the end of
	 * the file, then they've done a seek.  Unix filesystems allow
	 * files with holes in them.  DOS doesn't, so we must fill the hole
	 * with zeroed blocks.
	 */
    if (offset > original_size) {
		zero_off = original_size;
		lflag   |= IO_HEADZEROFILL;
	} else
		zero_off = 0;

	/* Write the data, and any zero filling */
	error = cluster_write(vp, uio, (off_t)original_size, (off_t)filesize,
				(off_t)zero_off, (off_t)0, lflag);
	
	if (uio_offset(uio) > dep->de_FileSize) {
		dep->de_FileSize = (uint32_t)uio_offset(uio);
		ubc_setsize(vp, (off_t)dep->de_FileSize);
	}
	
	if (partial_resid)
		uio_setresid(uio, uio_resid(uio) + partial_resid);

	if (original_resid > uio_resid(uio))
		dep->de_flag |= DE_UPDATE;
	
	/*
	 * If the write failed and they want us to, truncate the file back
	 * to the size it was before the write was attempted.
	 */
errexit:
	if (error) {
		if (ioflag & IO_UNIT) {
			/* msdosfs_detrunc internally updates dep->de_FileSize and calls ubc_setsize. */
			msdosfs_detrunc(dep, original_size, ioflag, context);
			uio_setoffset(uio, original_offset);
			uio_setresid(uio, original_resid);
		} else {
			/* msdosfs_detrunc internally updates dep->de_FileSize and calls ubc_setsize. */
			msdosfs_detrunc(dep, dep->de_FileSize, ioflag, context);
		}
	} else if (ioflag & IO_SYNC)
		error = msdosfs_deupdat(dep, 1, context);
	msdosfs_meta_flush(pmp, (ioflag & IO_SYNC));

exit:
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_WRITE|DBG_FUNC_END, error, uio_offset(uio), uio_resid(uio), dep->de_FileSize, 0);
	lck_mtx_unlock(dep->de_lock);
	return error;
}

/*
 * Read one or more VM pages from a file on disk.
 *
 * This routine assumes that the denode's de_lock has already been acquired
 * (such as inside of msdosfs_vnop_read).
 *
 * [It wouldn't make sense to call this routine if the file's size or
 * location on disk could be changing.  If it could, then the page(s)
 * passed in could be invalid before we could reference them.]
 */
int msdosfs_vnop_pagein(struct vnop_pagein_args *ap)
/* {
		vnode_t a_vp;
		upl_t a_pl;
		vm_offset_t a_pl_offset;
		off_t a_f_offset;
		size_t a_size;
		int a_flags;
		vfs_context_t a_context;
	} */
{
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);
	int error;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_PAGEIN|DBG_FUNC_START, dep, ap->a_f_offset, ap->a_size, dep->de_FileSize, 0);
	error = cluster_pagein(vp, ap->a_pl, ap->a_pl_offset, ap->a_f_offset,
				(int)ap->a_size, (off_t)dep->de_FileSize,
				ap->a_flags);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_PAGEIN|DBG_FUNC_END, error, 0, 0, 0, 0);
	
	return error;
}

/*
 * Write one or more VM pages to a file on disk.
 *
 * This routine assumes that the denode's de_lock has already been acquired
 * (such as inside of msdosfs_vnop_write).
 *
 * [It wouldn't make sense to call this routine if the file's size or
 * location on disk could be changing.  If it could, then the page(s)
 * passed in could be invalid before we could reference them.]
 */
int msdosfs_vnop_pageout(struct vnop_pageout_args *ap)
/* {
		vnode_t a_vp;
		upl_t a_pl;
		vm_offset_t a_pl_offset;
		off_t a_f_offset;
		size_t a_size;
		int a_flags;
		vfs_context_t a_context;
	} */
{
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);
	int error;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_PAGEOUT|DBG_FUNC_START, dep, ap->a_f_offset, ap->a_size, dep->de_FileSize, 0);
	error = cluster_pageout(vp, ap->a_pl, ap->a_pl_offset, ap->a_f_offset,
				(int)ap->a_size, (off_t)dep->de_FileSize,
				ap->a_flags);
	if (!error)
		dep->de_flag |= DE_UPDATE;
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_PAGEOUT|DBG_FUNC_END, error, 0, 0, 0, 0);
	
	return error;
}

/*
 * Assumes the denode's de_lock is already acquired.
 */
int msdosfs_fsync_internal(vnode_t vp, int sync, int do_dirs, vfs_context_t context)
{
	int error;
	struct denode *dep = VTODE(vp);
	
	/*
	 * First of all, write out any clusters.
	 */
	cluster_push(vp, sync ? IO_SYNC : 0);

	/*
	 * Flush all dirty buffers associated with a vnode.
	 */
	buf_flushdirtyblks(vp, sync, 0, "msdosfs_fsync_internal");

	if (do_dirs && (dep->de_Attributes & ATTR_DIRECTORY))
		(void) msdosfs_dir_flush(dep, sync);

	error = msdosfs_deupdat(dep, sync, context);
	
	return error;
}

/*
 * Flush the blocks of a file to disk.
 *
 * This function is worthless for vnodes that represent directories. Maybe we
 * could just do a sync if they try an fsync on a directory file.
 *
 * NOTE: This gets called for every vnode, with a_waitfor=MNT_WAIT, during
 * an unmount (via vflush and vclean).  Possible optimization: keep track
 * of whether the file has dirty content, dirty FAT blocks, or dirty
 * directory blocks; skip the msdosfs_fsync_internal and msdosfs_meta_flush
 * if nothing is dirty.
 */
int msdosfs_vnop_fsync(struct vnop_fsync_args *ap)
/* {
		vnode_t a_vp;
		int a_waitfor;
		vfs_context_t a_context;
	} */
{
	int error;
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);
	
	/*
	 * Skip everything if there was no denode.  This can happen
	 * If the vnode was temporarily created in msdosfs_check_link.
	 */
	if (dep == NULL)
		return 0;

	if (dep->de_pmp->pm_flags & MSDOSFSMNT_RONLY) {
		/* For read-only mounts, there's nothing to do */
		return 0;
	}
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_FSYNC|DBG_FUNC_START, dep, 0, 0, 0, 0);

	lck_mtx_lock(dep->de_lock);

	/* sync dirty buffers associated with this vnode */
	error = msdosfs_fsync_internal(vp, (ap->a_waitfor == MNT_WAIT), TRUE, ap->a_context);

	/*
	 * Flush primary copy of FAT and all directory blocks delayed or asynchronously.
	 *
	 * Ideally, we would do this synchronously if MNT_WAIT was given.  But that
	 * leads to poor performance because every unlink_rmdir causes the vnode to
	 * be recycled, and VFS does a VNOP_FSYNC(..., MNT_WAIT, ...), causing us
	 * to flush ALL dirty metadata synchronously.  Ouch.
	 *
	 * Note that HFS with journal does not write the dirty metadata in this
	 * case, either.  And there is always fcntl(F_FULLFSYNC) if the user really
	 * wants to be sure the data has made it to the media.
	 */ 
	msdosfs_meta_flush(dep->de_pmp, FALSE);

	lck_mtx_unlock(dep->de_lock);
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_FSYNC|DBG_FUNC_END, error, 0, 0, 0, 0);

	return error;
}

/*
 * Remove (unlink) a file.
 */
int msdosfs_vnop_remove(struct vnop_remove_args *ap)
/* {
		vnode_t a_dvp;
		vnode_t a_vp;
		struct componentname *a_cnp;
		int a_flags;
		vfs_context_t a_context;
	} */
{
    vnode_t vp = ap->a_vp;
    vnode_t dvp = ap->a_dvp;
    struct denode *dep = VTODE(vp);
    struct denode *ddep = VTODE(dvp);
    int error;
	uint32_t cluster, offset;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_REMOVE|DBG_FUNC_START, ddep, dep, 0, 0, 0);
	
	msdosfs_lock_two(ddep, dep);
	
	/*
	 * Make sure the parent directory hasn't been deleted.
	 */
	if (ddep->de_refcnt <= 0)
	{
		cache_purge(dvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the child denode hasn't been deleted.
	 */
	if (dep->de_refcnt <= 0)
	{
		cache_purge(vp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the child still has the same name.
	 */
	error = msdosfs_lookup_name(ddep, ap->a_cnp, &cluster, &offset, NULL, NULL, NULL, NULL, ap->a_context);
	if (error || cluster != dep->de_dirclust || offset != dep->de_diroffset)
	{
		cache_purge(vp);
		error = ENOENT;
		goto exit;
	}
	
    /* Make sure the file isn't read-only */
    /* Note that this is an additional imposition over the
       normal deletability rules... */
    if (dep->de_Attributes & ATTR_READONLY)
    {
        error = EPERM;
        goto exit;
    }

	/* Don't allow deletes of busy files (option used by Carbon) */
	if ((ap->a_flags & VNODE_REMOVE_NODELETEBUSY) && vnode_isinuse(vp, 0))
	{
		error = EBUSY;
		goto exit;
	}

	cache_purge(vp);
	dep->de_refcnt--;
	if (DEBUG && dep->de_refcnt < 0)
		panic("msdosfs_vnop_remove: de_refcnt went negative");
    error = msdosfs_removede(ddep, dep->de_diroffset, ap->a_context);
	if (DEBUG && error) panic("msdosfs_vnop_remove: msdosfs_removede returned %d\n", error);
	msdosfs_meta_flush(ddep->de_pmp, FALSE);
	
exit:
	lck_mtx_unlock(ddep->de_lock);
	lck_mtx_unlock(dep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_REMOVE|DBG_FUNC_END, error, 0, 0, 0, 0);
	return error;
}

/*
 * Renames on files require moving the denode to a new hash queue since the
 * location of the directory entry is used to compute the hash.
 *
 * What follows is the basic algorithm:
 *
 * if (file move) {
 *	if (dest file exists) {
 *		remove dest file
 *	}
 *	if (dest and src in same directory) {
 *		rewrite name in existing directory slot
 *	} else {
 *		write new entry in dest directory
 *		update offset and dirclust in denode
 *		move denode to new hash chain
 *		clear old directory entry
 *	}
 * } else {
 *	directory move
 *	if (dest directory exists) {
 *		if (dest is not empty) {
 *			return ENOTEMPTY
 *		}
 *		remove dest directory
 *	}
 *	if (dest and src in same directory) {
 *		rewrite name in existing entry
 *	} else {
 *		be sure dest is not a child of src directory
 *		write entry in dest directory
 *		update "." and ".." in moved directory
 *		clear old directory entry for moved directory
 *	}
 * }
 *
 * Locking:
 *	On entry, VFS has locked all of the vnodes in an
 *	arbitrary, but consistent, order.  For example,
 *	it may lock the parent with the smallest vnode pointer,
 *	then its child, then the parent with the larger vnode
 *	pointer, then its child.
 *
 *	However, VFS doesn't acquire the locks until after the
 *	lookups on both source and destination have completed.
 *	It is possible that the source or destination have been
 *	deleted or renamed between the lookup and locking the vnode.
 *	So we must be paranoid and make sure things haven't changed
 *	since VFS locked the vnodes.
 *
 *	Traditionally, one of the hardest parts about rename's
 *	locking is when the source is a directory, and we
 *	have to verify that the destination parent isn't a
 *	descendant of the source.  We need to walk up the
 *	directory hierarchy from the destination parent up to
 *	the root.  If any of those directories is the source,
 *	the operation is invalid.  But walking up the hierarchy
 *	violates the top-down lock order; to avoid deadlock, we
 *	must not have two nodes locked at the same time.
 *
 *	But there is a solution: a mutex on rename operations that
 *	reshape the hierarchy (i.e. the source and destination parents
 *	are different).  During the walk up the hierarchy, we must
 *	prevent any change to the hierarchy that could cause the
 *	destination parent to become or stop being an ancestor of
 *	the source.  Any operation on a file can't affect the
 *	ancestry of directories.  Directory create operations can't
 *	affect the ancestry of pre-existing directories.
 *	Directory delete operations outside the ancestry path
 *	don't matter, and deletes within the ancestry path will
 *	fail as long as the directories remain locked (the delete
 *	will fail because the directory is locked, or not empty, or
 *	both).  The only operation that can affect the ancestry is
 *	other rename operations on the same volume (and even then,
 *	only if the source parent is not the destination parent).
 *
 *	It is sufficient if the rename mutex is taken only
 *	when the source parent is not the destination parent.
 */
int msdosfs_vnop_rename(struct vnop_rename_args *ap)
/* {
		vnode_t a_fdvp;
		vnode_t a_fvp;
		struct componentname *a_fcnp;
		vnode_t a_tdvp;
		vnode_t a_tvp;
		struct componentname *a_tcnp;
		vfs_context_t a_context;
	} */
{
	vnode_t tdvp = ap->a_tdvp;
	vnode_t fvp = ap->a_fvp;
	vnode_t fdvp = ap->a_fdvp;
	vnode_t tvp = ap->a_tvp;
	struct componentname *tcnp = ap->a_tcnp;
	vfs_context_t context = ap->a_context;
	u_char toname[SHORT_NAME_LEN], oldname[SHORT_NAME_LEN];
	uint32_t to_diroffset;
	uint32_t to_long_count;
    int needs_generation;
	u_int32_t from_offset;
	u_int8_t new_deLowerCase;	/* deLowerCase corresponding to toname */
	int doingdirectory = 0, newparent = 0;
    int change_case;
	int error;
	uint32_t cn;
	daddr64_t bn = 0;
	struct denode *fddep;	/* from file's parent directory	 */
	struct denode *fdep;	/* from file or directory	 */
	struct denode *tddep;	/* to file's parent directory	 */
	struct denode *tdep;	/* to file or directory		 */
	struct msdosfsmount *pmp;
	struct buf *bp;
	uint32_t cluster, offset;
	
	fddep = VTODE(fdvp);
	fdep = VTODE(fvp);
	tddep = VTODE(tdvp);
	tdep = tvp ? VTODE(tvp) : NULL;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_RENAME|DBG_FUNC_START, fddep, fdep, tddep, tdep, 0);
	
	msdosfs_lock_four(fddep, fdep, tddep, tdep);
	
	pmp = fddep->de_pmp;

	/*
	 * VNOP_RENAME is different from other VNOPs to non-thread-safe file
	 * systems.  Because two paths are involved, VFS does not lock the
	 * parent vnodes before the VNOP_LOOKUPs.  It locks all four vnodes
	 * after all of the lookups are done (and all vnodes referenced).
	 *
	 * This means that the source and destination objects may no longer
	 * exist in the namespace, and that the children may no longer be
	 * children of the given parents (the child may have been renamed
	 * by another thread).  And the children may not have the names you
	 * find in the component name.
	 *
	 * I think the best we can do is to try to verify that the vnodes
	 * still have the indicated relationship.  Plus, we need to check that
	 * the desintation name does not exist in the desintation directory.
	 * (A wrinkle here is that a case variant of the desintation name may
	 * exist, and it may be the source!
	 */
	
	/*
	 * Make sure the from parent directory hasn't been deleted.
	 */
	if (fddep->de_refcnt <= 0)
	{
		cache_purge(fdvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the from child denode hasn't been deleted.
	 */
	if (fdep->de_refcnt <= 0)
	{
		cache_purge(fvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the from child still has the same name.
	 */
	error = msdosfs_lookup_name(fddep, ap->a_fcnp, &cluster, &offset, NULL, NULL, NULL, NULL, context);
	if (error || cluster != fdep->de_dirclust || offset != fdep->de_diroffset)
	{
		cache_purge(fvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the to parent directory hasn't been deleted.
	 */
	if (tddep->de_refcnt <= 0)
	{
		cache_purge(tdvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * If there was a to child, make sure it hasn't been deleted.
	 */
	if (tdep && tdep->de_refcnt <= 0)
	{
		cache_purge(tvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Look for the destination name in the destination directory.  If it exists,
	 * it had better be tdep; otherwise, tdep had better be NULL.
	 */
	error = msdosfs_lookup_name(tddep, tcnp, &cluster, &offset, NULL, NULL, NULL, NULL, context);
	if (tdep)
	{
		/* We think the destination exists... */
		if (error || cluster != tdep->de_dirclust || offset != tdep->de_diroffset)
		{
			error = ERESTART;
			goto exit;
		}	
	}
	else
	{
		/* We think the destination does not exist... */
		if (error != ENOENT)
		{
			error = EEXIST;
			goto exit;
		}
	}
	
	/*
	 * If source and dest are the same, then it is a rename
	 * that may be changing the upper/lower case of the name.
	 */
    change_case = 0;
	if (tvp == fvp) {
        /* Pretend the destination doesn't exist. */
        tvp = NULL;
		
        /* Remember we're changing case, so we can skip msdosfs_uniqdosname() */
        change_case = 1;
	}

	/* 
	 * If there is a destination, and it's a file, make sure it isn't
	 * read-only.
	 */
	if (tdep && (tdep->de_Attributes & (ATTR_READONLY | ATTR_DIRECTORY)) == ATTR_READONLY)
	{
		error = EPERM;
		goto exit;
	}
    
	/* If the source is a file, make sure it isn't read-only. */
	if ((fdep->de_Attributes & (ATTR_READONLY | ATTR_DIRECTORY)) == ATTR_READONLY)
	{
		error = EPERM;
		goto exit;
	}

	/*
	 * Figure out where to put the new directory entry.
	 */
	error = msdosfs_findslots(tddep, tcnp, toname, &needs_generation, &new_deLowerCase, &to_diroffset, &to_long_count, context);
	if (error)
		goto exit;

	/*
	 * If this is the root directory and there is no space left we
	 * can't do anything.  This is because the root directory can not
	 * change size.  (FAT12 and FAT16 only)
	 *
	 * On FAT32, we can grow the root directory, and de_StartCluster
	 * will be the actual cluster number of the root directory.
	 */
	if (tddep->de_StartCluster == MSDOSFSROOT && to_diroffset >= tddep->de_FileSize)
	{
        printf("msdosfs_vnop_rename: Cannot grow the root directory on FAT12 or FAT16; returning ENOSPC.\n");
		error = ENOSPC;
		goto exit;
	}

	if (fdep->de_Attributes & ATTR_DIRECTORY)
		doingdirectory = 1;
	
	/*
	 * If ".." must be changed (ie the directory gets a new
	 * parent) then the source directory must not be in the
	 * directory heirarchy above the target, as this would
	 * orphan everything below the source directory. Also
	 * the user must have write permission in the source so
	 * as to be able to change "..".
	 */
	if (fddep->de_StartCluster != tddep->de_StartCluster)
		newparent = 1;
	if (doingdirectory && newparent) {
		lck_mtx_lock(pmp->pm_rename_lock);
		error = msdosfs_doscheckpath(fdep, tddep, context);
		if (error) goto exit;
	}

	/* Remember the offset of fdep within fddep. */
	from_offset = fdep->de_diroffset;

	if (tvp != NULL && tdep != NULL) {
		uint32_t dest_offset;	/* Of the pre-existing entry being removed */
		
		/*
		 * Target must be empty if a directory and have no links
		 * to it. Also, ensure source and target are compatible
		 * (both directories, or both not directories).
		 */
		if (tdep->de_Attributes & ATTR_DIRECTORY) {
			if (!msdosfs_dosdirempty(tdep, context)) {
				error = ENOTEMPTY;
				goto exit;
			}
			if (!doingdirectory) {
				error = ENOTDIR;
				goto exit;
			}
		} else {
			if (doingdirectory) {
				error = EISDIR;
				goto exit;
			}
		}
		dest_offset = tdep->de_diroffset;
		cache_purge(tvp);		/* Purge tvp before we delete it on disk */
		tdep->de_refcnt--;
		if (DEBUG && tdep->de_refcnt < 0)
			panic("msdosfs_vnop_rename: de_refcnt went negative");
		error = msdosfs_removede(tddep, dest_offset, context);
		if (error)
		{
			if (DEBUG) panic("msdosfs_vnop_rename: msdosfs_removede (destination) returned %d\n", error);
			goto flush_exit;
		}
	}

	/*
	 * Figure out the new short name (toname).  If we're just changing
	 * case, then the short name stays the same.  Otherwise, we have
	 * to convert the long name to a unique short name.
	 */
    if (change_case)
	{
		bcopy(fdep->de_Name, toname, SHORT_NAME_LEN);
	}
	else
	{
        error = msdosfs_uniqdosname(tddep, toname, needs_generation ? to_diroffset - to_long_count * sizeof(struct dosdirentry) : 1, context);
        /*¥ What if we get an error and we already deleted the target? */
        if (error)
		{
			if (DEBUG) panic("msdosfs_vnop_rename: msdosfs_uniqdosname returned %d\n", error);
			goto flush_exit;
		}
    }

	/*
	 * First write a new entry in the destination
	 * directory and mark the entry in the source directory
	 * as deleted.  Then move the denode to the correct hash
	 * chain for its new location in the filesystem.  And, if
	 * we moved a directory, then update its .. entry to point
	 * to the new parent directory.  Set the name in the denode
	 * to the new short name.
	 *
	 * The name in the denode is updated for files.  For
	 * directories, the denode points at the "." entry in
	 * the directory, so temporarily change the name in
	 * the denode, and restore it; otherwise the "." entry
	 * may be overwritten.
	 */
	cache_purge(fvp);
	bcopy(fdep->de_Name, oldname, SHORT_NAME_LEN);
	bcopy(toname, fdep->de_Name, SHORT_NAME_LEN);	/* update denode */
	fdep->de_LowerCase = new_deLowerCase;
	
	/*
	 * If the new name starts with ".", make it invisible on Windows.
	 * Otherwise, make it visible.
	 */
	if (tcnp->cn_nameptr[0] == '.')
		fdep->de_Attributes |= ATTR_HIDDEN;
	else
		fdep->de_Attributes &= ~ATTR_HIDDEN;

	error = msdosfs_createde(fdep, tddep, NULL, tcnp, to_diroffset, to_long_count, context);
	if (error)
	{
		if (DEBUG) panic("msdosfs_vnop_rename: msdosfs_createde returned %d\n", error);
		bcopy(oldname, fdep->de_Name, SHORT_NAME_LEN);
		/*¥ What if we already deleted the target? */
		/* And shouldn't we also restore fdep->de_LowerCase? */
		goto flush_exit;
	}
	else
	{
		cache_purge_negatives(tdvp);
	}

	fdep->de_parent = tddep;
	
	error = msdosfs_removede(fddep, from_offset, context);
	if (error) {
		if (DEBUG) panic("msdosfs_vnop_rename: msdosfs_removede (source) returned %d\n", error);
		goto flush_exit;
	}

	/*
	 * Fix fdep's de_dirclust and de_diroffset to reflect
	 * its new location in the destination directory.
	 */
	error = msdosfs_pcbmap(tddep, de_cluster(pmp, to_diroffset), 1,
				NULL, &fdep->de_dirclust, NULL);
	if (error) {
		if (DEBUG) panic("msdosfs_vnop_rename: msdosfs_pcbmap returned %d\n", error);
		goto flush_exit;
	}
	fdep->de_diroffset = to_diroffset;

	/*
	 * fdep's identity (name and parent) have changed, so we must msdosfs_hash_reinsert
	 * it in our denode hash.
	 */
	msdosfs_hash_reinsert(fdep);

	/*
	 * If we moved a directory to a new parent directory, then we must
	 * fixup the ".." entry in the moved directory.
	 */
	if (doingdirectory && newparent) {
		struct dosdirentry *dotdotp;

		/* Read in the first cluster of the directory and point to ".." entry */
		cn = fdep->de_StartCluster;
		bn = cntobn(pmp, cn);
		error = (int)buf_meta_bread(pmp->pm_devvp, bn, pmp->pm_bpcluster, vfs_context_ucred(context), &bp);
		if (error) {
			if (DEBUG) panic("msdosfs_vnop_rename: buf_meta_bread returned %d\n", error);
			buf_brelse(bp);
			goto flush_exit;
		}
		dotdotp = (struct dosdirentry *)buf_dataptr(bp) + 1;

		/* Update the starting cluster of ".." */
		putuint16(dotdotp->deStartCluster, tddep->de_StartCluster);
		if (FAT32(pmp))
			putuint16(dotdotp->deHighClust, tddep->de_StartCluster >> 16);

		error = (int)buf_bdwrite(bp);
		if (error) {
			if (DEBUG) panic("msdosfs_vnop_rename: buf_bdwrite returned %d\n", error);
			goto flush_exit;
		}
	}

flush_exit:
	msdosfs_meta_flush(pmp, FALSE);
exit:

	if (doingdirectory && newparent)
		lck_mtx_unlock(pmp->pm_rename_lock);
	msdosfs_unlock_four(fddep, fdep, tddep, tdep);

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_RENAME|DBG_FUNC_END, error, 0, 0, 0, 0);

	return error;

}

static struct {
	struct dosdirentry dot;
	struct dosdirentry dotdot;
} dosdirtemplate = {
	{	".       ", "   ",			/* the . entry */
		ATTR_DIRECTORY,				/* file attribute */
		0,	 				/* reserved */
		0, { 0, 0 }, { 0, 0 },			/* create time & date */
		{ 0, 0 },				/* access date */
		{ 0, 0 },				/* high bits of start cluster */
		{ 210, 4 }, { 210, 4 },			/* modify time & date */
		{ 0, 0 },				/* startcluster */
		{ 0, 0, 0, 0 } 				/* filesize */
	},
	{	"..      ", "   ",			/* the .. entry */
		ATTR_DIRECTORY,				/* file attribute */
		0,	 				/* reserved */
		0, { 0, 0 }, { 0, 0 },			/* create time & date */
		{ 0, 0 },				/* access date */
		{ 0, 0 },				/* high bits of start cluster */
		{ 210, 4 }, { 210, 4 },			/* modify time & date */
		{ 0, 0 },				/* startcluster */
		{ 0, 0, 0, 0 }				/* filesize */
	}
};

int msdosfs_vnop_mkdir(struct vnop_mkdir_args *ap)
/* {
		vnode_t a_dvp;
		vnode_t *a_vpp;
		struct componentname *a_cnp;
		struct vattr *a_vap;
		vfs_context_t a_context;
	} */
{
	vnode_t dvp = ap->a_dvp;
	struct denode *pdep = VTODE(dvp);
	struct componentname *cnp = ap->a_cnp;
	vfs_context_t context = ap->a_context;
	struct denode *dep;
	struct dosdirentry *denp;
	struct msdosfsmount *pmp = pdep->de_pmp;
	struct buf *bp;
	uint32_t newcluster, pcl;
	daddr64_t bn;
	int error;
	struct denode ndirent;
	struct timespec ts;
	char *bdata;
	uint32_t offset;
	uint32_t long_count;
	int needs_generation;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_MKDIR|DBG_FUNC_START, pdep, 0, 0, 0, 0);
	
	lck_mtx_lock(pdep->de_lock);
	
	/*
	 * Make sure the parent directory hasn't been deleted.
	 */
	if (pdep->de_refcnt <= 0)
	{
		cache_purge(dvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the name does not exist in the parent directory.  (It didn't
	 * exist during VNOP_LOOKUP, but another thread may have created the name
	 * before we got the lock on the parent.)
	 */
	error = msdosfs_lookup_name(pdep, cnp, NULL, NULL, NULL, NULL, NULL, NULL, context);
	if (error != ENOENT)
	{
		error = EEXIST;
		goto exit;
	}
	
	/*
	 * Find space in the directory to place the new name.
	 */
	bzero(&ndirent, sizeof(ndirent));
	error = msdosfs_findslots(pdep, cnp, ndirent.de_Name, &needs_generation, &ndirent.de_LowerCase, &offset, &long_count, context);
	if (error)
		goto exit;

	/*
	 * If this is the root directory and there is no space left we
	 * can't do anything.  This is because the root directory can not
	 * change size.  (FAT12 and FAT16 only)
	 *
	 * On FAT32, we can grow the root directory, and de_StartCluster
	 * will be the actual cluster number of the root directory.
	 */
	if (pdep->de_StartCluster == MSDOSFSROOT && offset >= pdep->de_FileSize)
	{
        printf("msdosfs_vnop_mkdir: Cannot grow the root directory on FAT12 or FAT16; returning ENOSPC.\n");
		error = ENOSPC;
		goto exit;
	}

	/*
	 * Allocate a cluster to hold the about to be created directory.
	 */
	error = msdosfs_clusteralloc(pmp, 0, 1, CLUST_EOFE, &newcluster, NULL);
	if (error)
		goto exit;

	ndirent.de_pmp = pmp;
	ndirent.de_flag = DE_ACCESS | DE_CREATE | DE_UPDATE;
	getnanotime(&ts);
	DETIMES(&ndirent, &ts, &ts, &ts);

	/*
	 * Now fill the cluster with the "." and ".." entries. And write
	 * the cluster to disk.  This way it is there for the parent
	 * directory to be pointing at if there were a crash.
	 */
	bn = cntobn(pmp, newcluster);
	bp = buf_getblk(pmp->pm_devvp, bn, pmp->pm_bpcluster, 0, 0, BLK_META);
	bdata = (char *)buf_dataptr(bp);

	bzero(bdata, pmp->pm_bpcluster);
	bcopy(&dosdirtemplate, bdata, sizeof dosdirtemplate);
	denp = (struct dosdirentry *)bdata;
	putuint16(denp[0].deStartCluster, newcluster);
	putuint16(denp[0].deCDate, ndirent.de_CDate);
	putuint16(denp[0].deCTime, ndirent.de_CTime);
	denp[0].deCHundredth = ndirent.de_CHun;
	putuint16(denp[0].deADate, ndirent.de_ADate);
	putuint16(denp[0].deMDate, ndirent.de_MDate);
	putuint16(denp[0].deMTime, ndirent.de_MTime);
	pcl = pdep->de_StartCluster;
	if (FAT32(pmp) && pcl == pmp->pm_rootdirblk)
		pcl = 0;
	putuint16(denp[1].deStartCluster, pcl);
	putuint16(denp[1].deCDate, ndirent.de_CDate);
	putuint16(denp[1].deCTime, ndirent.de_CTime);
	denp[1].deCHundredth = ndirent.de_CHun;
	putuint16(denp[1].deADate, ndirent.de_ADate);
	putuint16(denp[1].deMDate, ndirent.de_MDate);
	putuint16(denp[1].deMTime, ndirent.de_MTime);
	if (FAT32(pmp)) {
		putuint16(denp[0].deHighClust, newcluster >> 16);
		putuint16(denp[1].deHighClust, pdep->de_StartCluster >> 16);
	}

	error = (int)buf_bdwrite(bp);
	if (error)
		goto exit;

	/*
	 * Now build up a directory entry pointing to the newly allocated
	 * cluster.  This will be written to an empty slot in the parent
	 * directory.
	 */
	error = msdosfs_uniqdosname(pdep, ndirent.de_Name, needs_generation ? offset - long_count * sizeof(struct dosdirentry) : 1, context);
	if (error)
		goto exit;

	ndirent.de_Attributes = ATTR_DIRECTORY;
	ndirent.de_StartCluster = newcluster;
	ndirent.de_FileSize = 0;
	ndirent.de_dev = pdep->de_dev;
	ndirent.de_devvp = pdep->de_devvp;

	/*
	 * If the file name starts with ".", make it invisible on Windows.
	 */
	if (cnp->cn_nameptr[0] == '.')
		ndirent.de_Attributes |= ATTR_HIDDEN;

	error = msdosfs_createde(&ndirent, pdep, &dep, cnp, offset, long_count, context);
	if (error)
	{
		if (DEBUG)
			panic("msodsfs_mkdir: msdosfs_createde failed\n");
		msdosfs_freeclusterchain(pmp, newcluster);
	}
	else
	{
		*ap->a_vpp = DETOV(dep);
		cache_purge_negatives(dvp);
	}

exit:
	msdosfs_meta_flush(pmp, FALSE);
	lck_mtx_unlock(pdep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_MKDIR|DBG_FUNC_END, error, 0, 0, 0, 0);
	return error;
}

/*
 * Remove a directory.
 *
 * On entry, vp has been suspended, so there are no pending
 * create or lookup operations happening using it as the
 * parent directory.
 *
 * VFS has already checked that dvp != vp.
 * 
 */
int msdosfs_vnop_rmdir(struct vnop_rmdir_args *ap)
/* {
		vnode_t a_dvp;
		vnode_t a_vp;
		struct componentname *a_cnp;
		vfs_context_t a_context;
	} */
{
	vnode_t vp = ap->a_vp;
	vnode_t dvp = ap->a_dvp;
	vfs_context_t context = ap->a_context;
	struct denode *ip, *dp;
	int error;
	uint32_t cluster, offset;
	
	ip = VTODE(vp);
	dp = VTODE(dvp);

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_RMDIR|DBG_FUNC_START, dp, ip, 0, 0, 0);
	msdosfs_lock_two(dp, ip);
	
	/*
	 * Make sure the parent directory hasn't been deleted.
	 */
	if (dp->de_refcnt <= 0)
	{
		cache_purge(dvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the child denode hasn't been deleted.
	 */
	if (ip->de_refcnt <= 0)
	{
		cache_purge(vp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the child still has the same name.
	 */
	error = msdosfs_lookup_name(dp, ap->a_cnp, &cluster, &offset, NULL, NULL, NULL, NULL, context);
	if (error || cluster != ip->de_dirclust || offset != ip->de_diroffset)
	{
		cache_purge(vp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * No rmdir "." please.
	 *
	 * VFS already checks this in rmdir(), so do we
	 * need to check again?  (It would only be useful if
	 * some other entity called our VNOP directly.)
	 */
	if (dp == ip) {
		error = EINVAL;
		goto exit;
	}

	/*
	 * Verify the directory is empty (and valid).
	 * (Rmdir ".." won't be valid since
	 *  ".." will contain a reference to
	 *  the current directory and thus be
	 *  non-empty.)
	 */
	if (!msdosfs_dosdirempty(ip, context)) {
		error = ENOTEMPTY;
		goto exit;
	}

	/*
	 * Delete the entry from the directory.  For dos filesystems this
	 * gets rid of the directory entry on disk.  The in memory copy
	 * still exists but the de_refcnt is <= 0.  This prevents it from
	 * being found by msdosfs_deget().  When the vput() on dep is done we give
	 * up access and eventually msdosfs_vnop_reclaim() will be called which
	 * will remove it from the denode cache.
	 */
	ip->de_refcnt--;
	if (DEBUG && ip->de_refcnt < 0)
		panic("msdosfs_vnop_rmdir: de_refcnt went negative");
	error = msdosfs_removede(dp, ip->de_diroffset, context);
	if (error)
	{
		if (DEBUG) panic("msdosfs_vnop_rmdir: msdosfs_removede returned %d\n", error);
		goto flush_exit;
	}
	
	/*
	 * Invalidate the directory's contents.  If directory I/O went through
	 * the directory's vnode, this wouldn't be needed; the invalidation
	 * done in msdosfs_detrunc would be sufficient.
	 */
	error = msdosfs_dir_invalidate(ip);
	if (error)
	{
		if (DEBUG) panic("msdosfs_vnop_rmdir: msdosfs_dir_invalidate returned %d\n", error);
		goto flush_exit;
	}

	/*
	 * Truncate the directory that is being deleted.
	 * msdosfs_detrunc internally updates dep->de_FileSize and calls ubc_setsize.
	 */
	error = msdosfs_detrunc(ip, 0, 0, context);
	if (DEBUG && error) panic("msdosfs_vnop_rmdir: msdosfs_detrunc returned %d\n", error);
	cache_purge(vp);

flush_exit:
	msdosfs_meta_flush(dp->de_pmp, FALSE);
exit:
	lck_mtx_unlock(dp->de_lock);
	lck_mtx_unlock(ip->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_RMDIR|DBG_FUNC_END, error, 0, 0, 0, 0);
	return error;
}


/*
 * Set the d_namlen and d_reclen fields in a directory buffer.
 *
 * The d_reclen field must be rounded up to a multiple of the length of the d_ino
 * field so that all entries will have their d_ino field on natural alignment.
 */
ssize_t msdosfs_dirbuf_size(union msdosfs_dirbuf *buf, size_t name_length, int flags)
{
	if (flags & VNODE_READDIR_EXTENDED)
	{
		buf->direntry.d_namlen = name_length;
		buf->direntry.d_reclen = (offsetof(struct direntry, d_name) + name_length + 8) & ~7;
		return buf->direntry.d_reclen;
	}
	else
	{
		buf->dirent.d_namlen = name_length;
		buf->dirent.d_reclen = (offsetof(struct dirent, d_name) + name_length + 4) & ~3;
		return buf->dirent.d_reclen;
	}
}

int msdosfs_vnop_readdir(struct vnop_readdir_args *ap)
/* {
		vnode_t a_vp;
		struct uio *a_uio;
		int a_flags;
		int *a_eofflag;
		int *a_numdirent;
		vfs_context_t a_context;
	} */
{
	int error = 0;
	vnode_t vp = ap->a_vp;
	struct uio *uio = ap->a_uio;
	struct denode *dep = VTODE(vp);
	struct msdosfsmount *pmp = dep->de_pmp;
	vfs_context_t context = ap->a_context;
	uint32_t blsize;
	int32_t entry_offset;
	uint32_t cn;
	uint32_t fileno;
	int32_t bias = 0;
	daddr64_t bn, dir_cluster;
	struct buf *bp;
	struct dosdirentry *dentp;
	off_t offset;			/* Current offset within directory */
	off_t long_name_offset;	/* Offset to start of long name */
	int chksum = -1;
	u_int16_t ucfn[WIN_MAXLEN];
	u_int16_t unichars = 0;
	size_t outbytes;
	char *bdata;
	union msdosfs_dirbuf buf;
	char *buf_name;
	size_t max_name;
	ssize_t buf_reclen;
	uint8_t buf_type;
	int eofflag = 0;
	int numdirent = 0;
	
	if (ap->a_numdirent)
		*ap->a_numdirent = 0;

	/* Assume we won't hit end of directory */
	if (ap->a_eofflag)
		*ap->a_eofflag = 0;

	if (ap->a_flags & VNODE_READDIR_REQSEEKOFF)
		return EINVAL;

	/* Make a pointer to the start of the name field in the buffer. */
	if (ap->a_flags & VNODE_READDIR_EXTENDED)
	{
		buf_name = &buf.direntry.d_name[0];
		max_name = sizeof(buf.direntry.d_name);
		buf.direntry.d_seekoff = 0;
	}
	else
	{
		buf_name = &buf.dirent.d_name[0];
		max_name = sizeof(buf.dirent.d_name);
	}
	
	/*
	 * msdosfs_vnop_readdir() won't operate properly on regular files since
	 * it does i/o only with the device vnode, and hence can
	 * retrieve the wrong block from the buffer cache for a plain file.
	 * So, fail attempts to readdir() on a plain file.
	 */
	if (!vnode_isdir(vp))
		return ENOTDIR;
	
	/*
	 * If the file offset is not a multiple of the size of a
	 * directory entry, then we fail the read.  The remaining
	 * space in the buffer will be checked before each uiomove.
	 */
	long_name_offset = offset = uio_offset(uio);
	if (offset & (sizeof(struct dosdirentry) - 1))
		return EINVAL;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_READDIR|DBG_FUNC_START, dep, uio_offset(uio), uio_resid(uio), ap->a_flags, 0);
	lck_mtx_lock(dep->de_lock);
	
	/*
	 * If they are reading from the root directory, then we simulate
	 * the . and .. entries since these don't exist in the root
	 * directory.  We also set the offset bias to make up for having to
	 * simulate these entries. By this I mean that at file offset 64 we
	 * read the first entry in the root directory that lives on disk.
	 */
	if (dep->de_StartCluster == MSDOSFSROOT
	    || (FAT32(pmp) && dep->de_StartCluster == pmp->pm_rootdirblk)) {
		bias = 2 * sizeof(struct dosdirentry);
		while (offset < bias) {
			if (ap->a_flags & VNODE_READDIR_EXTENDED) {
				buf.direntry.d_fileno = msdosfs_defileid(dep);
				buf.direntry.d_type = DT_DIR;
			} else {
				buf.dirent.d_fileno = msdosfs_defileid(dep);
				buf.dirent.d_type = DT_DIR;
			}
			if (offset == 0) {
				strlcpy(buf_name, ".", max_name);
				outbytes = 1;
			} else {
				strlcpy(buf_name, "..", max_name);
				outbytes = 2;
			}
			buf_reclen = msdosfs_dirbuf_size(&buf, outbytes, ap->a_flags);
			if (uio_resid(uio) < buf_reclen)
				goto out;
			error = uiomove((caddr_t) &buf, (int)buf_reclen, uio);
			if (error)
				goto out;
			++numdirent;
			offset += sizeof(struct dosdirentry);
		}
	}

	while (uio_resid(uio) > 0) {
		dir_cluster = de_cluster(pmp, offset - bias);
		entry_offset = (offset - bias) & pmp->pm_crbomask;
		if (dep->de_FileSize <= (offset - bias)) {
			eofflag = 1;	/* Hit end of directory */
			break;
		}
		error = msdosfs_pcbmap(dep, (uint32_t)dir_cluster, 1, &bn, &cn, &blsize);
		if (error)
			break;
		error = (int)buf_meta_bread(pmp->pm_devvp, bn, blsize, vfs_context_ucred(context), &bp);
		if (error) {
			buf_brelse(bp);
			goto exit;
		}
		bdata = (char *)buf_dataptr(bp);

		/*
		 * Correct the block size to be just the amount of data actually
		 * returned in the buffer.  That is, the amount we asked to read
		 * minus the "resid" (amount remaining to read).
		 *
		 * Normally, buf_resid() should return 0; that is, it should have
		 * read as much as we asked for.  But we've seen some badly behaved
		 * devices which either misreport the number of sectors (giving us
		 * a value that is too large), or are unable to access sectors past
		 * a given offset.  If we end up trying to read past that offset,
		 * the devices tend to return success but zero bytes of data,
		 * and buf_resid() will end up being the number of bytes we
		 * expected to get, but didn't actually get.
		 */
		blsize -= buf_resid(bp);
		
		/*
		 * Convert from dos directory entries to fs-independent
		 * directory entries.
		 */
		for (dentp = (struct dosdirentry *)(bdata + entry_offset);
		     (char *)dentp < bdata + blsize;
		     dentp++, offset += sizeof(struct dosdirentry)) {
			/*
			 * If this is an unused entry, we can stop.
			 */
			if (dentp->deName[0] == SLOT_EMPTY) {
				buf_brelse(bp);
				if (ap->a_eofflag)
					*ap->a_eofflag = 1;	/* Hit end of directory */
				goto out;
			}
			/*
			 * Skip deleted entries.
			 */
			if (dentp->deName[0] == SLOT_DELETED) {
				chksum = -1;
				continue;
			}

			/*
			 * Handle Win95 long directory entries
			 */
			if ((dentp->deAttributes & ATTR_WIN95_MASK) == ATTR_WIN95) {
				if (dentp->deName[0] & WIN_LAST)
					long_name_offset = offset;
				chksum = msdosfs_getunicodefn((struct winentry *)dentp,
						ucfn, &unichars, chksum);
				continue;
			}

			/*
			 * Skip volume labels
			 */
			if (dentp->deAttributes & ATTR_VOLUME) {
				chksum = -1;
				continue;
			}
			
			/*
			 * This computation of d_fileno must match
			 * the computation in msdosfs_defileid().
             */
            fileno = getuint16(dentp->deStartCluster);
            if (FAT32(pmp))
                fileno |= getuint16(dentp->deHighClust) << 16;
			if (dentp->deAttributes & ATTR_DIRECTORY) {
            	if (fileno == MSDOSFSROOT) {
                    /* if this is the root directory */
                    if (FAT32(pmp))
                        fileno = pmp->pm_rootdirblk;
                    else
                        fileno = FILENO_ROOT;
                }
				buf_type = DT_DIR;
			} else {
                if (fileno == 0)
                    fileno = FILENO_EMPTY;	/* constant for empty files */
				if (getuint32(dentp->deFileSize) == sizeof(struct symlink))
					buf_type = DT_UNKNOWN;	/* Might be a symlink */
				else
					buf_type = DT_REG;
			}
			if (chksum != msdosfs_winChksum(dentp->deName)) {
				chksum = -1;
				unichars = msdosfs_dos2unicodefn(dentp->deName, ucfn,
				    dentp->deLowerCase);
			}
			
			/* translate the name in ucfn into UTF-8 */
			(void) utf8_encodestr(ucfn, unichars * 2, (u_int8_t*)buf_name,
						&outbytes, max_name, 0,
						UTF_DECOMPOSED|UTF_SFM_CONVERSIONS);
			
			/* Fill in the other buffer fields. */
			if (ap->a_flags & VNODE_READDIR_EXTENDED)
			{
				buf.direntry.d_ino = fileno;
				buf.direntry.d_type = buf_type;
			}
			else
			{
				buf.dirent.d_ino = fileno;
				buf.dirent.d_type = buf_type;
			}
			buf_reclen = msdosfs_dirbuf_size(&buf, outbytes, ap->a_flags);

			if (uio_resid(uio) < buf_reclen) {
				buf_brelse(bp);
				goto out;
			}
			error = uiomove((caddr_t) &buf, (int)buf_reclen, uio);
			if (error) {
				buf_brelse(bp);
				goto out;
			}
			chksum = -1;
			++numdirent;
		}
		buf_brelse(bp);
	}

out:
	/* Update the current position within the directory */
	if (chksum != -1)
		offset = long_name_offset;
	uio_setoffset(uio, offset);
exit:
	lck_mtx_unlock(dep->de_lock);
	if (ap->a_eofflag)
		*ap->a_eofflag = eofflag;
	if (ap->a_numdirent)
		*ap->a_numdirent = numdirent;
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_READDIR|DBG_FUNC_END, error, uio_offset(uio), numdirent, eofflag, 0);
	return error;
}

/* blktooff converts a logical block number to a file offset */
int msdosfs_vnop_blktooff(struct vnop_blktooff_args *ap)
/* {
		vnode_t a_vp;
		daddr64_t a_lblkno;
		off_t *a_offset;    
	} */
{
	if (ap->a_vp == NULL)
		return EINVAL;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_BLKTOOFF|DBG_FUNC_START, ap->a_lblkno, 0, 0, 0, 0);
	*ap->a_offset = ap->a_lblkno * PAGE_SIZE_64;
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_BLKTOOFF|DBG_FUNC_END, 0, *ap->a_offset, 0, 0, 0);

	return 0;
}

/* offtoblk converts a file offset to a logical block number */
int msdosfs_vnop_offtoblk(struct vnop_offtoblk_args *ap)
/* {
		vnode_t a_vp;
		off_t a_offset;    
		daddr64_t *a_lblkno;
	} */
{
	if (ap->a_vp == NULL)
		return EINVAL;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_OFFTOBLK|DBG_FUNC_START, ap->a_offset, 0, 0, 0, 0);
	*ap->a_lblkno = ap->a_offset / PAGE_SIZE_64;
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_OFFTOBLK|DBG_FUNC_END, 0, *ap->a_lblkno, 0, 0, 0);
	
	return 0;
}


/*
 * Map a logical file range (offset and length) to an on-disk extent
 * (block number and number of contiguous blocks).
 *
 * This routine assumes that the denode's de_lock has already been acquired
 * (such as inside of msdosfs_vnop_read or msdosfs_vnop_write, or by the caller of
 * buf_meta_bread).
 *
 * [It wouldn't make sense to call this routine if the file's size or
 * location on disk could be changing.  If it could, then any output
 * from this routine could be obsolete before the caller could use it.]
 */
int msdosfs_vnop_blockmap(struct vnop_blockmap_args *ap)
/* {
		vnode_t a_vp;
		off_t a_foffset;
		size_t a_size;
		daddr64_t *a_bpn;
		size_t *a_run;
		void *a_poff;
		int a_flags;
		vfs_context_t a_context;
	} */
{
	int error;
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);
    struct msdosfsmount *pmp = dep->de_pmp;
	uint32_t runsize;
    uint32_t		cn;
    uint32_t		numclusters;
    daddr64_t	bn;

	if (ap->a_bpn == NULL)
		return 0;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_BLOCKMAP|DBG_FUNC_START, dep, ap->a_foffset, ap->a_size, 0, 0);
    if (ap->a_size == 0)
        panic("msdosfs_vnop_blockmap: a_size == 0");

    /* Find the cluster that contains the given file offset */
    cn = de_cluster(pmp, ap->a_foffset);
    
    /* Determine number of clusters occupied by the given range */
    numclusters = de_cluster(pmp, ap->a_foffset + ap->a_size - 1) - cn + 1;
	 
    /* Find the physical (device) block where that cluster starts */
    error = msdosfs_pcbmap(dep, cn, numclusters, &bn, NULL, &runsize);

    /* Add the offset in physical (device) blocks from the start of the cluster */
    bn += (((uint32_t)ap->a_foffset - de_cn2off(pmp, cn)) >> pmp->pm_bnshift);
    runsize -= ((uint32_t)ap->a_foffset - (de_cn2off(pmp, cn)));
    
    *ap->a_bpn = bn;
	if (error == 0 && ap->a_run) {
		if (runsize > ap->a_size)
			* ap->a_run = ap->a_size;
		else
			* ap->a_run = runsize;
	}
	if (ap->a_poff)
		*(int *)ap->a_poff = 0;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_BLOCKMAP|DBG_FUNC_END, error, bn, runsize, 0, 0);
	return error;
}

/*
 * prepare and issue the I/O
 * buf_strategy knows how to deal
 * with requests that require 
 * fragmented I/Os
 */
int msdosfs_vnop_strategy(struct vnop_strategy_args *ap)
/* {
		struct buf *a_bp;
	} */
{
	buf_t	bp = ap->a_bp;
	vnode_t	vp = buf_vnode(bp);
	struct denode *dep = VTODE(vp);
	int error;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_STRATEGY|DBG_FUNC_START, dep, buf_lblkno(bp), buf_resid(bp), buf_flags(bp), 0);
	error = buf_strategy(dep->de_devvp, ap);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_STRATEGY|DBG_FUNC_END, error, buf_error(bp), buf_resid(bp), buf_flags(bp), 0);
	return error;
}

int msdosfs_vnop_pathconf(struct vnop_pathconf_args *ap)
/* {
		vnode_t a_vp;
		int a_name;
		register_t *a_retval;
		vfs_context_t a_context;
	} */
{
	int error = 0;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_PATHCONF|DBG_FUNC_START, ap->a_name, 0, 0, 0, 0);
	switch (ap->a_name) {
	case _PC_LINK_MAX:
		*ap->a_retval = 1;
		break;
	case _PC_NAME_MAX:
		*ap->a_retval = WIN_MAXLEN;
		break;
	case _PC_PATH_MAX:
		*ap->a_retval = PATH_MAX;
		break;
	case _PC_CHOWN_RESTRICTED:
		*ap->a_retval = 1;
		break;
	case _PC_NO_TRUNC:
		*ap->a_retval = 0;
		break;
	case _PC_CASE_SENSITIVE:
		*ap->a_retval = 0;
		break;
	case _PC_CASE_PRESERVING:
		*ap->a_retval = 1;
		break;
	case _PC_FILESIZEBITS:
		*ap->a_retval = 33;
		break;
	default:
		error = EINVAL;
		break;
	}
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_PATHCONF|DBG_FUNC_END, error, *ap->a_retval, 0, 0, 0);
	return error;
}


/*
 * Given a chunk of memory, compute the md5 digest as a string of 32
 * hex digits followed by a NUL character.
 */
void msdosfs_md5_digest(void *text, size_t length, char digest[33])
{
	int i;
	MD5_CTX context;
	unsigned char digest_raw[16];

	MD5Init(&context);
	MD5Update(&context, text, (unsigned)length);
	MD5Final(digest_raw, &context);
	
	for (i=0; i<16; ++i)
	{
		/*
		 * The "3" below is for the two hex digits plus trailing '\0'.
		 * Note that the trailing '\0' from byte N is overwritten
		 * by the first character of byte N+1, and that digest[] has
		 * a length of 33 == 2 * 16 + 1 in order to have room for a
		 * trailing '\0' after the last byte's characters.
		 */
		(void) snprintf(digest, 3, "%02x", digest_raw[i]);
		digest += 2;
	}
}


/*
 * Determine whether the given denode refers to a symlink or an ordinary
 * file.  This is called during vnode creation, so the de_vnode field
 * has not been set up.  Returns the vnode type to create (either
 * VREG or VLNK).
 *
 * In order to tell whether the file is an ordinary file or a symlink,
 * we need to read from it.  The easiest way to do this is to create a
 * temporary vnode of type VREG, so we can use the buffer cache.  We will
 * terminate the vnode before returning.  msdosfs_deget() will create the real
 * vnode to be returned.
 *
 * Assumes that the denode's de_lock is already acquired, or that the denode is
 * otherwise protected (not part of the denode hash, not in the name cache, not
 * in the volume's name space).
 */
enum vtype msdosfs_check_link(struct denode *dep, vfs_context_t context)
{
	int error;
	int i;
	unsigned length;
	char c;
	enum vtype result;
	struct msdosfsmount *pmp;
	vnode_t vp = NULL;
    buf_t bp = NULL;
	struct symlink *link;
	char digest[33];
	struct vnode_fsparam vfsp;

	if (dep->de_FileSize != sizeof(struct symlink))
	{
		result = VREG;
		goto exit;
	}

	/*
	 * The file is the magic symlink size.  We need to read it in so we
	 * can validate the header and update the size to reflect the
	 * length of the symlink.
	 *
	 * We create a temporary vnode to read in the contents of the file
	 * (since it may be fragmented, and this lets us take advantage of the
	 * blockmap and strategy routines).
	 *
	 * The temporary vnode's type is set to VNON instead of VREG so that
	 * vnode_iterate won't return it.  This prevents a race with
	 * msdosfs_vfs_sync that might try to fsync this vnode as its v_data pointer
	 * gets cleared by vnode_clearfsnode below.  (The race can lead to a
	 * NULL denode pointer in msdosfs_deupdat(), which causes a panic.)
	 */
	result = VREG;		/* Assume it's not a symlink, until we verify it. */
	pmp = dep->de_pmp;
	
	vfsp.vnfs_mp = pmp->pm_mountp;
	vfsp.vnfs_vtype = VNON;
	vfsp.vnfs_str = "msdosfs";
	vfsp.vnfs_dvp = NULL;
	vfsp.vnfs_fsnode = dep;
	vfsp.vnfs_cnp = NULL;
	vfsp.vnfs_vops = msdosfs_vnodeop_p;
	vfsp.vnfs_rdev = 0;
	vfsp.vnfs_filesize = dep->de_FileSize;
	vfsp.vnfs_flags = VNFS_NOCACHE;
	vfsp.vnfs_markroot = 0;
	vfsp.vnfs_marksystem = 0;
	
	error = vnode_create(VNCREATE_FLAVOR, VCREATESIZE, &vfsp, &dep->de_vnode);
	vp = dep->de_vnode;
	if (error) goto exit;

    error = buf_meta_bread(vp, 0, roundup(sizeof(*link),pmp->pm_bpcluster),
        vfs_context_ucred(context), &bp);
    if (error) goto exit;
    link = (struct symlink *) buf_dataptr(bp);

	/* Verify the magic */
	if (strncmp(link->magic, symlink_magic, 5) != 0)
		goto exit;

	/* Parse and sanity check the length field */
	length = 0;
	for (i=0; i<4; ++i)
	{
		c = link->length[i];
		if (c < '0' || c > '9')
			goto exit;		/* Length is non-decimal */
		length = 10 * length + c - '0';
	}
	if (length > SYMLINK_LINK_MAX)
		goto exit;			/* Length is too big */

	/* Verify the MD5 digest */
	msdosfs_md5_digest(link->link, length, digest);
	if (strncmp(digest, link->md5, 32) != 0)
		goto exit;

	/* It passed all the checks; must be a symlink */
	result = VLNK;
	dep->de_FileSize = length;
	dep->de_flag |= DE_SYMLINK;

exit:
    if (bp)
    {
        /*
         * We're going to be getting rid of the vnode, so we might as well
         * mark the buffer invalid so that it will get reused right away.
         */
        buf_markinvalid(bp);
        buf_brelse(bp);
    }
    
	if (vp)
	{
		(void) vnode_clearfsnode(vp);	/* So we won't free dep */
		(void) vnode_recycle(vp);		/* get rid of the vnode now */
		(void) vnode_put(vp);			/* to balance vnode_create */
	}

	return result;
}


/*
 * Create a symbolic link.
 *
 * The idea is to write the symlink file content to disk and
 * create a new directory entry pointing at the symlink content.
 * Then msdosfs_createde will automatically create the correct type of
 * vnode.
 */
int msdosfs_vnop_symlink(struct vnop_symlink_args *ap)
/* {
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	struct vnode_attr *a_vap;
	char *a_target;
	vfs_context_t a_context;
	} */
{
	int error;
	vnode_t dvp = ap->a_dvp;
	struct denode *dep = VTODE(dvp);
    struct msdosfsmount *pmp = dep->de_pmp;
    struct componentname *cnp = ap->a_cnp;
	struct vnode_attr *vap = ap->a_vap;
	char *target = ap->a_target;
	vfs_context_t context = ap->a_context;
	size_t length;		/* length of target path */
	struct symlink *link = NULL;
	uint32_t cn = 0;			/* first cluster of symlink */
	uint32_t clusters, got;	/* count of clusters needed, actually allocated */
	buf_t bp = NULL;
	struct denode ndirent;
	struct denode *new_dep;
	struct timespec ts;
	uint32_t offset;
	uint32_t long_count;
	int needs_generation;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_SYMLINK|DBG_FUNC_START, dep, 0, 0, 0, 0);
	lck_mtx_lock(dep->de_lock);
	
	/*
	 * Make sure the parent directory hasn't been deleted.
	 */
	if (dep->de_refcnt <= 0)
	{
		cache_purge(dvp);
		error = ENOENT;
		goto exit;
	}
	
	/*
	 * Make sure the name does not exist in the parent directory.  (It didn't
	 * exist during VNOP_LOOKUP, but another thread may have created the name
	 * before we got the lock on the parent.)
	 */
	error = msdosfs_lookup_name(dep, cnp, NULL, NULL, NULL, NULL, NULL, NULL, context);
	if (error != ENOENT)
	{
		error = EEXIST;
		goto exit;
	}
	
	/*
	 * Find space in the directory to place the new name.
	 */
	bzero(&ndirent, sizeof(ndirent));
	error = msdosfs_findslots(dep, cnp, ndirent.de_Name, &needs_generation, &ndirent.de_LowerCase, &offset, &long_count, context);
	if (error)
		goto exit;

	/*
	 * If this is the root directory and there is no space left we
	 * can't do anything.  This is because the root directory can not
	 * change size.  (FAT12 and FAT16 only)
	 *
	 * On FAT32, we can grow the root directory, and de_StartCluster
	 * will be the actual cluster number of the root directory.
	 */
	if (dep->de_StartCluster == MSDOSFSROOT && offset >= dep->de_FileSize)
	{
        printf("msdosfs_vnop_symlink: Cannot grow the root directory on FAT12 or FAT16; returning ENOSPC.\n");
		error = ENOSPC;
		goto exit;
	}

	length = strlen(target);
	if (length > SYMLINK_LINK_MAX)
	{
		error = ENAMETOOLONG;
		goto exit;
	}

	/*
	 * Allocate (contiguous) space to store the symlink.  In an ideal world,
	 * we should support creating a non-contiguous symlink file, but that
	 * would be much more complicated (creating a temporary denode and vnode
	 * just so that we can allocate and write to the link, then removing that
	 * vnode and creating a real one with the correct vtype).
	 */
	clusters = de_clcount(pmp, sizeof(*link));
	error = msdosfs_clusteralloc(pmp, 0, clusters, CLUST_EOFE, &cn, &got);
	if (error) goto exit;
	if (got < clusters)
	{
		error = ENOSPC;
		goto exit;
	}

	/* Get a buffer to hold the symlink */
	bp = buf_getblk(pmp->pm_devvp, cntobn(pmp, cn),
		roundup(sizeof(*link),pmp->pm_bpcluster),
		0, 0, BLK_META);
	buf_clear(bp);
	link = (struct symlink *) buf_dataptr(bp);

	/*
	 * Fill in each of the symlink fields.  We have to do this in the same
	 * order as the fields appear in the structure because some of the
	 * operations clobber one byte past the end of their field (with a
	 * NUL character that is a string terminator).
	 */
	bcopy(symlink_magic, link->magic, sizeof(symlink_magic));
	/* 6 = 4 bytes of digits + newline + '\0' */
	snprintf(link->length, 6, "%04u\n", (unsigned)length);
	msdosfs_md5_digest(target, length, link->md5);
	link->newline2 = '\n';
	bcopy(target, link->link, length);

	/* Pad with newline if there is room */
	if (length < SYMLINK_LINK_MAX)
		link->link[length++] = '\n';

	/* Pad with spaces if there is room */
	if (length < SYMLINK_LINK_MAX)
		memset(&link->link[length], ' ', SYMLINK_LINK_MAX-length);

	/* Write out the symlink */
	error = buf_bwrite(bp);
	bp = NULL;
	buf_invalblkno(pmp->pm_devvp, cntobn(pmp, cn), BUF_WAIT);
	if (error)
	{
		if (DEBUG) panic("msdosfs_vnop_symlink: buf_bwrite returned %d\n", error);
		goto exit;
	}

	/* Start setting up new directory entry */
	error = msdosfs_uniqdosname(dep, ndirent.de_Name, needs_generation ? offset - long_count * sizeof(struct dosdirentry) : 1, context);
	if (error)
	{
		if (DEBUG) panic("msdosfs_vnop_symlink: msdosfs_uniqdosname returned %d\n", error);
		goto exit;
	}
	
	/*
	 * Set read-only attribute if one of the immutable bits is set.
	 * Always set the "needs archive" attribute on newly created files.
	 */
	ndirent.de_Attributes = ATTR_ARCHIVE;
	if (VATTR_IS_ACTIVE(vap, va_flags))
	{
		if ((vap->va_flags & (SF_IMMUTABLE | UF_IMMUTABLE)) != 0)
			ndirent.de_Attributes |= ATTR_READONLY;
		VATTR_SET_SUPPORTED(vap, va_flags);
	}
        
	ndirent.de_StartCluster = cn;
	ndirent.de_FileSize = sizeof(*link);
	ndirent.de_dev = dep->de_dev;
	ndirent.de_devvp = dep->de_devvp;
	ndirent.de_pmp = dep->de_pmp;
	ndirent.de_flag = DE_ACCESS | DE_CREATE | DE_UPDATE;
	getnanotime(&ts);
	DETIMES(&ndirent, &ts, &ts, &ts);
	
	/* Create a new directory entry pointing at the newly allocated clusters */
	error = msdosfs_createde(&ndirent, dep, &new_dep, cnp, offset, long_count, context);
	if (error)
	{
		if (DEBUG) panic("msdosfs_vnop_symlink: msdosfs_createde returned %d\n", error);
		goto exit;
	}
	*ap->a_vpp = DETOV(new_dep);
	cache_purge_negatives(dvp);

exit:
	if (bp)
	{
		/*
		 * After the symlink is created, we should access the contents via the
		 * symlink's vnode, not via the device.  Marking it invalid here
		 * prevents double caching (and the inconsistencies which can result).
		 */
		buf_markinvalid(bp);
		buf_brelse(bp);
	}
	if (error != 0 && cn != 0)
		(void) msdosfs_freeclusterchain(pmp, cn);

	msdosfs_meta_flush(pmp, FALSE);
	lck_mtx_unlock(dep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_SYMLINK|DBG_FUNC_END, error, 0, 0, 0, 0);
	return error;
}


int msdosfs_vnop_readlink(struct vnop_readlink_args *ap)
/* {
	vnode_t a_vp;
	struct uio *a_uio;
	vfs_context_t a_context;
	} */
{
	int error;
	vnode_t vp = ap->a_vp;
	struct denode *dep = VTODE(vp);
    struct msdosfsmount *pmp = dep->de_pmp;
    buf_t bp = NULL;
	struct symlink *link;
	
	if (vnode_vtype(vp) != VLNK)
		return EINVAL;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_READLINK|DBG_FUNC_START, dep, 0, 0, 0, 0);
	lck_mtx_lock(dep->de_lock);
	
	if (dep->de_refcnt <= 0)
	{
		cache_purge(vp);
		error = EAGAIN;
		goto exit;
	}
	
	if (dep->de_StartCluster == 0)
		panic("msdosfs_vnop_readlink: de_StartCluster == 0!\n");
	
	/*
	 * If the vnode was created of type VLNK, then we assume the symlink
	 * file has been checked for consistency.  So we skip it here.
	 */
    error = buf_meta_bread(vp, 0, roundup(sizeof(*link),pmp->pm_bpcluster),
        vfs_context_ucred(ap->a_context), &bp);
    if (error) goto exit;
    link = (struct symlink *) buf_dataptr(bp);

	/*
	 * Return the link.  Assumes de_FileSize was set to the length
	 * of the symlink.
	 */
	error = uiomove(link->link, dep->de_FileSize, ap->a_uio);

exit:
    if (bp)
        buf_brelse(bp);

	lck_mtx_unlock(dep->de_lock);
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_READLINK|DBG_FUNC_END, error, 0, 0, 0, 0);
	
	return error;
}


int msdosfs_vnop_ioctl(struct vnop_ioctl_args *ap)
/* {
	vnode_t a_vp;
	uint32_t a_command;
	caddr_t a_data;
	int a_fflag;
	vfs_context_t a_context;
	} */
{
	int error;
	vnode_t vp = ap->a_vp;
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_IOCTL|DBG_FUNC_START, VTODE(vp), ap->a_command, 0, 0, 0);
	switch(ap->a_command) {
		case F_FULLFSYNC: 
		{
			struct vnop_fsync_args fsync_args;

			bzero(&fsync_args, sizeof(fsync_args));
			fsync_args.a_vp = ap->a_vp;
			fsync_args.a_waitfor = MNT_WAIT;
			fsync_args.a_context = ap->a_context;

			error = msdosfs_vnop_fsync(&fsync_args);
			if (error) {
				goto exit;
			}

			/* Call device ioctl to flush media track cache */
			error = VNOP_IOCTL(VTODE(vp)->de_pmp->pm_devvp, DKIOCSYNCHRONIZECACHE,
							   NULL, FWRITE, ap->a_context); 
			if (error) {
				goto exit;
			}

			break;
		} /* F_FULLFSYNC */

		default: 
		{
			error = ENOTTY;
			goto exit;
		}
	}/* switch(ap->a_command) */
	
exit:
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VNOP_IOCTL|DBG_FUNC_START, error, 0, 0, 0, 0);
	return error;
}



/* Global vfs data structures for msdosfs */

typedef int     vnop_t(void *);

int (**msdosfs_vnodeop_p)(void *);
static struct vnodeopv_entry_desc msdosfs_vnodeop_entries[] = {
	{ &vnop_default_desc,		(vnop_t *) vn_default_error },
	{ &vnop_lookup_desc,		(vnop_t *) msdosfs_vnop_lookup },
	{ &vnop_create_desc,		(vnop_t *) msdosfs_vnop_create },
	{ &vnop_mknod_desc,			(vnop_t *) msdosfs_vnop_mknod },
	{ &vnop_open_desc,			(vnop_t *) msdosfs_vnop_open },
	{ &vnop_close_desc,			(vnop_t *) msdosfs_vnop_close },
	{ &vnop_getattr_desc,		(vnop_t *) msdosfs_vnop_getattr },
	{ &vnop_setattr_desc,		(vnop_t *) msdosfs_vnop_setattr },
	{ &vnop_getxattr_desc,		(vnop_t *) msdosfs_vnop_getxattr },
	{ &vnop_setxattr_desc,		(vnop_t *) msdosfs_vnop_setxattr },
	{ &vnop_removexattr_desc,	(vnop_t *) msdosfs_vnop_removexattr },
	{ &vnop_listxattr_desc,		(vnop_t *) msdosfs_vnop_listxattr },
	{ &vnop_read_desc,			(vnop_t *) msdosfs_vnop_read },
	{ &vnop_write_desc,			(vnop_t *) msdosfs_vnop_write },
	{ &vnop_fsync_desc,			(vnop_t *) msdosfs_vnop_fsync },
	{ &vnop_remove_desc,		(vnop_t *) msdosfs_vnop_remove },
	{ &vnop_rename_desc,		(vnop_t *) msdosfs_vnop_rename },
	{ &vnop_mkdir_desc,			(vnop_t *) msdosfs_vnop_mkdir },
	{ &vnop_rmdir_desc,			(vnop_t *) msdosfs_vnop_rmdir },
	{ &vnop_readdir_desc,		(vnop_t *) msdosfs_vnop_readdir },
	{ &vnop_inactive_desc,		(vnop_t *) msdosfs_vnop_inactive },
	{ &vnop_reclaim_desc,		(vnop_t *) msdosfs_vnop_reclaim },
	{ &vnop_pathconf_desc,		(vnop_t *) msdosfs_vnop_pathconf },
	{ &vnop_pagein_desc,		(vnop_t *) msdosfs_vnop_pagein },
	{ &vnop_pageout_desc,		(vnop_t *) msdosfs_vnop_pageout },
	{ &vnop_blktooff_desc,		(vnop_t *) msdosfs_vnop_blktooff },
	{ &vnop_offtoblk_desc,		(vnop_t *) msdosfs_vnop_offtoblk },
  	{ &vnop_blockmap_desc,		(vnop_t *) msdosfs_vnop_blockmap },
	{ &vnop_strategy_desc,		(vnop_t *) msdosfs_vnop_strategy },
	{ &vnop_symlink_desc,		(vnop_t *) msdosfs_vnop_symlink },
	{ &vnop_readlink_desc,		(vnop_t *) msdosfs_vnop_readlink },
	{ &vnop_ioctl_desc, 		(vnop_t *) msdosfs_vnop_ioctl},
	{ &vnop_bwrite_desc,		(vnop_t *) vn_bwrite },
	{ NULL, NULL }
};

extern int (**msdosfs_fat_vnodeop_p)(void *);
extern struct vnodeopv_entry_desc msdosfs_fat_vnodeop_entries[];

struct vnodeopv_desc msdosfs_vnodeop_opv_desc =
	{ &msdosfs_vnodeop_p, msdosfs_vnodeop_entries };
