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
/*	$NetBSD: archive.c,v 1.7 1995/03/26 03:27:46 glass Exp $	*/
/*	$OpenBSD: archive.c,v 1.2 1996/03/02 00:40:57 tholo Exp $	*/

/*-
 * Copyright (c) 1990, 1993, 1994
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
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)archive.c	8.3 (Berkeley) 4/2/94";
static char rcsid[] = "$NetBSD: archive.c,v 1.7 1995/03/26 03:27:46 glass Exp $";
#endif
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h> /* cctools-port */

#include <ar.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <unistd.h>

#include <mach-o/fat.h>

#include "archive.h"
#include "extern.h"

typedef struct ar_hdr HDR;
static char hb[sizeof(HDR) + 1];	/* real header */

int archive_opened_for_writing = 0;

#ifndef DEFFILEMODE
#define DEFFILEMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#endif

int
open_archive(mode)
	int mode;
{
	int created, fd, nr, r;
	char buf[SARMAG];
	
	created = 0;
	if (mode & O_CREAT) {
		mode |= O_EXCL;
		if ((fd = open(archive, mode, DEFFILEMODE)) >= 0) {
			/* POSIX.2 puts create message on stderr. */
			if (!(options & AR_C))
				warnx("creating archive %s", archive);
			created = 1;
			goto opened;
		}
		if (errno != EEXIST)
			error(archive);
		mode &= ~O_EXCL;
	}
	if ((fd = open(archive, mode, DEFFILEMODE)) < 0)
		error(archive);

	if((mode & O_ACCMODE) == O_RDONLY)
	    goto skip_flock;

	/* 
	 * Attempt to place a lock on the opened file - if we get an 
	 * error then someone is already working on this library (or
	 * it's going across NFS).
	 */
opened:
	r = flock(fd, LOCK_EX|LOCK_NB);
	if (r && errno == EAGAIN) {
		/*
		 * If we get EAGAIN sleep for a second and loop up to 10 times
		 * trying again.
		 */
		static int tries = 0;

		sleep(1);
		tries++;
		if (tries < 10)
		    goto opened;
	}
	if (r) {
		switch (errno)
		{
		/* Something interupted us let's not try again. */
		case EINTR:

		/* Coding errors */
		case EACCES:
		case EBADF:
		case EMFILE:
		case EINVAL:
		case ESRCH:
		case EAGAIN:
		case EFAULT:
		case EROFS:
		case EOVERFLOW:
		case EFBIG:

		/* Something bad happened  so no point in going on. */
		case EISDIR:
		case EDEADLK:
		case ESTALE:
			error(archive);
			break;

		/* Locking is supported but we are out of resources right now */
		case ENOLCK:

		/* Locking seems to not be working */
		case ENOTSUP:
		case EHOSTUNREACH:
#ifdef __APPLE__ /* cctools-port */
		case EBADRPC:
#endif /* __APPLE__ */
		default:
			/* Filesystem does not support locking */
			break;
		}
	}
skip_flock:

	/*
	 * If not created, O_RDONLY|O_RDWR indicates that it has to be
	 * in archive format.
	 */
	if (!created &&
	    ((mode & O_ACCMODE) == O_RDONLY || (mode & O_ACCMODE) == O_RDWR)) {
		if ((nr = read(fd, buf, SARMAG) != SARMAG)) {
			if (nr >= 0)
				badfmt();
			error(archive);
		} else if (bcmp(buf, ARMAG, SARMAG)) {
			uint32_t magic;
			memcpy(&magic, buf, sizeof(uint32_t));
#ifdef __BIG_ENDIAN__
			if(magic == FAT_MAGIC)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
			if(magic == FAT_CIGAM)
#endif /* __LITTLE_ENDIAN__ */
				fprintf(stderr, "ar: %s is a fat file (use "
					"libtool(1) or lipo(1) and ar(1) on "
					"it)\n", archive);
			badfmt();
		}
	} else if (write(fd, ARMAG, SARMAG) != SARMAG)
		error(archive);

	if ((mode & O_ACCMODE) == O_RDWR)
		archive_opened_for_writing = 1;

	return (fd);
}

void
close_archive(fd)
	int fd;
{

	(void)close(fd);			/* Implicit unlock. */
}

