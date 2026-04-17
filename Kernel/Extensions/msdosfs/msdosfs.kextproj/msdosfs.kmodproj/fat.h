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
/* $FreeBSD: src/sys/msdosfs/fat.h,v 1.9 1999/12/29 04:54:53 peter Exp $ */
/*	$NetBSD: fat.h,v 1.12 1997/11/17 15:36:36 ws Exp $	*/

/*-
 * Copyright (C) 1994, 1997 Wolfgang Solfrank.
 * Copyright (C) 1994, 1997 TooLs GmbH.
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
 * Some useful cluster numbers.
 */
#define	MSDOSFSROOT	0		/* cluster 0 means the root dir */
#define	CLUST_FREE	0		/* cluster 0 also means a free cluster */
#define	MSDOSFSFREE	CLUST_FREE
#define	CLUST_FIRST	2		/* first legal cluster number */
#define	CLUST_RSRVD	0xfffffff6	/* reserved cluster range */
#define	CLUST_BAD	0xfffffff7	/* a cluster with a defect */
#define	CLUST_EOFS	0xfffffff8	/* start of eof cluster range */
#define	CLUST_EOFE	0xffffffff	/* end of eof cluster range */

/*
 * Note: FILENO_EMPTY must be larger than FAT32_MASK so that it can't accidentally
 * occur as a valid cluster number.
 */
#define FILENO_EMPTY	999999999	/* Fake file number used for empty files */
#define FILENO_ROOT	1		/* File number used for root directory of FAT12 and FAT16 */

#define	FAT12_MASK	0x00000fff	/* mask for 12 bit cluster numbers */
#define	FAT16_MASK	0x0000ffff	/* mask for 16 bit cluster numbers */
#define	FAT32_MASK	0x0fffffff	/* mask for FAT32 cluster numbers */

/*
 * MSDOSFS:
 * Return true if filesystem uses 12 bit fats. Microsoft Programmer's
 * Reference says if the maximum cluster number in a filesystem is greater
 * than 4078 ((CLUST_RSRVS - CLUST_FIRST) & FAT12_MASK) then we've got a
 * 16 bit fat filesystem. While mounting, the result of this test is stored
 * in pm_fatentrysize.
 * GEMDOS-flavour (atari):
 * If the filesystem is on floppy we've got a 12 bit fat filesystem, otherwise
 * 16 bit. We check the d_type field in the disklabel struct while mounting
 * and store the result in the pm_fatentrysize. Note that this kind of
 * detection gets flakey when mounting a vnd-device.
 */
#define	FAT12(pmp)	(pmp->pm_fatmask == FAT12_MASK)
#define	FAT16(pmp)	(pmp->pm_fatmask == FAT16_MASK)
#define	FAT32(pmp)	(pmp->pm_fatmask == FAT32_MASK)

#define	MSDOSFSEOF(pmp, cn)	((((cn) | ~(pmp)->pm_fatmask) & CLUST_EOFS) == CLUST_EOFS)

/*
		Symbolic Links for FAT

FAT does not have native support for symbolic links (symlinks).  We
implement them using ordinary files with a particular format.  Our
symlink file format is modeled after the SMB for Mac OS X implementation.

Symlink files are ordinary text files that look like:

XSym
1234
00112233445566778899AABBCCDDEEFF
/the/sym/link/path

The lines of the file are separated by ASCII newline (0x0A).  The first
line is a "magic" value to help identify the file.  The second line is
the length of the symlink itself; it is four decimal digits, with leading
zeroes.  The third line is the MD5 checksum of the symlink as 16
hexadecimal bytes.  The fourth line is the symlink, up to 1024 bytes long.
If the symlink is less than 1024 bytes, then it is padded with a single
newline character and as many spaces as needed to occupy 1024 bytes.

The file size is exactly 1067 (= 4 + 1 + 4 + 1 + 32 + 1 + 1024) bytes.
When we encounter an ordinary file whose length is 1067, we must read
it to verify that the header (including length and MD5 checksum) is
correct.

Since the file size is constant, we use the de_FileSize field in the
denode to store the actual length of the symlink.  That way, we only
check and parse the header once at vnode creation time.

*/

static const char symlink_magic[5] = "XSym\n";

#define SYMLINK_LINK_MAX 1024

struct symlink {
	char magic[5];		/* == symlink_magic */
	char length[4];		/* four decimal digits */
	char newline1;		/* '\n' */
	char md5[32];		/* MD5 hex digest of "length" bytes of "link" field */
	char newline2;		/* '\n' */
	char link[SYMLINK_LINK_MAX]; /* "length" bytes, padded by '\n' and spaces */
};

#ifdef KERNEL
/*
 * These are the values for the function argument to the function
 * msdosfs_fatentry().
 */
#define	FAT_GET		0x0001	/* get a fat entry */
#define	FAT_SET		0x0002	/* set a fat entry */
#define	FAT_GET_AND_SET	(FAT_GET | FAT_SET)

/*
 * Flags to msdosfs_extendfile:
 */
#define	DE_CLEAR	1	/* Zero out the blocks allocated */
#define DE_SYNC		IO_SYNC	/* 0x4 do it synchronisly...from vnode.h */


void msdosfs_fat_init(void);
void msdosfs_fat_uninit(void);
int  msdosfs_fat_init_vol(struct msdosfsmount *pmp);
void msdosfs_fat_uninit_vol(struct msdosfsmount *pmp);
int msdosfs_update_fsinfo(struct msdosfsmount *pmp, int waitfor, vfs_context_t context);
int msdosfs_pcbmap (struct denode *dep, uint32_t findcn, uint32_t numclusters, daddr64_t *bnp, uint32_t *cnp, uint32_t *sp);
int msdosfs_pcbmap_internal(struct denode *dep, uint32_t findcn, uint32_t numclusters, daddr64_t *bnp, uint32_t *cnp, uint32_t *sp);
int msdosfs_clusteralloc(struct msdosfsmount *pmp, uint32_t start, uint32_t count, uint32_t fillwith, uint32_t *retcluster, uint32_t *got);
int msdosfs_fatentry(int function, struct msdosfsmount *pmp, uint32_t cluster, uint32_t *oldcontents, uint32_t newcontents);
int msdosfs_freeclusterchain(struct msdosfsmount *pmp, uint32_t startchain);
int msdosfs_extendfile(struct denode *dep, uint32_t count, uint32_t *numAllocated);

/* [2753891]
 * Routine to mark a FAT16 or FAT32 volume as "clean" or "dirty" by manipulating the upper bit
 * of the FAT entry for cluster 1.  Note that this bit is not defined for FAT12 volumes.
 */
int msdosfs_markvoldirty(struct msdosfsmount *pmp, int dirty);

/*
 * Write the primary/active FAT and all directories to the device.  This
 * skips the boot sector, FSInfo sector, and non-active copies of the FAT.
 */
void msdosfs_meta_flush(struct msdosfsmount *pmp, int sync);
void msdosfs_meta_sync_callback(void *pmp, void *unused);

enum vtype msdosfs_check_link(struct denode *dep, vfs_context_t context);

extern u_char l2u[256];

/*
 * Tunables to control delayed metadata sync.
 */
extern uint32_t msdosfs_meta_delay;

#endif	/* KERNEL */
