/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
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
/* $FreeBSD: src/sys/msdosfs/msdosfsmount.h,v 1.20 2000/01/27 14:43:07 nyan Exp $ */
/*	$NetBSD: msdosfsmount.h,v 1.17 1997/11/17 15:37:07 ws Exp $	*/

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
 * This software is provided "as is"MSDOSFileSystem.
 *
 * The author supplies this software to be publicly redistributed on the
 * understanding that the author is not responsible for the correct
 * functioning of this software in any circumstances and is not liable for
 * any damages caused by this software.
 *
 * October 1992
 */

#ifndef _MSDOSFS_MSDOSFSMOUNT_H_
#define	_MSDOSFS_MSDOSFSMOUNT_H_

#ifdef KERNEL

#include <libkern/OSTypes.h>
#include <kern/thread_call.h>

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_MSDOSFSMNT);
#endif

/*
 * Layout of the mount control block for a msdos file system.
 */
struct msdosfsmount {
	struct mount *pm_mountp;/* vfs mount struct for this fs */
	dev_t pm_dev;		/* block special device mounted */
	uid_t pm_uid;		/* uid to set as owner of the files */
	gid_t pm_gid;		/* gid to set as owner of the files */
	mode_t pm_mask;		/* mask to and with file protection bits */
	vnode_t pm_devvp;	/* vnode for block device mntd */
	struct bpb50 pm_bpb;	/* BIOS parameter blk for this fs */
	u_int32_t pm_BlockSize;	/* device's logical block size */
	u_int32_t pm_PhysBlockSize;	/* device's physical block size */
	uint32_t pm_BlocksPerSec;	/* pm_BytesPerSec divided by pm_BlockSize (device blocks per DOS sector) */
	uint32_t pm_rootdirblk;	/* block # (cluster # for FAT32) of root directory number */
	uint32_t pm_rootdirsize;	/* number of physical (device) blocks in root directory (FAT12 and FAT16 only) */
	uint32_t pm_firstcluster;	/* sector number of first cluster (relative to start of volume) */
	uint32_t pm_maxcluster;	/* maximum cluster number; clusters are in range 2..pm_maxcluster */
	uint32_t pm_freeclustercount;	/* number of free clusters */
	uint32_t pm_cnshift;	/* shift file offset right this amount to get a cluster number */
	uint32_t pm_crbomask;	/* and a file offset with this mask to get cluster rel offset */
	uint32_t pm_bnshift;	/* shift amount to convert between bytes and physical (device) blocks */
	uint32_t pm_bpcluster;	/* bytes per cluster */
	uint32_t pm_fatblocksize;	/* size of fat blocks in bytes */
	uint32_t pm_fatmask;	/* mask to use for fat numbers */
	uint32_t pm_fsinfo_sector;	/* Logical block number to read to get FSInfo */
	uint32_t pm_fsinfo_size;	/* Number of bytes to read to get FSInfo */
	uint32_t pm_fsinfo_offset;	/* Offset, in bytes, from pm_fsinfo_sector to start of FSInfo */
	uint32_t pm_nxtfree;	/* next free cluster in fsinfo block */
	u_int pm_fatmult;	/* these 2 values are used in fat */
	u_int pm_fatdiv;	/*	offset computation */
	u_int pm_curfat;	/* current fat for FAT32 (0 otherwise) */
	u_int pm_flags;		/* see below */
	uint8_t pm_volume_serial_num[4];	/* Volume serial number (ID) from boot block. */
	uuid_t pm_uuid;
	u_int8_t pm_label[64];	/* Volume name/label */
	uint32_t pm_label_cluster; /* logical cluster within root that contains the label */
	uint32_t pm_label_offset;	/* byte offset of label within above cluster */
	lck_mtx_t	*pm_fat_lock;	/* Protects the File Allocation Table (in memory, and on disk) */
	lck_mtx_t	*pm_rename_lock;
	vnode_t pm_fat_active_vp;	/* vnode for accessing the active FAT */
	vnode_t pm_fat_mirror_vp;	/* vnode for accessing FAT mirrors */
	void *pm_free_extents;		/* PAGE_SIZE bytes; holds known free extents. */
	void *pm_fat_cache;	/* most recently used block of the FAT */
	u_int32_t pm_fat_bytes;	/* size of one copy of the FAT, in bytes */
	u_int32_t pm_fat_cache_offset;	/* which block is being cached; relative to start of active FAT */
	int pm_fat_flags;	/* Flags about state of the FAT; see constants below */
	uint32_t pm_free_extent_count;	/* Number of extents in *pm_free_extents. */