/* Convert ar header field to an integer. */
#define	AR_ATOI(from, to, len, base) { \
	memmove(buf, from, len); \
	buf[len] = '\0'; \
	to = strtol(buf, (char **)NULL, base); \
}

/*
 * get_arobj --
 *	read the archive header for this member
 */
int
get_arobj(fd)
	int fd;
{
	struct ar_hdr *hdr;
	size_t len, nr;
	char *p, buf[20];
	long longval;

	nr = read(fd, hb, sizeof(HDR));
	if (nr != sizeof(HDR)) {
		if (!nr)
			return (0);
		if (nr < 0)
			error(archive);
		badfmt();
	}

	hdr = (struct ar_hdr *)hb;
	if (strncmp(hdr->ar_fmag, ARFMAG, sizeof(ARFMAG) - 1))
		badfmt();

	/* Convert the header into the internal format. */
#define	DECIMAL	10
#define	OCTAL	 8

	AR_ATOI(hdr->ar_date, chdr.date, sizeof(hdr->ar_date), DECIMAL);
	AR_ATOI(hdr->ar_uid, longval, sizeof(hdr->ar_uid), DECIMAL);
	chdr.uid = (uid_t)longval;
	AR_ATOI(hdr->ar_gid, longval, sizeof(hdr->ar_gid), DECIMAL);
	chdr.gid = (gid_t)longval;
	AR_ATOI(hdr->ar_mode, chdr.mode, sizeof(hdr->ar_mode), OCTAL);
	AR_ATOI(hdr->ar_size, chdr.size, sizeof(hdr->ar_size), DECIMAL);

	/* Leading spaces should never happen. */
	if (hdr->ar_name[0] == ' ')
		badfmt();

	/*
	 * Long name support.  Set the "real" size of the file, and the
	 * long name flag/size.
	 */
	if (!bcmp(hdr->ar_name, AR_EFMT1, sizeof(AR_EFMT1) - 1)) {
		chdr.lname = len = atoi(hdr->ar_name + sizeof(AR_EFMT1) - 1);
		if (len <= 0 || len > MAXNAMLEN)
			badfmt();
		nr = read(fd, chdr.name, len);
		if (nr != len) {
			if (nr < 0)
				error(archive);
			badfmt();
		}
		chdr.name[len] = 0;
		chdr.size -= len;
	} else {
		chdr.lname = 0;
		memmove(chdr.name, hdr->ar_name, sizeof(hdr->ar_name));

		/* Strip trailing spaces, null terminate. */
		for (p = chdr.name + sizeof(hdr->ar_name) - 1; *p == ' '; --p);
		*++p = '\0';
	}
	return (1);
}

static size_t already_written;

/*
 * put_arobj --
 *	Write an archive member to a file.
 */
void
put_arobj(cfp, sb)
	CF *cfp;
	struct stat *sb;
{
	size_t lname;
	char *name;
	struct ar_hdr *hdr;
	off_t size;
	long int tv_sec;

	/*
	 * If passed an sb structure, reading a file from disk.  Get stat(2)
	 * information, build a name and construct a header.  (Files are named
	 * by their last component in the archive.)  If not, then just write
	 * the last header read.
	 */
	if (sb) {
		name = rname(cfp->rname);
		(void)fstat(cfp->rfd, sb);

		/*
		 * The environment variable ZERO_AR_DATE is used here and other
		 * places that write archives to allow testing and comparing
		 * things for exact binary equality.
		 */
		if (getenv("ZERO_AR_DATE") == NULL)
			/* cctools-port: sb->st_mtimespec.tv_sec -> sb->st_mtime */
			tv_sec = (long int)sb->st_mtime;
		else
			tv_sec = (long int)0;

		/*
		 * If not truncating names and the name is too long or contains
		 * a space, use extended format 1.
		 */
		lname = strlen(name);
		if (options & AR_TR) {
			if (lname > OLDARMAXNAME) {
				(void)fflush(stdout);
				warnx("warning: %s truncated to %.*s",
				    name, OLDARMAXNAME, name);
				(void)fflush(stderr);
			}
			(void)sprintf(hb, HDR3, name, (long int)tv_sec,
			    (unsigned int)(u_short)sb->st_uid,
			    (unsigned int)(u_short)sb->st_gid,
			    /* cctools-port: int64_t cast */
			    sb->st_mode, (int64_t)sb->st_size, ARFMAG);
			lname = 0;
		} else if (lname > sizeof(hdr->ar_name) || strchr(name, ' '))
			(void)sprintf(hb, HDR1, AR_EFMT1,
			    (int)((lname + 3) & ~3),
			    (long int)tv_sec,
			    (unsigned int)(u_short)sb->st_uid,
			    (unsigned int)(u_short)sb->st_gid,
			    /* cctools-port: int64_t casts */
			    sb->st_mode, (int64_t)sb->st_size + (int64_t)((lname + 3) & ~3),
			    ARFMAG);
		else {
			lname = 0;
			(void)sprintf(hb, HDR2, name, (long int)tv_sec,
			    (unsigned int)(u_short)sb->st_uid,
			    (unsigned int)(u_short)sb->st_gid,
			    /* cctools-port: int64_t cast */
			    sb->st_mode, (int64_t)sb->st_size, ARFMAG);
		}
		size = sb->st_size;
	} else {
		lname = chdr.lname;
		name = chdr.name;
		size = chdr.size;
	}

