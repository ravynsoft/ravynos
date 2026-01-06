/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
/*	$OpenBSD: archive.h,v 1.2 1996/06/26 05:31:18 deraadt Exp $	*/
/*	$NetBSD: archive.h,v 1.6 1995/03/25 06:39:43 glass Exp $	*/

/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hugh Smith at The University of Guelph.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *
 *	@(#)archive.h	8.3 (Berkeley) 4/2/94
 */

#include <sys/types.h>

/* Ar(1) options. */
#define	AR_A	0x0001
#define	AR_B	0x0002
#define	AR_C	0x0004
#define	AR_D	0x0008
#define	AR_M	0x0010
#define	AR_O	0x0020
#define	AR_P	0x0040
#define	AR_Q	0x0080
#define	AR_R	0x0100
#define	AR_T	0x0200
#define	AR_TR	0x0400
#define	AR_U	0x0800
#define	AR_V	0x1000
#define	AR_X	0x2000
#define	AR_S	0x4000
extern u_int options;

/* Set up file copy. */
#define	SETCF(from, fromname, to, toname, pad) { \
	cf.rfd = from; \
	cf.rname = fromname; \
	cf.wfd = to; \
	cf.wname = toname; \
	cf.flags = pad; \
}

/* File copy structure. */
typedef struct {
	int rfd;			/* read file descriptor */
	char *rname;			/* read name */
	int wfd;			/* write file descriptor */
	char *wname;			/* write name */
#define	NOPAD	0x00			/* don't pad */
#define	RPAD	0x01			/* pad on reads */
#define	WPAD	0x02			/* pad on writes */
	u_int flags;			/* pad flags */
} CF;

/* Header structure internal format. */
typedef struct {
	off_t size;			/* size of the object in bytes */
	time_t date;			/* date */
	int lname;			/* size of the long name in bytes */
	gid_t gid;			/* group */
	uid_t uid;			/* owner */
	u_short mode;			/* permissions */
	char name[MAXNAMLEN + 1];	/* name */
} CHDR;

/*
 * cctools-port:
 *  Replaced 'qd' with 'lld' in format strings
 *  https://github.com/tpoechtrager/osxcross/issues/216#issuecomment-599044727
 * */

/* Header format strings. */
#define	HDR1	"%s%-13d%-12ld%-6u%-6u%-8o%-10lld%2s"
#define	HDR2	"%-16.16s%-12ld%-6u%-6u%-8o%-10lld%2s"

#define	OLDARMAXNAME	15
#define	HDR3	"%-16.15s%-12ld%-6u%-6u%-8o%-10lld%2s"


#include <sys/cdefs.h>

struct stat;

void	close_archive __P((int));
void	copy_ar __P((CF *, off_t));
int	get_arobj __P((int));
int	open_archive __P((int));
void	put_arobj __P((CF *, struct stat *));
void	skip_arobj __P((int));

extern int archive_opened_for_writing;
