/*
 * Copyright (c) 2000, 2002-2013 Apple Inc. All rights reserved.
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
/* $FreeBSD: src/sys/msdosfs/denode.h,v 1.20 1999/12/29 04:54:52 peter Exp $ */
/*	$NetBSD: denode.h,v 1.25 1997/11/17 15:36:28 ws Exp $	*/

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
 * This is the pc filesystem specific portion of the vnode structure.
 *
 * To describe a file uniquely the de_dirclust, de_diroffset, and
 * de_StartCluster fields are used.
 *
 * de_dirclust contains the cluster number of the directory cluster
 *	containing the entry for a file or directory.
 * de_diroffset is the index into the parent directory (relative to the start
 *  of the directory, not the start of de_dirclust!) for the entry describing
 *	a file or directory.  Use de_diroffset modulo cluster size to find the
 *  offset of the directory entry from the start of de_dirclust.
 * de_StartCluster is the number of the first cluster of the file or directory.
 *
 * Now to describe the quirks of the pc filesystem.
 * - Clusters 0 and 1 are reserved.
 * - The first allocatable cluster is 2.
 * - The root directory is of fixed size and all blocks that make it up
 *   are contiguous.  (FAT12 and FAT16; in FAT32, the root directory is an
 *   ordinary directory)
 * - Cluster 0 refers to the root directory when it is found in the
 *   startcluster field of a directory entry that points to another directory.
 * - Cluster 0 implies a 0 length file when found in the start cluster field
 *   of a directory entry that points to a file.
 * - You can't use the cluster number 0 to derive the address of the root
 *   directory.
 * - Multiple directory entries can point to a directory. The entry in the
 *   parent directory points to a child directory.  Any directories in the
 *   child directory contain a ".." entry that points back to the parent.
 *   The child directory itself contains a "." entry that points to itself.
 * - The root directory does not contain a "." or ".." entry.
 * - The de_dirclust and de_diroffset for a directory point at the directory's
 *   own "." entry.  Since that is always the first entry, de_dirclust is the
 *   first cluster of the directory, and de_diroffset is zero.
 * - Directory entries for directories are never changed once they are created
 *   (except when removed).  The size stays 0, and the last modification time
 *   is never changed.  This is because so many directory entries can point to
 *   the physical clusters that make up a directory.  It would lead to an
 *   update nightmare.
 * - For the POSIX/BSD APIs, the dates for a directory come from the "." entry
 *   in the directory.
 * - The length field in a directory entry pointing to a directory contains 0
 *   (always).  The only way to find the end of a directory is to follow the
 *   cluster chain until the "last cluster" marker is found.
 *
 * My extensions to make this house of cards work.  These apply only to the in
 * memory copy of the directory entry.
 * - A reference count for each denode will be kept since dos doesn't keep such
 *   things.
 */

/*
 * Internal pseudo-offset for (nonexistent) directory entry for the root
 * dir in the root dir
 */
#define	MSDOSFSROOT_OFS	0x1fffffff

/*
 * This is the in memory variant of a dos directory entry.  It is the file
 * system specific data pointed to by a vnode.
 */
struct denode {
	lck_mtx_t *de_lock;			/* denode lock */
	struct denode *de_next;		/* Hash chain forward */
	struct denode **de_prev;	/* Hash chain back */
	vnode_t de_vnode;			/* addr of vnode we are part of */
	vnode_t de_devvp;			/* vnode of blk dev we live on */
	uint32_t de_flag;				/* flag bits */
	dev_t de_dev;				/* device where direntry lives */
	uint32_t de_dirclust;			/* cluster of the directory file containing this entry */
	uint32_t de_diroffset;		/* offset of this entry from the start of parent directory */
	int de_refcnt;				/* reference count */
	struct msdosfsmount *de_pmp;	/* addr of our mount struct */
	u_char de_Name[12];			/* name, from DOS directory entry */
	u_char de_Attributes;		/* attributes, from directory entry */
	u_char de_LowerCase;		/* NT VFAT lower case flags */
	u_char de_CHun;				/* Hundredth of second of CTime*/
	uint16_t de_CTime;			/* creation time */
	uint16_t de_CDate;			/* creation date */
	uint16_t de_ADate;			/* access date */
	uint16_t de_MTime;			/* modification time */
	uint16_t de_MDate;			/* modification date */
	uint32_t de_StartCluster;		/* starting cluster of file */
	uint32_t de_FileSize;			/* size of file, or length of symlink, in bytes */
	uint32_t de_LastCluster;		/* The last cluster of the file, or zero for empty files */
	u_quad_t de_modrev;			/* Revision level for lease. */
	struct msdosfs_lockf *de_lockf; /* Head of byte range lock list. */
	struct denode *de_parent;	/* Parent directory denode */
	
