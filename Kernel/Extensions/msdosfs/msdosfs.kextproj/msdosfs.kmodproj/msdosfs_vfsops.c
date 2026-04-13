/*
 * Copyright (c) 2000-2011 Apple Inc. All rights reserved.
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
/* $FreeBSD: src/sys/msdosfs/msdosfs_vfsops.c,v 1.63 2000/05/05 09:58:36 phk Exp $ */
/*	$NetBSD: msdosfs_vfsops.c,v 1.51 1997/11/17 15:36:58 ws Exp $	*/

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
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/fcntl.h>
#include <sys/malloc.h>
#include <sys/stat.h> 				/* defines ALLPERMS */
#include <sys/ubc.h>
#include <sys/utfconv.h>
#include <sys/disk.h>
#include <sys/sysctl.h>
#include <mach/kmod.h>
#include <libkern/OSBase.h>
#include <libkern/OSAtomic.h>
#include <kern/clock.h>
#include <kern/thread.h>
#include <kern/thread_call.h>
#include <miscfs/specfs/specdev.h>
#include <IOKit/IOTypes.h>
#include <libkern/OSMalloc.h>
#include <libkern/OSKextLib.h>
#include <libkern/crypto/md5.h>
#include <TargetConditionals.h>

#include "bpb.h"
#include "bootsect.h"
#include "direntry.h"
#include "denode.h"
#include "msdosfsmount.h"
#include "fat.h"
#include "msdosfs_kdebug.h"

/*
 * By default, don't try to auto unload the KEXT on embedded systems, since
 * that isn't currently supported.  You can always explicitly set
 * MSDOSFS_AUTO_UNLOAD to 0 or 1 to override the default.  Note that 
 * the simulator doesn't run kexts; it will use the host filesystem 
 * functionality. So even if we disable kext auto-unload for the simulator
 * target it doesn't do much.
 */
#ifndef MSDOSFS_AUTO_UNLOAD
#if !TARGET_OS_OSX
#define MSDOSFS_AUTO_UNLOAD		0
#else
#define MSDOSFS_AUTO_UNLOAD		1
#endif
#endif

#define MSDOSFS_DFLTBSIZE       4096

#define rounddown(x,y)	(((x)/(y))*(y))

extern u_int16_t dos2unicode[32];

extern int32_t msdos_secondsWest;	/* In msdosfs_conv.c */

__private_extern__ lck_grp_attr_t *msdosfs_lck_grp_attr = NULL;
__private_extern__ lck_grp_t *msdosfs_lck_grp = NULL;
__private_extern__ lck_attr_t *msdosfs_lck_attr = NULL;
__private_extern__ OSMallocTag msdosfs_malloc_tag = NULL;

#if DEBUG
SYSCTL_DECL(_vfs_generic);
SYSCTL_NODE(_vfs_generic, OID_AUTO, msdosfs, CTLFLAG_RW, 0, "msdosfs (FAT) file system");
SYSCTL_INT(_vfs_generic_msdosfs, OID_AUTO, meta_delay, CTLFLAG_RW, &msdosfs_meta_delay, 0, "max delay before flushing metadata (ms)");
#endif

int msdosfs_init(struct vfsconf *vfsp);
int msdosfs_uninit(void);
int msdosfs_start(struct mount *mp, int flags, vfs_context_t context);

int msdosfs_update_mp(struct mount *mp, struct msdosfs_args *argp);
int msdosfs_mount(vnode_t devvp, struct mount *mp, vfs_context_t context);
int msdosfs_vfs_mount(struct mount *mp, vnode_t devvp, user_addr_t data, vfs_context_t);
int msdosfs_vfs_root(struct mount *, vnode_t *, vfs_context_t);
int msdosfs_vfs_statfs(struct mount *, struct vfsstatfs *, vfs_context_t);
int msdosfs_vfs_getattr(mount_t mp, struct vfs_attr *attr, vfs_context_t context);
int msdosfs_vfs_setattr(mount_t mp, struct vfs_attr *attr, vfs_context_t context);
int msdosfs_vfs_sync(struct mount *, int, vfs_context_t);
int msdosfs_vfs_unmount(struct mount *, int, vfs_context_t);

int msdosfs_scan_root_dir(struct mount *mp, vfs_context_t context);
int msdosfs_sync_callback(vnode_t vp, void *cargs);

/* The routines are exported for the KEXT glue to link against. */
int msdosfs_module_start(kmod_info_t *ki, void *data);
int msdosfs_module_stop (kmod_info_t *ki, void *data);

/*ARGSUSED*/
int msdosfs_init(struct vfsconf *vfsp)
{
#pragma unused (vfsp)
	msdosfs_lck_grp_attr = lck_grp_attr_alloc_init();
	msdosfs_lck_grp = lck_grp_alloc_init("msdosfs", msdosfs_lck_grp_attr);
	msdosfs_lck_attr = lck_attr_alloc_init();
	
	msdosfs_malloc_tag = OSMalloc_Tagalloc("msdosfs", OSMT_DEFAULT);
	
	msdosfs_hash_init();
	return 0;
}


/*
 * There is no "un-init" VFS operation.  This routine is only called by
 * the KEXT as it is about to be unloaded.
 */

int msdosfs_uninit(void)
{
	msdosfs_hash_uninit();
	
	OSMalloc_Tagfree(msdosfs_malloc_tag);
	
	lck_attr_free(msdosfs_lck_attr);
	lck_grp_free(msdosfs_lck_grp);
	lck_grp_attr_free(msdosfs_lck_grp_attr);
	
	return 0;
}


int msdosfs_update_mp(struct mount *mp, struct msdosfs_args *argp)
{
	struct msdosfsmount *pmp = VFSTOMSDOSFS(mp);

	pmp->pm_gid = argp->gid;
	pmp->pm_uid = argp->uid;
	pmp->pm_mask = argp->mask & ALLPERMS;
	pmp->pm_flags |= argp->flags & MSDOSFSMNT_MNTOPT;
	if (argp->flags & MSDOSFSMNT_SECONDSWEST)
		msdos_secondsWest = argp->secondsWest;

	if (argp->flags & MSDOSFSMNT_LABEL)
		bcopy(argp->label, pmp->pm_label, sizeof(pmp->pm_label));

	return 0;
}


/*
 * mp - path - addr in user space of mount point (ie /usr or whatever)
 * data - addr in user space of mount params including the name of the block
 * special file to treat as a filesystem.
 */
int msdosfs_vfs_mount(struct mount *mp, vnode_t devvp, user_addr_t data, vfs_context_t context)
{
	struct msdosfs_args args; /* will hold data from mount request */
	/* msdosfs specific mount control block */
	struct msdosfsmount *pmp = NULL;
	int error, flags;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_MOUNT|DBG_FUNC_START, 0, 0, 0, 0, 0);
#if MSDOSFS_AUTO_UNLOAD
	OSKextRetainKextWithLoadTag(OSKextGetCurrentLoadTag());
#endif
	
	error = copyin(data, &args, sizeof(struct msdosfs_args));
	if (error)
		goto error_exit;
	if (args.magic != MSDOSFS_ARGSMAGIC)
		args.flags = 0;

	/*
	 * If updating, check whether changing from read-only to
	 * read/write; if there is no device name, that's all we do.
	 */
	if (vfs_isupdate(mp)) {
		pmp = VFSTOMSDOSFS(mp);
		error = 0;
		if (!(pmp->pm_flags & MSDOSFSMNT_RONLY) && vfs_isrdonly(mp)) {
			/* Downgrading from read/write to read-only */
			/* � Is vflush() sufficient?  Is there more we should flush out? */
			flags = WRITECLOSE;
			if (vfs_isforce(mp))
				flags |= FORCECLOSE;
			error = vflush(mp, NULLVP, flags);
		}
		if (!error && vfs_isreload(mp))
			/* not yet implemented */
			error = ENOTSUP;
		if (error)
			goto error_exit;
		if ((pmp->pm_flags & MSDOSFSMNT_RONLY) && vfs_iswriteupgrade(mp)) {
			/* � Assuming that VFS has verified we can write to the device */

			pmp->pm_flags &= ~MSDOSFSMNT_RONLY;

			/* Now that the volume is modifiable, mark it dirty */
			error = msdosfs_markvoldirty(pmp, 1);
			if (error) {
				pmp->pm_flags |= MSDOSFSMNT_RONLY;
				goto error_exit;
			}
		}
	}

	if ( !vfs_isupdate(mp)) {
		error = msdosfs_mount(devvp, mp, context);
		if (error)
			goto error_exit;	/* msdosfs_mount cleaned up already */
	}

	if (error == 0)
		error = msdosfs_update_mp(mp, &args);

	if (error == 0)
		(void) msdosfs_vfs_statfs(mp, vfs_statfs(mp), context);

	if (error)
		msdosfs_vfs_unmount(mp, MNT_FORCE, context);	/* NOTE: calls OSKextReleaseKextWithLoadTag */

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_MOUNT|DBG_FUNC_END, error, 0, 0, 0, 0);
	return error;