	/*
	 * About the sync counters:
	 * pm_sync_scheduled  keeps track whether a timer was scheduled but we
	 *                    haven't started processing the callback (i.e. we
	 *                    haven't begun the flush).  This will be non-zero
	 *                    even if the callback has been invoked, before we
	 *                    start the flush.
	 * pm_sync_incomplete keeps track of the number of callbacks that have
	 *                    not completed yet (including callbacks not yet
	 *                    invoked).  We cannot safely unmount until this
	 *                    drops to zero.
	 *
	 * In both cases, we use counters, not flags, so that we can avoid
	 * taking locks.
	 */
	SInt32		pm_sync_scheduled;
	SInt32		pm_sync_incomplete;
	thread_call_t	pm_sync_timer;
	
	u_int32_t	pm_iosize;		/* "optimal" I/O size */
};

/* Flag bits for pm_fat_flags */
enum {
	FAT_CACHE_DIRTY = 1,
	FSINFO_DIRTY = 2
};

/* Byte offset in FAT on filesystem pmp, cluster cn */
#define	FATOFS(pmp, cn)	((cn) * (pmp)->pm_fatmult / (pmp)->pm_fatdiv)

#define	VFSTOMSDOSFS(mp)	((struct msdosfsmount *)vfs_fsprivate((mp)))

/*
 * Shorthand for fields in the bpb contained in the msdosfsmount structure.
 */
#define	pm_BytesPerSec	pm_bpb.bpbBytesPerSec
#define	pm_ResSectors	pm_bpb.bpbResSectors
#define	pm_FATs		pm_bpb.bpbFATs
#define	pm_RootDirEnts	pm_bpb.bpbRootDirEnts

/*
 * Convert pointer to buffer -> pointer to dosdirentry
 */
#define	bptoep(pmp, bp, dirofs) \
	((struct dosdirentry *)((buf_dataptr(bp))	\
	 + ((dirofs) & (pmp)->pm_crbomask)))
/*
 * Convert number of blocks to number of clusters
 */
#define	de_bn2cn(pmp, bn) \
	((bn) >> ((pmp)->pm_cnshift - (pmp)->pm_bnshift))

/*
 * Convert number of clusters to number of blocks
 */
#define	de_cn2bn(pmp, cn) \
	((daddr64_t)(cn) << ((pmp)->pm_cnshift - (pmp)->pm_bnshift))

/*
 * Convert file offset to number of clusters
 */
#define de_cluster(pmp, off) \
	((uint32_t)((off) >> (pmp)->pm_cnshift))

/*
 * Clusters required to hold size bytes
 */
#define	de_clcount(pmp, size) \
	((uint32_t)(((off_t)(size) + (pmp)->pm_bpcluster - 1) >> (pmp)->pm_cnshift))

/*
 * Convert file offset to first block number of cluster containing offset.
 */
#define de_blk(pmp, off) \
	(de_cn2bn(pmp, de_cluster((pmp), (off))))

/*
 * Convert number of clusters to number of bytes
 */
#define	de_cn2off(pmp, cn) \
	((cn) << (pmp)->pm_cnshift)

/*
 * Convert number of blocks to number of bytes
 */