	/*
	 * A hack for caching the results of logical-to-physical block mapping.
	 * Basically, just cache the most recently used extent (contiguous run).
	 *
	 * Note that the lock below should really also protect the start cluster
	 * and file size fields (i.e. anything that might be affected by truncation
	 * or used in msdosfs_pcbmap).
	 */
	lck_mtx_t *de_cluster_lock;	/* Protects the cached extent fields */
	uint32_t de_cluster_physical;	/* First physical cluster of cached extent */
	uint32_t de_cluster_logical;	/* First logical cluster of cached extent */
	uint32_t de_cluster_count;	/* Size of cached extent, in clusters; 0 => no cached extent */
};

/*
 * Values for the de_flag field of the denode.
 */
#define DE_ROOT		0x0001	/* This node is the root directory */
#define DE_SYMLINK	0x0002	/* This node is a symlink */
#define	DE_UPDATE	0x0004	/* Modification time update request */
#define	DE_CREATE	0x0008	/* Creation time update */
#define	DE_ACCESS	0x0010	/* Access time update */
#define	DE_MODIFIED	0x0020	/* Denode (directory entry) has been modified */
#define DE_ATTR_MOD	0x0040	/* de_Attributes field has been modified */
#define DE_INIT		0x0080	/* Denode is in the process of being initialized */
#define DE_WAITINIT	0x0100	/* Someone is sleeping (on denode) waiting for initialization to finish */


#define DE_EXTERNALIZE(dp, dep)				\
	 ((dp)->deAttributes = (dep)->de_Attributes,	\
	 (dp)->deCHundredth = (dep)->de_CHun,		\
	 putuint16((dp)->deCTime, (dep)->de_CTime),	\
	 putuint16((dp)->deCDate, (dep)->de_CDate),	\
	 putuint16((dp)->deADate, (dep)->de_ADate),	\
	 putuint16((dp)->deMTime, (dep)->de_MTime),	\
	 putuint16((dp)->deMDate, (dep)->de_MDate),	\
	 putuint16((dp)->deStartCluster, (dep)->de_StartCluster), \
	 putuint32((dp)->deFileSize,			\
	     ((dep)->de_Attributes & ATTR_DIRECTORY) ? 0 :	\
			((dep)->de_flag & DE_SYMLINK) ? sizeof(struct symlink) : (dep)->de_FileSize), \
	 putuint16((dp)->deHighClust, (dep)->de_StartCluster >> 16))

/*
 * DE_EXTERNALIZE_ROOT is used to write the root directory's dates to the
 * volume label entry (if any).
 */
#define DE_EXTERNALIZE_ROOT(dp, dep)				\
	((dp)->deCHundredth = (dep)->de_CHun,		\
	 putuint16((dp)->deCTime, (dep)->de_CTime),	\
	 putuint16((dp)->deCDate, (dep)->de_CDate),	\
	 putuint16((dp)->deADate, (dep)->de_ADate),	\
	 putuint16((dp)->deMTime, (dep)->de_MTime),	\
	 putuint16((dp)->deMDate, (dep)->de_MDate))

#define	de_forw		de_chain[0]
#define	de_back		de_chain[1]

#ifdef KERNEL

#define	VTODE(vp)	((struct denode *)vnode_fsnode((vp)))
#define	DETOV(de)	((de)->de_vnode)