error_exit:
#if MSDOSFS_AUTO_UNLOAD
	OSKextReleaseKextWithLoadTag(OSKextGetCurrentLoadTag());
#endif
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_MOUNT|DBG_FUNC_END, error, 0, 0, 0, 0);
	return error;
}

/*
 * Create a version 3 UUID from unique data in the SHA1 "name space".
 * Version 3 UUIDs are derived using MD5 checksum.  Here, the unique
 * data is the 4-byte volume ID and the number of sectors (normalized
 * to a 4-byte little endian value).
 */
static void msdosfs_generate_volume_uuid(uuid_t result_uuid, uint8_t volumeID[4], uint32_t totalSectors)
{
    MD5_CTX c;
    uint8_t sectorsLittleEndian[4];
    
    UUID_DEFINE( kFSUUIDNamespaceSHA1, 0xB3, 0xE2, 0x0F, 0x39, 0xF2, 0x92, 0x11, 0xD6, 0x97, 0xA4, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC );

    /*
     * Normalize totalSectors to a little endian value so that this returns the
     * same UUID regardless of endianness.
     */
    putuint32(sectorsLittleEndian, totalSectors);
    
    /*
     * Generate an MD5 hash of our "name space", and our unique bits of data
     * (the volume ID and total sectors).
     */
    MD5Init(&c);
    MD5Update(&c, kFSUUIDNamespaceSHA1, sizeof(uuid_t));
    MD5Update(&c, volumeID, 4);
    MD5Update(&c, sectorsLittleEndian, sizeof(sectorsLittleEndian));
    MD5Final(result_uuid, &c);
    
    /* Force the resulting UUID to be a version 3 UUID. */
    result_uuid[6] = (result_uuid[6] & 0x0F) | 0x30;
    result_uuid[8] = (result_uuid[8] & 0x3F) | 0x80;
}