#define	de_bn2off(pmp, bn) \
	((bn) << (pmp)->pm_bnshift)
/*
 * Map a cluster number into a filesystem relative block number.
 */
#define	cntobn(pmp, cn) \
	(de_cn2bn((pmp), (cn)-CLUST_FIRST) + (pmp)->pm_firstcluster)

/*
 * Calculate block number for directory entry in root dir, offset dirofs
 */
#define	roottobn(pmp, dirofs) \
	(de_blk((pmp), (dirofs)) + (pmp)->pm_rootdirblk)

/*
 * Calculate block number for directory entry at cluster dirclu, offset
 * dirofs
 */
#define	detobn(pmp, dirclu, dirofs) \
	((dirclu) == MSDOSFSROOT \
	 ? roottobn((pmp), (dirofs)) \
	 : cntobn((pmp), (dirclu)))

int msdosfs_mountroot(mount_t mp, vnode_t devvp, vfs_context_t context);
uid_t get_pmuid(struct msdosfsmount *pmp, uid_t current_user);

extern lck_grp_attr_t *msdosfs_lck_grp_attr;
extern lck_grp_t *msdosfs_lck_grp;
extern lck_attr_t *msdosfs_lck_attr;

extern OSMallocTag msdosfs_malloc_tag;

#endif /* KERNEL */

/*
 *  Arguments to mount MSDOS filesystems.
 */
struct msdosfs_args {
#ifndef KERNEL
	char		*fspec;		/* path of device to mount */
#endif
	uid_t		uid;		/* uid that owns msdosfs files */
	gid_t		gid;		/* gid that owns msdosfs files */
	mode_t		mask;		/* mask to be applied for msdosfs perms */
	uint32_t	flags;		/* see below */
	uint32_t	magic;		/* version number */
	int32_t		secondsWest;	/* for GMT<->local time conversions */
	u_int8_t	label[64];	/* Volume label in UTF-8 */
};

/*
 * Msdosfs mount options:
 */
/*#define	MSDOSFSMNT_SHORTNAME	1	Force old DOS short names only */
/*#define	MSDOSFSMNT_LONGNAME		2	Force Win'95 long names */
/*#define	MSDOSFSMNT_NOWIN95		4	Completely ignore Win95 entries */
/*#ifndef __FreeBSD__*/
/*#define	MSDOSFSMNT_GEMDOSFS		8	This is a gemdos-flavour */
/*#endif*/
/*#define MSDOSFSMNT_U2WTABLE     0x10	Local->Unicode and local<->DOS tables loaded */
/*#define MSDOSFSMNT_ULTABLE      0x20	Local upper<->lower table loaded */
#define MSDOSFSMNT_SECONDSWEST	0x40	/* Use secondsWest for GMT<->local time conversion */
#define MSDOSFSMNT_LABEL		0x80	/* UTF-8 volume label in label[]; deprecated */

/* All flags above: */
#define MSDOSFSMNT_MNTOPT		(MSDOSFSMNT_SECONDSWEST | MSDOSFSMNT_LABEL)

/* Flags from the pm_flags field: */
#define	MSDOSFSMNT_RONLY		0x80000000	/* mounted read-only	*/
#define	MSDOSFSMNT_WAITONFAT	0x40000000	/* mounted synchronous	*/
#define	MSDOSFS_FATMIRROR		0x20000000	/* FAT is mirrored */
#define MSDOSFS_CORRUPT			0x10000000	/* Runtime corruption detected. */
#define MSDOSFS_HAS_EXT_BOOT		0x08000000	/* Boot sector has "extended boot" fields. */
#define MSDOSFS_HAS_VOL_UUID            0x04000000      /* Volume has a valid UUID in pm_uuid */

#define MSDOSFS_ARGSMAGIC		0xe4eff301

#endif /* !_MSDOSFS_MSDOSFSMOUNT_H_ */
