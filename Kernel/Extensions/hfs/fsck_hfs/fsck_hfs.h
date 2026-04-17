/*
 * Copyright (c) 1999-2008 Apple Inc. All rights reserved.
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

#include "cache.h"


const extern char	*cdevname;		/* name of device being checked */
extern char	*progname;
extern char	nflag;			/* assume a no response */
extern char	yflag;			/* assume a yes response */
extern char	preen;			/* just fix normal inconsistencies */
extern char	force;			/* force fsck even if clean */
extern char	debug;			/* output debugging info */
extern char	disable_journal;	/* If debug, and set, do not simulate journal replay */
extern char	embedded;		/* built for embedded */
extern char	hotroot;		/* checking root device */
extern char	scanflag;		/* Scan disk for bad blocks */

extern int	upgrading;		/* upgrading format */

extern int	fsmodified;		/* 1 => write done to file system */
extern int	fsreadfd;		/* file descriptor for reading file system */
extern int	fswritefd;		/* file descriptor for writing file system */
extern Cache_t	fscache;

extern int  detonator_run;


#define DIRTYEXIT  3		/* Filesystem Dirty, no checks */
#define FIXEDROOTEXIT  4	/* Writeable Root Filesystem was fixed */
#define	EEXIT	8		/* Standard error exit. */
#define	MAJOREXIT	47	/* We had major errors when doing a early-exit verify */


char	       *blockcheck __P((char *name));
void            cleanup_fs_fd __P((void));
void		catch __P((int));
void		ckfini __P((int markclean));
void		pfatal __P((const char *fmt, ...));
void		pwarn __P((const char *fmt, ...));
void		logstring(void *, const char *);     // write to log file 
void		outstring(void *, const char *);     // write to standard out
void 		llog(const char *fmt, ...);          // write to log file
void 		olog(const char *fmt, ...);          // write to standard out
void            plog(const char *fmt, ...);          // printf replacement that writes to both log file and standard out
void            vplog(const char *fmt, va_list ap);  // vprintf replacement that writes to both log file and standard out
void            fplog(FILE *stream, const char *fmt, ...);    // fprintf replacement that writes to both log file and standard out
#define printf  plog      // just in case someone tries to use printf/fprint
#define fprintf fplog

int		reply __P((char *question));

void		start_progress(void);
void		draw_progress(int);
void		end_progress(void);
void		DumpData(const void *ptr, size_t sz, char *label);