int msdosfs_mount(vnode_t devvp, struct mount *mp, vfs_context_t context)
{
	struct msdosfsmount *pmp;
	struct buf *bp;
	dev_t dev = vnode_specrdev(devvp);
	union bootsector *bsp;
	struct byte_bpb50 *b50;
	struct byte_bpb710 *b710;
	uint32_t total_sectors;
	uint32_t fat_sectors;
	uint32_t clusters;
	uint32_t fsinfo = 0;
	int	error;
	struct vfsstatfs *vfsstatfs;
	u_int8_t SecPerClust;
	
	/*
	 * Disallow multiple mounts of the same device.
	 * Disallow mounting of a device that is currently in use
	 * (except for root, which might share swap device for miniroot).
	 * Flush out any old buffers remaining from a previous use.
	 *
	 *� Obsolete?

	error = vfs_mountedon(devvp);
	if (error)
		return (error);
	if (vcount(devvp) > 1 && devvp != rootvp)
		return (EBUSY);
	 */

	error = buf_invalidateblks(devvp, BUF_WRITE_DATA, 0, 0);
	if (error)
		return (error);

	vfs_setlocklocal(mp);
	
	bp  = NULL; /* both used in error_exit */
	pmp = NULL;

	/*
	 * Read the boot sector of the filesystem, and then check the
	 * boot signature.  If not a dos boot sector then error out.
	 *
	 * NOTE: 4096 is a maximum sector size in current...
	 */
	error = (int)buf_meta_bread(devvp, 0, 4096, vfs_context_ucred(context), &bp);
	if (error)
		goto error_exit;
	buf_markaged(bp);
	bsp = (union bootsector *)buf_dataptr(bp);
	b50 = (struct byte_bpb50 *)bsp->bs50.bsBPB;
	b710 = (struct byte_bpb710 *)bsp->bs710.bsBPB;


	/* [2699033]
	 *
	 * The first three bytes are an Intel x86 jump instruction.  It should be one
	 * of the following forms:
	 *    0xE9 0x?? 0x??
	 *    0xEB 0x?? 0x90
	 * where 0x?? means any byte value is OK.
	 *
	 * [5016947]
	 *
	 * Windows doesn't actually check the third byte if the first byte is 0xEB,
	 * so we don't either
	 */
	if (bsp->bs50.bsJump[0] != 0xE9
		&& bsp->bs50.bsJump[0] != 0xEB)
	{
		error = EINVAL;
		goto error_exit;
	}

	MALLOC(pmp, struct msdosfsmount *, sizeof(*pmp), M_TEMP, M_WAITOK);
	bzero((caddr_t)pmp, sizeof *pmp);
	pmp->pm_mountp = mp;
	pmp->pm_fat_lock = lck_mtx_alloc_init(msdosfs_lck_grp, msdosfs_lck_attr);
	pmp->pm_rename_lock = lck_mtx_alloc_init(msdosfs_lck_grp, msdosfs_lck_attr);

	/*
	 * Compute several useful quantities from the bpb in the
	 * bootsector.  Copy in the dos 5 variant of the bpb then fix up
	 * the fields that are different between dos 5 and dos 3.3.
	 */
	SecPerClust = b50->bpbSecPerClust;
	pmp->pm_BytesPerSec = getuint16(b50->bpbBytesPerSec);
	pmp->pm_bpcluster = (u_int32_t) SecPerClust * (u_int32_t) pmp->pm_BytesPerSec;
	pmp->pm_ResSectors = getuint16(b50->bpbResSectors);
	pmp->pm_FATs = b50->bpbFATs;
	pmp->pm_RootDirEnts = getuint16(b50->bpbRootDirEnts);
	total_sectors = getuint16(b50->bpbSectors);
	if (total_sectors == 0)
		total_sectors = getuint32(b50->bpbHugeSectors);
	fat_sectors = getuint16(b50->bpbFATsecs);
	if (fat_sectors == 0)
		fat_sectors = getuint32(b710->bpbBigFATsecs);
	pmp->pm_label_cluster = CLUST_EOFE;	/* Assume there is no label in the root */

	/*
	 * Check a few values (could do some more):
	 * - logical sector size: power of 2, >= block size
	 * - sectors per cluster: power of 2, >= 1
	 * - number of sectors:   >= 1, <= size of partition
	 * - number of FAT sectors > 0 (too large values handled later)
	 */
	if (total_sectors == 0)
	{
		printf("msdosfs_mount: invalid total sectors (%u)\n", total_sectors);
		error = EINVAL;
		goto error_exit;
	}
	if (fat_sectors == 0)
	{
		printf("msdosfs_mount: invalid sectors per FAT (%u)\n", fat_sectors);
		error = EINVAL;
		goto error_exit;
	}
	if (SecPerClust == 0 || (SecPerClust & (SecPerClust - 1)))
	{
		printf("msdosfs_mount: invalid sectors per cluster (%u)\n", SecPerClust);
		error = EINVAL;
		goto error_exit;
	}
	if ((pmp->pm_BytesPerSec < DEV_BSIZE) ||
		(pmp->pm_BytesPerSec > 4096) ||
		(pmp->pm_BytesPerSec & (pmp->pm_BytesPerSec - 1)))
	{
		printf("msdosfs_mount: invalid bytes per sector (%u)\n", pmp->pm_BytesPerSec);
		error = EINVAL;
		goto error_exit;
	}
	
	/* calculate the ratio of sector size to device block size */
	error = VNOP_IOCTL(devvp, DKIOCGETBLOCKSIZE, (caddr_t) &pmp->pm_BlockSize, 0, context);
	if (error) {
		error = ENXIO;
		goto error_exit;
	}
	pmp->pm_BlocksPerSec = pmp->pm_BytesPerSec / pmp->pm_BlockSize;
	pmp->pm_bnshift = ffs(pmp->pm_BlockSize) - 1;
	
	/* Get the device's physical sector size */
	error = VNOP_IOCTL(devvp, DKIOCGETPHYSICALBLOCKSIZE, (caddr_t) &pmp->pm_PhysBlockSize, 0, context);
	if (error)
		pmp->pm_PhysBlockSize = pmp->pm_BlockSize;
	
	/*
	 * For now, assume a FAT12 or FAT16 volume with a dedicated root directory.
	 * We need these values before we can figure out how many clusters (and
	 * thus whether the volume is FAT32 or not).  Start by calculating sectors.
	 */
	pmp->pm_rootdirblk = (pmp->pm_ResSectors + (pmp->pm_FATs * fat_sectors));
	pmp->pm_rootdirsize = (pmp->pm_RootDirEnts * sizeof(struct dosdirentry) + pmp->pm_BytesPerSec - 1) / pmp->pm_BytesPerSec;
	pmp->pm_firstcluster = pmp->pm_rootdirblk + pmp->pm_rootdirsize;
	
	/* Change the root directory values to physical (device) blocks */
	pmp->pm_rootdirblk *= pmp->pm_BlocksPerSec;
	pmp->pm_rootdirsize *= pmp->pm_BlocksPerSec;
	
	if (fat_sectors > total_sectors ||
		pmp->pm_rootdirblk < fat_sectors ||		/* Catch numeric overflow! */
		pmp->pm_firstcluster + SecPerClust > total_sectors)
	{
		/* We think there isn't room for even a single cluster. */
		printf("msdosfs_mount: invalid configuration; no room for clusters\n");
		error = EINVAL;
		goto error_exit;
	}
	
	/*
	 * Usable clusters are numbered starting at 2, so the maximum usable cluster
	 * is (number of clusters) + 1.  Convert the pm_firstcluster to device blocks.
	 */
	pmp->pm_maxcluster = (total_sectors - pmp->pm_firstcluster) / SecPerClust + 1;
	pmp->pm_firstcluster *= pmp->pm_BlocksPerSec;
	
	/*
	 * Figure out the FAT type based on the number of clusters.
	 */
	if (pmp->pm_maxcluster < (CLUST_RSRVD & FAT12_MASK))
	{
		pmp->pm_fatmask = FAT12_MASK;
		pmp->pm_fatmult = 3;
		pmp->pm_fatdiv = 2;
	}
	else if (pmp->pm_maxcluster < (CLUST_RSRVD & FAT16_MASK))
	{
		pmp->pm_fatmask = FAT16_MASK;
		pmp->pm_fatmult = 2;
		pmp->pm_fatdiv = 1;
	}
	else if (pmp->pm_maxcluster < (CLUST_RSRVD & FAT32_MASK))
	{
		pmp->pm_fatmask = FAT32_MASK;
		pmp->pm_fatmult = 4;
		pmp->pm_fatdiv = 1;
	}
	else
	{
		printf("msdosfs_mount: number of clusters (0x%x) is too large\n", pmp->pm_maxcluster + 1);
		error = EINVAL;
		goto error_exit;
	}
	
	/* See if FAT32 has its FAT mirrored or not. */
	if (FAT32(pmp) && (getuint16(b710->bpbExtFlags) & FATMIRROR))
	{
		pmp->pm_curfat = getuint16(b710->bpbExtFlags) & FATNUM;
	}
	else
	{
		pmp->pm_flags |= MSDOSFS_FATMIRROR;
	}
	
	/*
	 * Sanity check some differences between FAT32 and FAT12/16.
	 * Also set up FAT32's root directory and FSInfo sector.
	 */
	if (FAT32(pmp))
	{
		fsinfo = getuint16(b710->bpbFSInfo);
		pmp->pm_rootdirblk = getuint32(b710->bpbRootClust);
		if (pmp->pm_rootdirblk < CLUST_FIRST || pmp->pm_rootdirblk > pmp->pm_maxcluster)
		{
			printf("msdosfs_mount: FAT32 root starting cluster (%u) out of range (%u..%u)\n",
				   pmp->pm_rootdirblk, CLUST_FIRST, pmp->pm_maxcluster);
			error = EINVAL;
			goto error_exit;
		}
		if (pmp->pm_RootDirEnts)
		{
			printf("msdosfs_mount: FAT32 has non-zero root directory count\n");
			error = EINVAL;
			goto error_exit;
		}
		if (getuint16(b710->bpbFSVers) != 0)
		{
			printf("msdosfs_mount: FAT32 has non-zero version\n");
			error = EINVAL;
			goto error_exit;
		}
		if (getuint16(b50->bpbSectors) != 0)
		{
			printf("msdosfs_mount: FAT32 has 16-bit total sectors\n");
			error = EINVAL;
			goto error_exit;
		}
		if (getuint16(b50->bpbFATsecs) != 0)
		{
			printf("msdosfs_mount: FAT32 has 16-bit FAT sectors\n");
			error = EINVAL;
			goto error_exit;
		}
	}
	else
	{
		if (pmp->pm_RootDirEnts == 0)
		{
			printf("msdosfs_mount: FAT12/16 has zero-length root directory\n");
			error = EINVAL;
			goto error_exit;
		}
		if (total_sectors < 0x10000 && getuint16(b50->bpbSectors) == 0)
		{
			printf("msdosfs_mount: Warning: FAT12/16 total sectors (%u) fit in 16 bits, but stored in 32 bits\n", total_sectors);
		}
		if (getuint16(b50->bpbFATsecs) == 0)
		{
			printf("msdosfs_mount: Warning: FAT12/16 has 32-bit FAT sectors\n");
		}
	}
	
	/*
	 * Compute number of clusters this FAT could hold based on its total size.
	 * Pin the maximum cluster number to the size of the FAT.
	 * NOTE: We have to do this AFTER determining the FAT type.  If we did this
	 * before, we could end up deducing a different FAT type than what's actually
	 * on disk, and that would be very bad.
	 */
	clusters = fat_sectors * pmp->pm_BytesPerSec;	/* Size of FAT in bytes */
	clusters *= pmp->pm_fatdiv;
	clusters /= pmp->pm_fatmult;				/* Max number of clusters, rounded down */
	if (pmp->pm_maxcluster >= clusters) {
		printf("msdosfs_mount: Warning: number of clusters (%d) exceeds FAT "
		    "capacity (%d)\n", pmp->pm_maxcluster + 1, clusters);
		pmp->pm_maxcluster = clusters - 1;
	}

	/*
	 * Pin the maximum cluster number based on the number of sectors the device
	 * is reporting (in case that is smaller than the total sectors field(s)
	 * in the boot sector).
	 */
	uint64_t block_count;
	error = VNOP_IOCTL(devvp, DKIOCGETBLOCKCOUNT, (caddr_t) &block_count, 0, context);
	if (error == 0 && block_count < total_sectors)
	{
		if (pmp->pm_firstcluster + SecPerClust > block_count)
		{
			printf("msdosfs_mount: device sector count (%llu) too small; no room for clusters\n", block_count);
			error = EINVAL;
			goto error_exit;
		}
		
		uint32_t maxcluster = (uint32_t)((block_count - pmp->pm_firstcluster) / SecPerClust + 1);
		if (maxcluster < pmp->pm_maxcluster)
		{
			printf("msdosfs_mount: device sector count (%llu) is less than volume sector count (%u); limiting maximum cluster to %u (was %u)\n",
				   block_count, total_sectors, maxcluster, pmp->pm_maxcluster);
			pmp->pm_maxcluster = maxcluster;
		}
		else
		{
			printf("msdosfs_mount: device sector count (%llu) is less than volume sector count (%u)\n",
				   block_count, total_sectors);
		}
	}
	
	/*
	 * Set up values for accessing the FAT.
	 */
	if (FAT12(pmp))
		pmp->pm_fatblocksize = 3 * pmp->pm_BytesPerSec;
	else
		pmp->pm_fatblocksize = PAGE_SIZE;
	pmp->pm_fat_bytes = fat_sectors * pmp->pm_BytesPerSec;

	/*
	 * Compute mask and shift value for isolating cluster relative byte
	 * offsets and cluster numbers from a file offset.
	 */
	pmp->pm_crbomask = pmp->pm_bpcluster - 1;
	pmp->pm_cnshift = ffs(pmp->pm_bpcluster) - 1;

	/*
	 * Compute an "optimal" I/O size based on the largest I/O that both
	 * UBC and the device can handle.
	 */
	{
		u_int32_t	temp_size;
		struct vfsioattr ioattr;

		pmp->pm_iosize = ubc_upl_maxbufsize();
		vfs_ioattr(mp, &ioattr);
		if (ioattr.io_maxreadcnt < pmp->pm_iosize)
			pmp->pm_iosize = ioattr.io_maxreadcnt;
		if (ioattr.io_maxwritecnt < pmp->pm_iosize)
			pmp->pm_iosize = ioattr.io_maxwritecnt;
		temp_size = ioattr.io_segreadcnt * ioattr.io_maxsegreadsize;
		if (temp_size < pmp->pm_iosize)
			pmp->pm_iosize = temp_size;
		temp_size = ioattr.io_segwritecnt * ioattr.io_maxsegwritesize;
		if (temp_size < pmp->pm_iosize)
			pmp->pm_iosize = temp_size;
		
		/*
		 * If the device returned bogus values, like zeroes, pin the optimal
		 * size to the cluster size.
		 */
		if (pmp->pm_iosize < pmp->pm_bpcluster)
			pmp->pm_iosize = pmp->pm_bpcluster;
	}
	
	/* Copy volume label and serial number from boot sector into mount point */
	{
		struct extboot *extboot;
		int i;
		u_char uc;
		
		/* Start out assuming no label (empty string) */
		pmp->pm_label[0] = '\0';

		if (FAT32(pmp)) {
			extboot = (struct extboot *) bsp->bs710.bsExt;
		} else {
			extboot = (struct extboot *) bsp->bs50.bsExt;
		}
		
		if (extboot->exBootSignature == EXBOOTSIG) {
			pmp->pm_flags |= MSDOSFS_HAS_EXT_BOOT;
			
			/* Copy the volume serial number into the mount point. */
			bcopy(extboot->exVolumeID, pmp->pm_volume_serial_num, sizeof(pmp->pm_volume_serial_num));
            
            /* 
             * See if the volume has a non-zero ID; if so, turn it into a UUID.
             * The field is odd aligned, so don't cast to uint32_t.
             */
            if (pmp->pm_volume_serial_num[0] || pmp->pm_volume_serial_num[1] ||
                pmp->pm_volume_serial_num[2] || pmp->pm_volume_serial_num[3])
            {
                pmp->pm_flags |= MSDOSFS_HAS_VOL_UUID;
                msdosfs_generate_volume_uuid(pmp->pm_uuid, pmp->pm_volume_serial_num, total_sectors);
            }
			
			/*
			 * Copy the label from the boot sector into the mount point.
			 *
			 * We don't call msdosfs_dos2unicodefn() because it assumes the last three
			 * characters are an extension, and it will put a period before the
			 * extension.
			 */
			for (i=0; i<SHORT_NAME_LEN; i++) {
				uc = extboot->exVolumeLabel[i];
				if (i==0 && uc == SLOT_E5)
					uc = 0xE5;
				pmp->pm_label[i] = (uc < 0x80 || uc > 0x9F ? uc : dos2unicode[uc - 0x80]);
			}

			/* Remove trailing spaces, add NUL terminator */
			for (i=10; i>=0 && pmp->pm_label[i]==' '; --i)
				;
			pmp->pm_label[i+1] = '\0';
		}
	}
        
	/*
	 * Release the bootsector buffer.
	 */
	buf_brelse(bp);
	bp = NULL;

	/*
	 * We always reset the "next allocation" pointer to the start of the volume
	 * on mount.  FAT12 and FAT16 don't have a way to persistently store it, and
	 * we need a valid value in all cases.
	 *
	 * For FAT32, we ignore the value in the FSInfo sector so that we'll tend
	 * to put allocations toward the start of the volume.  This is a trade
	 * off.  The benefits are that reads and writes to sectors near the start
	 * of the volume are often faster, and some broken devices report more
	 * sectors than they can actually access.  The cost is large volumes with
	 * all clusters near the start of the volume already allocated; we'll
	 * have to read through that part of the FAT once, which we could have
	 * skipped if we didn't ignore the "next free cluster" in the FSInfo sector.
	 */
	pmp->pm_nxtfree = CLUST_FIRST;

	/*
	 * Check FSInfo.
	 */
	if (fsinfo) {
		struct fsinfo *fp;
		u_int32_t log_per_phys;
		
		/* Convert FSInfo logical sector number to device block number */
		fsinfo *= pmp->pm_BytesPerSec / pmp->pm_BlockSize;
		
		/*
		 * %%% We want to read/write an entire physical sector when we access
		 * %%% the FSInfo sector.  So precompute the starting logical sector
		 * %%% number, size of the physical sector, and offset of FSInfo from
		 * %%% the start of the physical sector.
		 */
		log_per_phys = pmp->pm_PhysBlockSize / pmp->pm_BlockSize;
		if ((rounddown(fsinfo,log_per_phys) + log_per_phys) <= pmp->pm_ResSectors) {
			pmp->pm_fsinfo_sector = rounddown(fsinfo,log_per_phys);
			pmp->pm_fsinfo_size = pmp->pm_PhysBlockSize;
			pmp->pm_fsinfo_offset = (fsinfo % log_per_phys) * pmp->pm_BlockSize;
		} else {
			pmp->pm_fsinfo_sector = fsinfo;
			pmp->pm_fsinfo_size = pmp->pm_BlockSize;
			pmp->pm_fsinfo_offset = 0;
		}
		
		/*
		 * The FSInfo sector occupies pm_BytesPerSec bytes on disk,
		 * but only 512 of those have meaningful contents.  There's no point
		 * in reading all pm_BytesPerSec bytes if the device block size is
		 * smaller.  So just use the device block size here.
		 */
		error = buf_meta_bread(devvp, pmp->pm_fsinfo_sector, pmp->pm_fsinfo_size, vfs_context_ucred(context), &bp);
		if (error)
			goto error_exit;
		fp = (struct fsinfo *)(buf_dataptr(bp) + pmp->pm_fsinfo_offset);
		if (!bcmp(fp->fsisig1, "RRaA", 4)
		    && !bcmp(fp->fsisig2, "rrAa", 4)
		    && !bcmp(fp->fsisig3, "\0\0\125\252", 4))
		{
			pmp->pm_nxtfree = getuint32(fp->fsinxtfree);
			pmp->pm_freeclustercount = getuint32(fp->fsinfree);
		} else {
			printf("msdosfs_mount: FSInfo has bad signature\n");
			pmp->pm_fsinfo_size = 0;
		}
		buf_brelse(bp);
		bp = NULL;
	}

	/*
	 * Check and validate (or perhaps invalidate?) the fsinfo structure?		XXX
	 */

	/*
	 * If they want fat updates to be synchronous then let them suffer
	 * the performance degradation in exchange for the on disk copy of
	 * the fat being correct just about all the time.  I suppose this
	 * would be a good thing to turn on if the kernel is still flakey.
	 */
	if (vfs_issynchronous(mp))
		pmp->pm_flags |= MSDOSFSMNT_WAITONFAT;

	/*
	 * msdosfs_fat_init_vol() needs pm_devvp.
	 */
	pmp->pm_dev = dev;
	pmp->pm_devvp = devvp;

	/*
	 * Set up the per-volume FAT structures, including
	 * the in-use map.
	 */
	error = msdosfs_fat_init_vol(pmp);
	if (error != 0)
		goto error_exit;

	/*
	 * Initialize a timer to automatically sync shortly after writing.
	 */
	pmp->pm_sync_timer = thread_call_allocate(msdosfs_meta_sync_callback, pmp);
	if (pmp->pm_sync_timer == NULL)
	{
		error = ENOMEM;
		goto error_exit;
	}
	
	/*
	 * Set up our private data pointer for use by other routines.
	 */
	vfs_setfsprivate(mp, (void *)pmp);
	
	/*
	 * Look through the root directory for volume name, and Windows hibernation.
	 */
	error = msdosfs_scan_root_dir(mp, context);
	if (error)
	{
		if (error == EIO && vfs_isrdwr(mp))
		{
			(void) msdosfs_markvoldirty(pmp, 1);	/* Verify/repair the volume next time. */
		}
		goto error_exit;
	}

	/*
	 * NOTE: we have to call vfs_isrdonly here, not cache the value from earlier.
	 * It is possible that msdosfs_scan_root_dir made the mount read-only due to a
	 * Windows hibernation image.
	 */
	if (vfs_isrdonly(mp))
		pmp->pm_flags |= MSDOSFSMNT_RONLY;
	else {
		/* [2753891] Mark the volume dirty while it is mounted read/write */
		if ((error = msdosfs_markvoldirty(pmp, 1)) != 0)
			goto error_exit;
	}

	/*
	 * Fill in the statvfs fields that are constant (not updated by msdosfs_vfs_statfs)
	 */
	vfsstatfs = vfs_statfs(mp);
	vfsstatfs->f_bsize = pmp->pm_bpcluster;
	vfsstatfs->f_iosize = pmp->pm_iosize;
	/* Clusters are numbered from 2..pm_maxcluster, so pm_maxcluster - 2 + 1 of them */
	vfsstatfs->f_blocks = pmp->pm_maxcluster - 1;
	vfsstatfs->f_fsid.val[0] = dev;
	vfsstatfs->f_fsid.val[1] = vfs_typenum(mp);

	vfs_setflags(mp, MNT_IGNORE_OWNERSHIP);
	

	return 0;

error_exit:
	if (bp)
		buf_brelse(bp);
	if (pmp) {
		if (pmp->pm_sync_timer)
		{
		    thread_call_cancel(pmp->pm_sync_timer);
			thread_call_free(pmp->pm_sync_timer);
			pmp->pm_sync_timer = NULL;
		}
		
		(void) vflush(mp, NULLVP, SKIPSYSTEM|FORCECLOSE);
		
		msdosfs_fat_uninit_vol(pmp);

		(void) vflush(mp, NULLVP, FORCECLOSE);
		
		lck_mtx_free(pmp->pm_fat_lock, msdosfs_lck_grp);
		lck_mtx_free(pmp->pm_rename_lock, msdosfs_lck_grp);
		
		FREE(pmp, M_TEMP);

		vfs_setfsprivate(mp, (void *)NULL);
	}
	return (error);
}

