/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*
 * Copyright (C) 1995, 1996, 1997 Wolfgang Solfrank
 * Copyright (c) 1995 Martin Husemann
 * Some structure declaration borrowed from Paul Popelka
 * (paulp@uts.amdahl.com), see /sys/msdosfs/ for reference.
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
 *	This product includes software developed by Martin Husemann
 *	and Wolfgang Solfrank.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DOSFS_H
#define DOSFS_H

#define DOSBOOTBLOCKSIZE 512
#define MAX_SECTOR_SIZE 4096

typedef	u_int32_t	cl_t;	/* type holding a cluster number */

/*
 * architecture independent description of all the info stored in a
 * FAT boot block.
 */
struct bootblock {
	u_int	BytesPerSec;		/* bytes per sector */
	u_int	SecPerClust;		/* sectors per cluster */
	u_int	ResSectors;		/* number of reserved sectors */
	u_int	FATs;			/* number of FATs */
	u_int	RootDirEnts;		/* number of root directory entries */
	u_int	Media;			/* media descriptor */
	u_int	FATsmall;		/* number of sectors per FAT */
	u_int	SecPerTrack;		/* sectors per track */
	u_int	Heads;			/* number of heads */
	u_int32_t Sectors;		/* total number of sectors */
	u_int32_t HiddenSecs;		/* # of hidden sectors */
	u_int32_t HugeSectors;		/* # of sectors if bpbSectors == 0 */
	u_int	FSInfo;			/* FSInfo sector */
	u_int	Backup;			/* Backup of Bootblocks */
	cl_t	RootCl;			/* Start of Root Directory */
	cl_t	FSFree;			/* Number of free clusters acc. FSInfo */
	cl_t	FSNext;			/* Next free cluster acc. FSInfo */

	/* and some more calculated values */
	u_int	flags;			/* some flags: */
#define	FAT32		1		/* this is a FAT32 filesystem */
					/*
					 * Maybe, we should separate out
					 * various parts of FAT32?	XXX
					 */
	int	ValidFat;		/* valid fat if FAT32 non-mirrored */
	cl_t	ClustMask;		/* mask for entries in FAT */
	cl_t	NumClusters;		/* # of entries in a FAT */
	u_int32_t NumSectors;		/* how many sectors are there */
	u_int32_t FATsecs;		/* how many sectors are in FAT */
	u_int32_t NumFatEntries;	/* how many entries really are there */
	u_int	ClusterOffset;		/* at what sector would cluster #2 start */
	u_int	ClusterSize;		/* Cluster size in bytes */

	/* Now some statistics: */
	u_int	NumFiles;		/* # of plain files */
	u_int	NumFree;		/* # of free clusters */
	u_int	NumBad;			/* # of bad clusters */
};

/*
 * Note: FAT32 only uses 28 bits for cluster numbers; the upper 4 bits are
 * reserved.  For ease of coding, reserved/bad/EOF values are sign extended
 * to a full 32 bits regardless of FAT type.
 *
 * The CLUST_ERROR value is a value that can't be represented in the FAT.
 * It is a synthetic value that indicates an error occured.
 */
#define	CLUST_FREE	0		/* 0 means cluster is free */
#define	CLUST_FIRST	2		/* 2 is the minimum valid cluster number */
#define CLUST_ERROR	0xEEEEEEEE	/* fat_get got an error */
#define	CLUST_RSRVD	0xfffffff6	/* start of reserved clusters */
#define	CLUST_BAD	0xfffffff7	/* a cluster with a defect */
#define	CLUST_EOFS	0xfffffff8	/* start of EOF indicators */
#define	CLUST_EOF	0xffffffff	/* standard value for last cluster */

/*
 * Masks for cluster values
 */
#define	CLUST12_MASK	0x0fff
#define	CLUST16_MASK	0xffff
#define	CLUST32_MASK	0x0fffffff

#define	DOSLONGNAMELEN	256		/* long name maximal length */
#define LRFIRST		0x40		/* first long name record */
#define	LRNOMASK	0x1f		/* mask to extract long record
					 * sequence number */

/*
 * Architecture independent description of a directory entry
 */
struct dosDirEntry {
	struct dosDirEntry
		*parent,		/* previous tree level */
		*next,			/* next brother */
		*child;			/* if this is a directory */
	char name[8+1+3+1];		/* alias name first part */
	char lname[DOSLONGNAMELEN];	/* real name */
	uint flags;			/* attributes */
	cl_t head;			/* cluster no */
	cl_t end;			/* cluster # to stop iterating at */
	u_int32_t size;			/* filesize in bytes */
	u_int64_t physicalSize;		/* Number of bytes in object's cluster chain */
	uint fsckflags;			/* flags during fsck */
};
/* Flags in fsckflags: */
#define	DIREMPTY	1
#define	DIREMPWARN	2

/*
 *  TODO-list of unread directories
 */
struct dirTodoNode {
	struct dosDirEntry *dir;
	struct dirTodoNode *next;
};

#endif