	/* cctools-port */
	if (strlen(hb) != sizeof(HDR)) {
		fprintf(stderr, "ar is not working correctly. "
			"Please report this issue to the cctools-port "
			"project. Thank you.\n");
		exit(1);
	}
	/* cctools-port end */

	if (write(cfp->wfd, hb, sizeof(HDR)) != sizeof(HDR))
		error(cfp->wname);
	/*
	 * For Rhapsody if long names are used then the name is padded with
	 * '\0's to a 4 byte size.  This keeps members on 4-byte boundaries
	 * which is required for object files in archives.
	 */
	if (lname) {
		if (write(cfp->wfd, name, lname) != (ssize_t)lname)
			error(cfp->wname);
		already_written = lname;
		if ((lname % 4) != 0) {
			static char pad[3] = "\0\0\0";
			if (write(cfp->wfd, pad, 4-(lname%4)) !=
			    (ssize_t)(4-(lname%4)))
				error(cfp->wname);
			already_written += 4 - (lname % 4);
		}
	}
	copy_ar(cfp, size);
	already_written = 0;
}

/*
 * copy_ar --
 *	Copy size bytes from one file to another - taking care to handle the
 *	extra byte (for odd size files) when reading archives and writing an
 *	extra byte if necessary when adding files to archive.  The length of
 *	the object is the long name plus the object itself; the variable
 *	already_written gets set if a long name was written.
 *
 *	The padding is really unnecessary, and is almost certainly a remnant
 *	of early archive formats where the header included binary data which
 *	a PDP-11 required to start on an even byte boundary.  (Or, perhaps,
 *	because 16-bit word addressed copies were faster?)  Anyhow, it should
 *	have been ripped out long ago.
 */
void
copy_ar(cfp, size)
	CF *cfp;
	off_t size;
{
	static char pad = '\n';
	off_t sz;
	ssize_t nr, nw;
	int from, off, to;
	char buf[8*1024];

	nr = 0;
	
	if (!(sz = size))
		return;

	from = cfp->rfd;
	to = cfp->wfd;
	sz = size;
	while (sz && (nr = read(from, buf, MIN(sz, sizeof(buf)))) > 0) {
		sz -= nr;
		for (off = 0; off < nr; nr -= off, off += nw)
			if ((nw = write(to, buf + off, nr)) < 0)
				error(cfp->wname);
	}
	if (sz) {
		if (nr == 0)
			badfmt();
		error(cfp->rname);
	}

	if (cfp->flags & RPAD && (size + chdr.lname) & 1 &&
	    (nr = read(from, buf, 1)) != 1) {
		if (nr == 0)
			badfmt();
		error(cfp->rname);
	}
	if (cfp->flags & WPAD && (size + already_written) & 1 &&
	    write(to, &pad, 1) != 1)
		error(cfp->wname);
}

/*
 * skip_arobj -
 *	Skip over an object -- taking care to skip the pad bytes.
 */
void
skip_arobj(fd)
	int fd;
{
	off_t len;

	len = chdr.size + ((chdr.size + chdr.lname) & 1);
	if (lseek(fd, len, SEEK_CUR) == (off_t)-1)
		error(archive);
}