/*
 * Make a filesystem operational.
 * Nothing to do at the moment.
 */
/* ARGSUSED */
int msdosfs_start(struct mount *mp, int flags, vfs_context_t context)
{
#pragma unused (mp)
#pragma unused (flags)
#pragma unused (context)
	return (0);
}

/*
 * Unmount the filesystem described by mp.
 */
int msdosfs_vfs_unmount(struct mount *mp, int mntflags, vfs_context_t context)
{
	struct msdosfsmount *pmp;
	int error, flags;
	int force;

	pmp = VFSTOMSDOSFS(mp);

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_UNMOUNT|DBG_FUNC_START, pmp, 0, 0, 0, 0);
	
	flags = SKIPSYSTEM;
	force = 0;
	if (mntflags & MNT_FORCE) {
		flags |= FORCECLOSE;
		force = 1;
	}

	/*
	 * Cancel any pending timers for this volume.  Then wait for any timers
	 * which have fired, but whose callbacks have not yet completed.
	 */
	if (pmp->pm_sync_timer)
	{
		struct timespec ts = {0, 100000000};	/* 0.1 seconds */
		
		/*
		 * Cancel any timers that have been scheduled, but have not
		 * fired yet.  NOTE: The kernel considers a timer complete as
		 * soon as it starts your callback, so the kernel does not
		 * keep track of the number of callbacks in progress.
		 */
		if (thread_call_cancel(pmp->pm_sync_timer))
			OSDecrementAtomic(&pmp->pm_sync_incomplete);
		thread_call_free(pmp->pm_sync_timer);
		pmp->pm_sync_timer = NULL;
		
		/*
		 * This waits for all of the callbacks that were entered before
		 * we did thread_call_cancel above, but have not completed yet.
		 */
		while(pmp->pm_sync_incomplete > 0)
		{
			msleep(&pmp->pm_sync_incomplete, NULL, PWAIT, "msdosfs_vfs_unmount", &ts);
		}
		
		if (pmp->pm_sync_incomplete < 0)
			panic("msdosfs_vfs_unmount: pm_sync_incomplete underflow!\n");
	}
	
	error = vflush(mp, NULLVP, flags);
	if (error && !force)
		goto error_exit;

	/*
	 * [2753891] If the volume was mounted read/write, and no corruption
	 * was detected, mark it clean now.
	 */
	if ((pmp->pm_flags & (MSDOSFSMNT_RONLY | MSDOSFS_CORRUPT)) == 0) {
		error = msdosfs_markvoldirty(pmp, 0);
		if (error && !force)
			goto error_exit;
	}
	
	msdosfs_fat_uninit_vol(pmp);
	(void) vflush(mp, NULLVP, FORCECLOSE);
	VNOP_FSYNC(pmp->pm_devvp, MNT_WAIT, context);

	lck_mtx_free(pmp->pm_fat_lock, msdosfs_lck_grp);
	lck_mtx_free(pmp->pm_rename_lock, msdosfs_lck_grp);
	
	FREE(pmp, M_TEMP);

	vfs_setfsprivate(mp, (void *)NULL);