#define	DETIMES(dep, acc, mod, cre) do {				\
	if ((dep)->de_flag & DE_UPDATE) { 				\
		(dep)->de_flag |= DE_MODIFIED;				\
		msdosfs_unix2dostime((mod), &(dep)->de_MDate, &(dep)->de_MTime,	\
		    NULL);						\
		(dep)->de_Attributes |= ATTR_ARCHIVE; 			\
	}								\
	if ((dep)->de_flag & DE_ACCESS) {				\
	    	u_int16_t adate;					\
									\
		msdosfs_unix2dostime((acc), &adate, NULL, NULL);		\
		if (adate != (dep)->de_ADate) {				\
			(dep)->de_flag |= DE_MODIFIED;			\
			(dep)->de_ADate = adate;			\
		}							\
	}								\
	if ((dep)->de_flag & DE_CREATE) {				\
		msdosfs_unix2dostime((cre), &(dep)->de_CDate, &(dep)->de_CTime,	\
		    &(dep)->de_CHun);					\
		    (dep)->de_flag |= DE_MODIFIED;			\
	}								\
	(dep)->de_flag &= ~(DE_UPDATE | DE_CREATE | DE_ACCESS);		\
} while (0);


extern int (**msdosfs_vnodeop_p)(void *);

int msdosfs_vnop_lookup(struct vnop_lookup_args *ap);
int msdosfs_lookup_name(
	struct denode *dep,					/* parent directory */
	struct componentname *cnp,			/* the name to look up */
	uint32_t *dirclust,					/* cluster containing short name entry */
	uint32_t *diroffset,				/* byte offset from start of directory */
	struct dosdirentry *direntry,		/* copy of found directory entry */
	u_int16_t *found_name,
	u_int16_t *found_name_length,
	boolean_t *case_folded,
	vfs_context_t context);
int msdosfs_vnop_inactive(struct vnop_inactive_args *ap);
int msdosfs_vnop_reclaim(struct vnop_reclaim_args *ap);
int msdosfs_vnop_blktooff(struct vnop_blktooff_args *ap);
int msdosfs_vnop_offtoblk(struct vnop_offtoblk_args *ap);
int msdosfs_vnop_blockmap(struct vnop_blockmap_args *ap);

/*
 * Internal service routine prototypes.
 */
void msdosfs_hash_init(void);
void msdosfs_hash_uninit(void);
int msdosfs_deget(struct msdosfsmount *pmp, uint32_t dirclust, uint32_t diroffset, vnode_t dvp, struct componentname *cnp, struct denode **, vfs_context_t context);
int msdosfs_scan_dir_for_short_name(struct denode *dep, u_char short_name[SHORT_NAME_LEN], vfs_context_t context);
int msdosfs_uniqdosname(struct denode *dep, u_char short_name[SHORT_NAME_LEN], uint32_t dir_offset, vfs_context_t context);

int msdosfs_readep(struct msdosfsmount *pmp, uint32_t dirclu, uint32_t dirofs,  struct buf **bpp, struct dosdirentry **epp, vfs_context_t context);
int readde(struct denode *dep, struct buf **bpp, struct dosdirentry **epp, vfs_context_t context);
int msdosfs_deextend(struct denode *dep, uint32_t length, int flags, vfs_context_t context);
void msdosfs_hash_reinsert(struct denode *dep);
int msdosfs_dosdirempty(struct denode *dep, vfs_context_t context);
int msdosfs_createde(struct denode *dep, struct denode *ddep, struct denode **depp, struct componentname *cnp, uint32_t offset, uint32_t long_count, vfs_context_t context);
int msdosfs_deupdat(struct denode *dep, int waitfor, vfs_context_t context);
int msdosfs_removede(struct denode *pdep, uint32_t offset, vfs_context_t context);
int msdosfs_detrunc(struct denode *dep, uint32_t length, int flags, vfs_context_t context);
int msdosfs_doscheckpath( struct denode *source, struct denode *target, vfs_context_t context);
int msdosfs_findslots(struct denode *dep, struct componentname *cnp, uint8_t short_name[SHORT_NAME_LEN], int *needs_generation, uint8_t *lower_case, uint32_t *offset, uint32_t *long_count, vfs_context_t context);
uint32_t msdosfs_defileid(struct denode *dep);
int msdosfs_dir_flush(struct denode *dep, int sync);
int msdosfs_dir_invalidate(struct denode *dep);
#endif	/* KERNEL */
