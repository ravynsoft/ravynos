/*
 * Copyright (c) 2000-2008, 2010-2012 Apple Inc. All rights reserved.
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
/* $FreeBSD: src/sys/msdosfs/direntry.h,v 1.15 1999/12/29 04:54:52 peter Exp $ */
/*	$NetBSD: direntry.h,v 1.14 1997/11/17 15:36:32 ws Exp $	*/

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
 * Structure of a dos directory entry.
 */

#ifndef __direntry_h_
#define __direntry_h_

struct dosdirentry {
	u_int8_t	deName[8];	/* filename, blank filled */
#define	SLOT_EMPTY	0x00		/* slot has never been used */
#define	SLOT_E5		0x05		/* the real value is 0xe5 */
#define	SLOT_DELETED	0xe5		/* file in this slot deleted */
	u_int8_t	deExtension[3];	/* extension, blank filled */
	u_int8_t	deAttributes;	/* file attributes */
#define	ATTR_NORMAL	0x00		/* normal file */
#define	ATTR_READONLY	0x01		/* file is read-only (immutable) */
#define	ATTR_HIDDEN	0x02		/* file is hidden */
#define	ATTR_SYSTEM	0x04		/* file is a system file */
#define	ATTR_VOLUME	0x08		/* entry is a volume label */
#define	ATTR_DIRECTORY	0x10		/* entry is a directory name */
#define	ATTR_ARCHIVE	0x20		/* file is new or modified */
	u_int8_t	deLowerCase;	/* NT VFAT lower case flags */
#define	LCASE_BASE	0x08		/* filename base in lower case */
#define	LCASE_EXT	0x10		/* filename extension in lower case */
	u_int8_t	deCHundredth;	/* hundredth of seconds in CTime */
	u_int8_t	deCTime[2];	/* create time */
	u_int8_t	deCDate[2];	/* create date */
	u_int8_t	deADate[2];	/* access date */
	u_int8_t	deHighClust[2];	/* high bytes of cluster number */
	u_int8_t	deMTime[2];	/* last update time */
	u_int8_t	deMDate[2];	/* last update date */
	u_int8_t	deStartCluster[2]; /* starting cluster of file */
	u_int8_t	deFileSize[4];	/* size of file in bytes */
};

/*
 * Structure of a Win95 long name directory entry
 */
struct winentry {
	u_int8_t	weCnt;
#define	WIN_LAST	0x40
#define	WIN_CNT		0x3f
	u_int8_t	wePart1[10];
	u_int8_t	weAttributes;
#define	ATTR_WIN95	0x0f
#define	ATTR_WIN95_MASK	0x3f
	u_int8_t	weReserved1;
	u_int8_t	weChksum;
	u_int8_t	wePart2[12];
	u_int16_t	weReserved2;
	u_int8_t	wePart3[4];
};
#define	WIN_CHARS	13	/* Number of chars per winentry */

/*
 * Maximum filename length in Win95
 * Note: Must be < sizeof(dirent.d_name)
 */
#define	WIN_MAXLEN	255

/*
 * Maximum filename length for short names:
 * 8 bytes of filename plus 3 bytes of extension.
 * The dot between the filename and extension is implied.
 */
#define SHORT_NAME_LEN	11

/*
 * This is the format of the contents of the deTime field in the dosdirentry
 * structure.
 * We don't use bitfields because we don't know how compilers for
 * arbitrary machines will lay them out.
 */
#define DT_2SECONDS_MASK	0x1F	/* seconds divided by 2 */
#define DT_2SECONDS_SHIFT	0
#define DT_MINUTES_MASK		0x7E0	/* minutes */
#define DT_MINUTES_SHIFT	5
#define DT_HOURS_MASK		0xF800	/* hours */
#define DT_HOURS_SHIFT		11

/*
 * This is the format of the contents of the deDate field in the dosdirentry
 * structure.
 */
#define DD_DAY_MASK		0x1F	/* day of month */
#define DD_DAY_SHIFT		0
#define DD_MONTH_MASK		0x1E0	/* month */
#define DD_MONTH_SHIFT		5
#define DD_YEAR_MASK		0xFE00	/* year - 1980 */
#define DD_YEAR_SHIFT		9

#ifdef KERNEL
struct dirent;
void msdosfs_unix2dostime(struct timespec *tsp, u_int16_t *ddp, 
	     u_int16_t *dtp, u_int8_t *dhp);
void msdosfs_dos2unixtime(u_int dd, u_int dt, u_int dh, struct timespec *tsp);
size_t msdosfs_dos2unicodefn(u_char dn[SHORT_NAME_LEN], u_int16_t *un, int lower);
int msdosfs_unicode_to_dos_name(const uint16_t *unicode,
								size_t unicode_length,
								uint8_t short_name[SHORT_NAME_LEN],
								u_int8_t *lower_case);
int msdosfs_apply_generation_to_short_name(uint8_t short_name[SHORT_NAME_LEN],
										   unsigned generation);
int msdosfs_unicode2winfn(const u_int16_t *un, int unlen, struct winentry *wep, int cnt, int chksum);
int msdosfs_winChkName(const u_int16_t *un, int unlen, struct winentry *wep, int chksum, u_int16_t *found_name, boolean_t *case_folded);
int msdosfs_getunicodefn(struct winentry *wep, u_int16_t ucfn[WIN_MAXLEN], u_int16_t *unichars, int chksum);
u_int8_t msdosfs_winChksum(u_int8_t *name);
int msdosfs_winSlotCnt(const u_int16_t *un, int unlen);
u_char msdosfs_unicode2dos(u_int16_t uc);
int msdosfs_fsync_internal(vnode_t vp, int sync, int do_dirs, vfs_context_t context);

#endif	/* KERNEL */

#endif /* __direntry_h_ */