#if MSDOSFS_AUTO_UNLOAD
	OSKextReleaseKextWithLoadTag(OSKextGetCurrentLoadTag());
#endif

	/*
	 * If "force" was set, we may get here with error != 0.  Since we have
	 * in fact completed the unmount (as best we can), we need to return
	 * no error so that VFS can clean up our mount point.
	 */
	error = 0;

error_exit:
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_UNMOUNT|DBG_FUNC_END, error, 0, 0, 0, 0);
	return (error);
}

int msdosfs_vfs_root(struct mount *mp, vnode_t *vpp, vfs_context_t context)
{
	struct msdosfsmount *pmp = VFSTOMSDOSFS(mp);
	struct denode *ndep;
	int error;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_ROOT|DBG_FUNC_START, pmp, 0, 0, 0, 0);

	error = msdosfs_deget(pmp, MSDOSFSROOT, MSDOSFSROOT_OFS, NULLVP, NULL, &ndep, context);
	if (error)
		return (error);
	*vpp = DETOV(ndep);
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_ROOT|DBG_FUNC_END, 0, ndep, 0, 0, 0);
	return 0;
}


int msdosfs_vfs_statfs(struct mount *mp, struct vfsstatfs *sbp, vfs_context_t context)
{
#pragma unused (context)
	struct msdosfsmount *pmp;

	pmp = VFSTOMSDOSFS(mp);

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_STATFS|DBG_FUNC_START, pmp, 0, 0, 0, 0);

	/*
	 * � VFS fills in everything from a cached copy.
	 * We only need to fill in fields that can change.
	 */
	sbp->f_bfree = pmp->pm_freeclustercount;
	sbp->f_bavail = pmp->pm_freeclustercount;
	sbp->f_bused = sbp->f_blocks - sbp->f_bavail;
	sbp->f_files = pmp->pm_RootDirEnts;			/* XXX */
	sbp->f_ffree = 0;	/* what to put in here? */
	vfs_name(mp, sbp->f_fstypename);
	
	/* Subtypes (flavors) for MSDOS
		0 - FAT12 
		1 - FAT16
		2 - FAT32
	*/
	if (pmp->pm_fatmask == FAT12_MASK) {
		 sbp->f_fssubtype = 0;	/* FAT12 */ 
	} else if (pmp->pm_fatmask == FAT16_MASK) {
		sbp->f_fssubtype = 1;	/* FAT16 */
	} else {
		sbp->f_fssubtype = 2;	/* FAT32 */
	}

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_STATFS|DBG_FUNC_END, 0, sbp->f_blocks, sbp->f_bfree, sbp->f_fssubtype, 0);
	return 0;
}


int msdosfs_vfs_getattr(mount_t mp, struct vfs_attr *attr, vfs_context_t context)
{
#pragma unused (context)
	struct msdosfsmount *pmp;

	pmp = VFSTOMSDOSFS(mp);

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_GETATTR|DBG_FUNC_START, pmp, attr->f_active, 0, 0, 0);
	
	/* FAT doesn't track the object counts */
	
	VFSATTR_RETURN(attr, f_bsize,  pmp->pm_bpcluster);
	VFSATTR_RETURN(attr, f_iosize, pmp->pm_iosize);
	/* Clusters are numbered from 2..pm_maxcluster, so pm_maxcluster - 2 + 1 of them */
	VFSATTR_RETURN(attr, f_blocks, pmp->pm_maxcluster - 1);
	VFSATTR_RETURN(attr, f_bfree,  pmp->pm_freeclustercount);
	VFSATTR_RETURN(attr, f_bavail, pmp->pm_freeclustercount);
	VFSATTR_RETURN(attr, f_bused,  attr->f_blocks - attr->f_bfree);
	
	/* FAT doesn't have a fixed limit on the number of file nodes */
	
	if (VFSATTR_IS_ACTIVE(attr, f_fsid)) {
		attr->f_fsid.val[0] = pmp->pm_dev;
		attr->f_fsid.val[1] = vfs_typenum(mp);
		VFSATTR_SET_SUPPORTED(attr, f_fsid);
	}
	
	if (VFSATTR_IS_ACTIVE(attr, f_capabilities)) {
		attr->f_capabilities.capabilities[VOL_CAPABILITIES_FORMAT] = 
			VOL_CAP_FMT_SYMBOLICLINKS |
			VOL_CAP_FMT_NO_ROOT_TIMES |
			VOL_CAP_FMT_CASE_PRESERVING |
			VOL_CAP_FMT_FAST_STATFS |
#ifdef VOL_CAP_FMT_NO_PERMISSIONS
			VOL_CAP_FMT_NO_PERMISSIONS |
#endif
			VOL_CAP_FMT_HIDDEN_FILES ;
		attr->f_capabilities.capabilities[VOL_CAPABILITIES_INTERFACES] = 
			VOL_CAP_INT_VOL_RENAME |
			VOL_CAP_INT_ADVLOCK |
			VOL_CAP_INT_FLOCK ;
		attr->f_capabilities.capabilities[VOL_CAPABILITIES_RESERVED1] = 0;
		attr->f_capabilities.capabilities[VOL_CAPABILITIES_RESERVED2] = 0;

		attr->f_capabilities.valid[VOL_CAPABILITIES_FORMAT] =
			VOL_CAP_FMT_PERSISTENTOBJECTIDS |
			VOL_CAP_FMT_SYMBOLICLINKS |
			VOL_CAP_FMT_HARDLINKS |
			VOL_CAP_FMT_JOURNAL |
			VOL_CAP_FMT_JOURNAL_ACTIVE |
			VOL_CAP_FMT_NO_ROOT_TIMES |
			VOL_CAP_FMT_SPARSE_FILES |
			VOL_CAP_FMT_ZERO_RUNS |
			VOL_CAP_FMT_CASE_SENSITIVE |
			VOL_CAP_FMT_CASE_PRESERVING |
			VOL_CAP_FMT_FAST_STATFS | 
			VOL_CAP_FMT_2TB_FILESIZE |
			VOL_CAP_FMT_OPENDENYMODES |
#ifdef VOL_CAP_FMT_NO_PERMISSIONS
			VOL_CAP_FMT_NO_PERMISSIONS |
#endif
			VOL_CAP_FMT_HIDDEN_FILES ;
		attr->f_capabilities.valid[VOL_CAPABILITIES_INTERFACES] =
			VOL_CAP_INT_SEARCHFS |
			VOL_CAP_INT_ATTRLIST |
			VOL_CAP_INT_NFSEXPORT |
			VOL_CAP_INT_READDIRATTR |
			VOL_CAP_INT_EXCHANGEDATA |
			VOL_CAP_INT_COPYFILE |
			VOL_CAP_INT_ALLOCATE |
			VOL_CAP_INT_VOL_RENAME |
			VOL_CAP_INT_ADVLOCK |
			VOL_CAP_INT_FLOCK |
			VOL_CAP_INT_MANLOCK ;
		attr->f_capabilities.valid[VOL_CAPABILITIES_RESERVED1] = 0;
		attr->f_capabilities.valid[VOL_CAPABILITIES_RESERVED2] = 0;
		VFSATTR_SET_SUPPORTED(attr, f_capabilities);
	}

	if (VFSATTR_IS_ACTIVE(attr, f_attributes)) {
		attr->f_attributes.validattr.commonattr =
			ATTR_CMN_NAME	|
			ATTR_CMN_DEVID	|
			ATTR_CMN_FSID	|
			ATTR_CMN_OBJTYPE |
			ATTR_CMN_OBJTAG	|
			ATTR_CMN_OBJID	|
			/* ATTR_CMN_OBJPERMANENTID | */
			ATTR_CMN_PAROBJID |
			/* ATTR_CMN_SCRIPT | */
			ATTR_CMN_CRTIME |
			ATTR_CMN_MODTIME |
			ATTR_CMN_CHGTIME |
			ATTR_CMN_ACCTIME |
			/* ATTR_CMN_BKUPTIME | */
			/* ATTR_CMN_FNDRINFO | */
			ATTR_CMN_OWNERID |
			ATTR_CMN_GRPID	|
			ATTR_CMN_ACCESSMASK |
			ATTR_CMN_FLAGS	|
			ATTR_CMN_USERACCESS |
			/* ATTR_CMN_EXTENDED_SECURITY | */
			/* ATTR_CMN_UUID | */
			/* ATTR_CMN_GRPUUID | */
			0;
		attr->f_attributes.validattr.volattr =
			ATTR_VOL_FSTYPE	|
			/* ATTR_VOL_SIGNATURE */
			ATTR_VOL_SIZE	|
			ATTR_VOL_SPACEFREE |
			ATTR_VOL_SPACEAVAIL |
			ATTR_VOL_MINALLOCATION |
			ATTR_VOL_ALLOCATIONCLUMP |
			ATTR_VOL_IOBLOCKSIZE |
			/* ATTR_VOL_OBJCOUNT */
			/* ATTR_VOL_FILECOUNT */
			/* ATTR_VOL_DIRCOUNT */
			/* ATTR_VOL_MAXOBJCOUNT */
			ATTR_VOL_MOUNTPOINT |
			ATTR_VOL_NAME	|
			ATTR_VOL_MOUNTFLAGS |
			ATTR_VOL_MOUNTEDDEVICE |
			/* ATTR_VOL_ENCODINGSUSED */
			ATTR_VOL_CAPABILITIES |
			ATTR_VOL_ATTRIBUTES;
		attr->f_attributes.validattr.dirattr =
			ATTR_DIR_LINKCOUNT |
			/* ATTR_DIR_ENTRYCOUNT */
			ATTR_DIR_MOUNTSTATUS;
		attr->f_attributes.validattr.fileattr =
			ATTR_FILE_LINKCOUNT |
			ATTR_FILE_TOTALSIZE |
			ATTR_FILE_ALLOCSIZE |
			/* ATTR_FILE_IOBLOCKSIZE */
			ATTR_FILE_DEVTYPE |
			/* ATTR_FILE_FORKCOUNT */
			/* ATTR_FILE_FORKLIST */
			ATTR_FILE_DATALENGTH |
			ATTR_FILE_DATAALLOCSIZE |
			ATTR_FILE_RSRCLENGTH |
			ATTR_FILE_RSRCALLOCSIZE;
		attr->f_attributes.validattr.forkattr = 0;
		attr->f_attributes.nativeattr.commonattr =
			ATTR_CMN_NAME	|
			ATTR_CMN_DEVID	|
			ATTR_CMN_FSID	|
			ATTR_CMN_OBJTYPE |
			ATTR_CMN_OBJTAG	|
			ATTR_CMN_OBJID	|
			/* ATTR_CMN_OBJPERMANENTID | */
			ATTR_CMN_PAROBJID |
			/* ATTR_CMN_SCRIPT | */
			ATTR_CMN_CRTIME |
			ATTR_CMN_MODTIME |
			/* ATTR_CMN_CHGTIME | */	/* Supported but not native */
			ATTR_CMN_ACCTIME |
			/* ATTR_CMN_BKUPTIME | */
			/* ATTR_CMN_FNDRINFO | */
			/* ATTR_CMN_OWNERID | */	/* Supported but not native */
			/* ATTR_CMN_GRPID	| */	/* Supported but not native */
			/* ATTR_CMN_ACCESSMASK | */	/* Supported but not native */
			ATTR_CMN_FLAGS	|
			ATTR_CMN_USERACCESS |
			/* ATTR_CMN_EXTENDED_SECURITY | */
			/* ATTR_CMN_UUID | */
			/* ATTR_CMN_GRPUUID | */
			0;
		attr->f_attributes.nativeattr.volattr =
			ATTR_VOL_FSTYPE	|
			/* ATTR_VOL_SIGNATURE */
			ATTR_VOL_SIZE	|
			ATTR_VOL_SPACEFREE |
			ATTR_VOL_SPACEAVAIL |
			ATTR_VOL_MINALLOCATION |
			ATTR_VOL_ALLOCATIONCLUMP |
			ATTR_VOL_IOBLOCKSIZE |
			/* ATTR_VOL_OBJCOUNT */
			/* ATTR_VOL_FILECOUNT */
			/* ATTR_VOL_DIRCOUNT */
			/* ATTR_VOL_MAXOBJCOUNT */
			ATTR_VOL_MOUNTPOINT |
			ATTR_VOL_NAME	|
			ATTR_VOL_MOUNTFLAGS |
			ATTR_VOL_MOUNTEDDEVICE |
			/* ATTR_VOL_ENCODINGSUSED */
			ATTR_VOL_CAPABILITIES |
			ATTR_VOL_ATTRIBUTES;
		attr->f_attributes.nativeattr.dirattr = 0;
		attr->f_attributes.nativeattr.fileattr =
			/* ATTR_FILE_LINKCOUNT | */	/* Supported but not native */
			ATTR_FILE_TOTALSIZE |
			ATTR_FILE_ALLOCSIZE |
			/* ATTR_FILE_IOBLOCKSIZE */
			ATTR_FILE_DEVTYPE |
			/* ATTR_FILE_FORKCOUNT */
			/* ATTR_FILE_FORKLIST */
			ATTR_FILE_DATALENGTH |
			ATTR_FILE_DATAALLOCSIZE |
			ATTR_FILE_RSRCLENGTH |
			ATTR_FILE_RSRCALLOCSIZE;
		attr->f_attributes.nativeattr.forkattr = 0;
		VFSATTR_SET_SUPPORTED(attr, f_attributes);
	}

	/* FAT doesn't have volume dates */
	
	if (VFSATTR_IS_ACTIVE(attr, f_fssubtype)) {
		/* Subtypes (flavors) for MSDOS
			0 - FAT12 
			1 - FAT16
			2 - FAT32
		*/
		if (pmp->pm_fatmask == FAT12_MASK) {
			attr->f_fssubtype = 0;	/* FAT12 */ 
		} else if (pmp->pm_fatmask == FAT16_MASK) {
			attr->f_fssubtype = 1;	/* FAT16 */
		} else {
			attr->f_fssubtype = 2;	/* FAT32 */
		}
		VFSATTR_SET_SUPPORTED(attr, f_fssubtype);
	}
	
	/* f_bsize returned above */
	
	if (VFSATTR_IS_ACTIVE(attr, f_vol_name)) {
		strlcpy(attr->f_vol_name, (char*)pmp->pm_label, MAXPATHLEN);
		VFSATTR_SET_SUPPORTED(attr, f_vol_name);
	}
    
    if (VFSATTR_IS_ACTIVE(attr, f_uuid) && (pmp->pm_flags & MSDOSFS_HAS_VOL_UUID))
    {
        memcpy(attr->f_uuid, pmp->pm_uuid, sizeof(uuid_t));
        VFSATTR_SET_SUPPORTED(attr, f_uuid);
    }
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_GETATTR|DBG_FUNC_END, pmp, attr->f_supported, 0, 0, 0);
	return 0;
}


int msdosfs_vfs_setattr(mount_t mp, struct vfs_attr *attr, vfs_context_t context)
{
    int error = 0;
	struct msdosfsmount *pmp = VFSTOMSDOSFS(mp);

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_SETATTR|DBG_FUNC_START, pmp, attr->f_active, 0, 0, 0);

	if (VFSATTR_IS_ACTIVE(attr, f_vol_name))
	{
	    struct buf *bp = NULL;
	    size_t i;
	    size_t len;
	    size_t unichars;
		u_int16_t c;
	    u_int16_t volName[SHORT_NAME_LEN];
	    u_char label[SHORT_NAME_LEN];

		len = strlen(attr->f_vol_name);
        if (len > 63)
        	return EINVAL;

		/* Convert the UTF-8 to UTF-16 */
        error = utf8_decodestr((u_int8_t*)attr->f_vol_name, len, volName,
        	&unichars, sizeof(volName), 0, UTF_PRECOMPOSED);
        if (error)
            return error;
        unichars /= 2;	/* Bytes to characters */
		if (unichars > SHORT_NAME_LEN)
			return EINVAL;

        /*
         * Convert from UTF-16 to local encoding (like a short name).
         * We can't call msdosfs_unicode_to_dos_name here because it
		 * assumes a dot between the first 8 and last 3 characters.
         *
         * The specification doesn't say what syntax limitations exist
         * for volume labels.  By experimentation, they appear to be
         * upper case only.  I am assuming they are like short names,
         * but no period is assumed/required after the 8th character.
         */
        
        /* Name is trailing space padded, so init to all spaces. */
        for (i=0; i<SHORT_NAME_LEN; ++i)
            label[i] = ' ';

        for (i=0; i<unichars; ++i) {
            c = volName[i];
            if (c < 0x100)
                c = l2u[c];			/* Convert to lower case */
            if (c != ' ')			/* Allow space to pass unchanged */
                c = msdosfs_unicode2dos(c);	/* Convert to local encoding */
            if (c < 3)
                return EINVAL;		/* Illegal char in name */
            label[i] = c;
        }

        /* Copy the UTF-8 to pmp->pm_label */
        bcopy(attr->f_vol_name, pmp->pm_label, len);
        pmp->pm_label[len] = '\0';

        /* Update label in boot sector */
        error = (int)buf_meta_bread(pmp->pm_devvp, 0, pmp->pm_BlockSize, vfs_context_ucred(context), &bp);
        if (!error) {
            if (FAT32(pmp))
                bcopy(label, (char*)buf_dataptr(bp)+71, SHORT_NAME_LEN);
            else
                bcopy(label, (char*)buf_dataptr(bp)+43, SHORT_NAME_LEN);
            buf_bdwrite(bp);
            bp = NULL;
        }
        if (bp)
            buf_brelse(bp);
        bp = NULL;

        /*
         * Update label in root directory, if any.  For now, don't
         * create one if it doesn't exist (in case devices like
         * cameras don't understand them).
         */
        if (pmp->pm_label_cluster != CLUST_EOFE) {
        	error = msdosfs_readep(pmp, pmp->pm_label_cluster, pmp->pm_label_offset, &bp, NULL, context);
            if (!error) {
                bcopy(label, (char *)buf_dataptr(bp) + pmp->pm_label_offset, SHORT_NAME_LEN);
                buf_bdwrite(bp);
                bp = NULL;
            }
            if (bp)
                buf_brelse(bp);
            bp=NULL;
        }
        
        if (error == 0)
        	VFSATTR_SET_SUPPORTED(attr, f_vol_name);
	}
	
	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_SETATTR|DBG_FUNC_END, pmp, attr->f_supported, 0, 0, 0);
	return error;
}


struct msdosfs_sync_cargs {
	vfs_context_t context;
	int		waitfor;
	int		error;
};


int msdosfs_sync_callback(vnode_t vp, void *cargs)
{
	struct msdosfs_sync_cargs *args;
	struct denode *dep;
	int error;

	/*
	 * msdosfs_check_link creates a temporary vnode whose v_data is
	 * NULL.  It normally gets reclaimed very quickly, but it is
	 * possible for a sync() to race with that reclaim.  Since that
	 * vnode doesn't have a denode to go with it, we can just ignore
	 * it for the purposes of syncing.
	 */
	dep = VTODE(vp);
	if (dep == NULL)
		return VNODE_RETURNED;
		
	args = (struct msdosfs_sync_cargs *)cargs;
	
	/*
	 * If this is a FAT vnode, then don't sync it here.  It will be sync'ed
	 * separately in msdosfs_vfs_sync.
	 */
	if (vnode_issystem(vp))
		return VNODE_RETURNED;
	
	lck_mtx_lock(dep->de_lock);
	
	error = msdosfs_fsync_internal(vp, args->waitfor, 0, args->context);
	if (error)
		args->error = error;
		
	lck_mtx_unlock(dep->de_lock);
	
	return VNODE_RETURNED;
}


int msdosfs_vfs_sync(mp, waitfor, context)
	struct mount *mp;
	int waitfor;
	vfs_context_t context;
{
	struct msdosfsmount *pmp = VFSTOMSDOSFS(mp);
	int error, allerror = 0;
	struct msdosfs_sync_cargs args;

	if (pmp->pm_flags & MSDOSFSMNT_RONLY) {
		/* For read-only mounts, there's no syncing to do */
		return 0;
	}

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_SYNC|DBG_FUNC_START, pmp, 0, 0, 0, 0);
	
	/*
	 * Flush the FSInfo sector and all copies of the FAT.
	 */
	error = msdosfs_update_fsinfo(pmp, waitfor, context);
	if (error)
		allerror = error;

	error = VNOP_FSYNC(pmp->pm_fat_active_vp, waitfor, context);
	if (error)
		allerror = error;
	if (pmp->pm_fat_mirror_vp)
		error = VNOP_FSYNC(pmp->pm_fat_mirror_vp, waitfor, context);
	if (error)
		allerror = error;
	
	/*
	 * Write back each (modified) denode.
	 */
	args.context = context;
	args.waitfor = waitfor;
	args.error = 0;
	/*
	 * msdosfs_sync_callback will be called for each vnode
	 * hung off of this mount point... the vnode will be
	 * properly referenced and unreferenced around the callback
	 */
	vnode_iterate(mp, VNODE_ITERATE_ACTIVE, msdosfs_sync_callback, (void *)&args);

	if (args.error)
		allerror = args.error;

	/*
	 * Flush directories.
	 */
	error = VNOP_FSYNC(pmp->pm_devvp, waitfor, context);
	if (error)
		allerror = error;

	KERNEL_DEBUG_CONSTANT(MSDOSFS_VFS_SYNC|DBG_FUNC_END, allerror, 0, 0, 0, 0);
	return allerror;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

struct vfsops msdosfs_vfsops = {
	msdosfs_vfs_mount,
	msdosfs_start,
	msdosfs_vfs_unmount,
	msdosfs_vfs_root,
	NULL, /* msdosfs_quotactl */
	msdosfs_vfs_getattr,
	msdosfs_vfs_sync,
	NULL, /* msdosfs_vget */
	NULL, /* msdosfs_fhtovp */
	NULL, /* msdosfs_vptofh */
	msdosfs_init,
	NULL, /* msdosfs_sysctl */
	msdosfs_vfs_setattr,
    // Remaining ops unused
};

#pragma clang diagnostic pop

extern struct vnodeopv_desc msdosfs_vnodeop_opv_desc;
extern struct vnodeopv_desc msdosfs_fat_vnodeop_opv_desc;
static struct vnodeopv_desc *msdosfs_vnodeop_opv_desc_list[2] =
{
	&msdosfs_vnodeop_opv_desc,
	&msdosfs_fat_vnodeop_opv_desc
};


static vfstable_t msdosfs_vfsconf;

int msdosfs_module_start(kmod_info_t *ki, void *data)
{
#pragma unused(ki)
#pragma unused(data)
	errno_t error;
	struct vfs_fsentry vfe;

	vfe.vfe_vfsops = &msdosfs_vfsops;
	vfe.vfe_vopcnt = 2;		/* We just have vnode operations for regular files and directories, and the FAT */
	vfe.vfe_opvdescs = msdosfs_vnodeop_opv_desc_list;
	strlcpy(vfe.vfe_fsname, "msdos", sizeof(vfe.vfe_fsname));
	vfe.vfe_flags = VFS_TBLTHREADSAFE | VFS_TBLNOTYPENUM | VFS_TBLLOCALVOL | VFS_TBL64BITREADY | VFS_TBLREADDIR_EXTENDED;
	vfe.vfe_reserv[0] = 0;
	vfe.vfe_reserv[1] = 0;
	
	error = vfs_fsadd(&vfe, &msdosfs_vfsconf);

#if DEBUG
	if (!error)
	{
		sysctl_register_oid(&sysctl__vfs_generic_msdosfs);
		sysctl_register_oid(&sysctl__vfs_generic_msdosfs_meta_delay);
	}
#endif

	return error ? KERN_FAILURE : KERN_SUCCESS;
}

int msdosfs_module_stop(kmod_info_t *ki, void *data)
{
#pragma unused(ki)
#pragma unused(data)
    errno_t error;

    error = vfs_fsremove(msdosfs_vfsconf);
    
    if (!error)
    {
	msdosfs_uninit();
#if DEBUG
	sysctl_unregister_oid(&sysctl__vfs_generic_msdosfs_meta_delay);
	sysctl_unregister_oid(&sysctl__vfs_generic_msdosfs);
#endif
    }
    
    return error ? KERN_FAILURE : KERN_SUCCESS;
}


/*
 * Look through the root directory for a volume label entry.
 * If found, use it to replace the label in the mount point.
 * Also look for a file with short name HIBERFIL.SYS; if it is less than
 * 4KiB in size, or has non-zero bytes in the first 4KiB, then force the
 * mount to read-only because Windows is hibernated on this volume.
 */
int msdosfs_scan_root_dir(struct mount *mp, vfs_context_t context)
{
    int error;
    struct msdosfsmount *pmp;
    vnode_t vp = NULL;
    struct buf *bp = NULL;
    uint32_t frcn;	/* file relative cluster number in root directory */
    daddr64_t bn;		/* block number of current dir block */
    uint32_t cluster;	/* cluster number of current dir block */
    uint32_t blsize;	/* size of current dir block */
    unsigned blkoff;		/* dir entry offset within current dir block */
	unsigned diroff;	/* offset from start of directory */
    struct dosdirentry *dep = NULL;
    struct denode *root;
    u_int16_t unichars;
    u_int16_t ucfn[12];
    u_char uc;
    int i;
    size_t outbytes;
    char *bdata = NULL;

    pmp = VFSTOMSDOSFS(mp);

    error = msdosfs_vfs_root(mp, &vp, context);
    if (error)
        return error;
    root = VTODE(vp);
    
	diroff = 0;
    for (frcn=0; ; frcn++) {
        error = msdosfs_pcbmap(root, frcn, 1, &bn, &cluster, &blsize);
        if (error) {
            /* It is fine if no volume label entry was found in the root directory */
            if (error == E2BIG)
                error = 0;
            goto not_found;
        }

        for (blkoff = 0; blkoff < blsize; blkoff += sizeof(struct dosdirentry), diroff += sizeof(struct dosdirentry)) {
			/* Make sure we have the buffer containing the current entry */
			if (bp == NULL) {
				error = (int)buf_meta_bread(pmp->pm_devvp, bn, blsize, vfs_context_ucred(context), &bp);
				if (error) {
					goto not_found;
				}
				bdata = (char *)buf_dataptr(bp);
			}
			
            dep = (struct dosdirentry *) (bdata + blkoff);

            /* Skip deleted directory entries */
            if (dep->deName[0] == SLOT_DELETED)
                continue;
            
            /* Stop if we hit the end of the directory (a never used entry) */
            if (dep->deName[0] == SLOT_EMPTY) {
                goto not_found;
            }

            /* Skip long name entries */
            if (dep->deAttributes == ATTR_WIN95)
                continue;
            
            if (dep->deAttributes & ATTR_VOLUME) {
                pmp->pm_label_cluster = cluster;
                pmp->pm_label_offset = blkoff;

                /*
                 * Copy the dates from the label to the root vnode.
                 */
                root->de_CHun = dep->deCHundredth;
                root->de_CTime = getuint16(dep->deCTime);
                root->de_CDate = getuint16(dep->deCDate);
                root->de_ADate = getuint16(dep->deADate);
                root->de_MTime = getuint16(dep->deMTime);
                root->de_MDate = getuint16(dep->deMDate);

				/*
                 * We don't call msdosfs_dos2unicodefn() because it assumes the last three
                 * characters are an extension, and it will put a period before the
                 * extension.
                 */
				for (i=0; i<SHORT_NAME_LEN; i++) {
					uc = dep->deName[i];
					if (i==0 && uc == SLOT_E5)
						uc = 0xE5;
					ucfn[i] = (uc < 0x80 || uc > 0x9F ? (u_int16_t)uc : dos2unicode[uc - 0x80]);
				}
				for (i=10; i>=0 && ucfn[i]==' '; --i)
					;
				unichars = i+1;
				
				/* translate the name in ucfn into UTF-8 */
				error = utf8_encodestr(ucfn, unichars * 2,
								pmp->pm_label, &outbytes,
								sizeof(pmp->pm_label), 0, UTF_DECOMPOSED);
                goto found;
            }
			
			if (!bcmp(dep->deName, "HIBERFILSYS", SHORT_NAME_LEN))
			{
				struct denode *hibernate = NULL;
				buf_t hibernate_bp = NULL;
				char *hibernate_data = NULL;
				
				if (getuint32(dep->deFileSize) < 4096)
				{
					if (DEBUG) printf("msdosfs: Volume is hibernated; hiberfile.sys < 4096 bytes\n");
					goto hibernated;
				}
				
				/* Release the current directory block so we can msdosfs_deget() the current entry. */
				buf_brelse(bp);
				bp = NULL;

				/* Need to open the file so we can check the first 4KiB */
				error = msdosfs_deget(pmp, cluster, diroff, NULL, NULL, &hibernate, context);
				if (error)
				{
					printf("msdosfs: error %d trying to open hiberfil.sys\n", error);
					error = 0;
					goto hibernated;
				}
				error = buf_meta_bread(DETOV(hibernate), 0, 4096, vfs_context_ucred(context), &hibernate_bp);
				if (error)
				{
					printf("msdosfs: error %d trying to read hiberfil.sys\n", error);
					error = 0;
					goto hibernated;
				}
				hibernate_data = (char *) buf_dataptr(hibernate_bp);
				if (!strncmp(hibernate_data, "hibr", 4))
				{
					if (DEBUG) printf("msdosfs: Volume is hibernated; signature = 'hibr'\n");
					goto hibernated;
				}
				if (!strncmp(hibernate_data, "HIBR", 4))
				{
					if (DEBUG) printf("msdosfs: Volume is hibernated; signature = 'HIBR'\n");
					goto hibernated;
				}
				for (i=0; i<4096; ++i)
				{
					if (hibernate_data[i])
					{
						if (DEBUG) printf("msdosfs: Volume is hibernated (non-zero header)\n");
						goto hibernated;
					}
				}
				
				if (DEBUG) printf("msdosfs: Volume is not hibernated (hiberfil.sys header all zeroes)\n");
				goto not_hibernated;
				
hibernated:
				printf("msdosfs: Mounting hibernated volume read-only\n");
				vfs_setflags(mp, MNT_RDONLY);
				
not_hibernated:
				if (hibernate_bp)
					buf_brelse(hibernate_bp);
				if (hibernate)
				{
					vnode_recycle(DETOV(hibernate));
					vnode_put(DETOV(hibernate));
				}
			}
        }
        
		/* We're done with the current block.  Release it if we still have it. */
		if (bp != NULL) {
			buf_brelse(bp);
			bp = NULL;
		}
    }

found:
not_found:
    if (bp)
        buf_brelse(bp);

    if (vp) {
        if (error)
            vnode_recycle(vp);
        vnode_put(vp);
    }

    return error;
}
