/*
 * Copyright (c) 2005-2007 Rob Braun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 03-Apr-2005
 * DRI: Rob Braun <bbraun@synack.net>
 */
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
*/

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef __linux__
#define __USE_XOPEN
#include <time.h>
#endif
#include "config.h"
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include "xar.h"
#include "archive.h"
#include "filetree.h"

#ifndef HAVE_STRMODE
#include "strmode.h"
#endif

uint64_t xar_ntoh64(uint64_t num) {
	int t = 1234;
	union conv {
		uint64_t i64;
		uint32_t i32[2];
	} *in, out;

	if( (int)ntohl(t) == t ) {
		out.i64 = num;
		return out.i64;
	}
	in = (union conv *)&num;
	out.i32[1] = ntohl(in->i32[0]);
	out.i32[0] = ntohl(in->i32[1]);
	return(out.i64);
}

uint32_t xar_swap32(uint32_t num) {
	uint8_t *one, *two;
	uint32_t ret;

	two = (uint8_t *)&ret;
	one = (uint8_t *)&num;
	two[3] = one[0];
	two[2] = one[1];
	two[1] = one[2];
	two[0] = one[3];

	return ret;
}

/* xar_get_path
 * Summary: returns the archive path of the file f.
 * Caller needs to free the return value.
 */
char *xar_get_path(xar_file_t f) {
	char *ret, *tmp;
	const char *name;
	xar_file_t i;

	xar_prop_get(f, "name", &name);
	ret = strdup(name);
	for(i = XAR_FILE(f)->parent; i; i = XAR_FILE(i)->parent) {
		int err;
		const char *name;
	       	xar_prop_get(i, "name", &name);
		tmp = ret;
		err = asprintf(&ret, "%s/%s", name, tmp);
		free(tmp);
	}

	return ret;
}

uint64_t xar_get_heap_offset(xar_t x) {
	return (uint64_t)XAR(x)->toc_count + XAR(x)->header.size;
}

/* xar_read_fd
 * Summary: Reads from a file descriptor a certain number of bytes to a specific
 * buffer.  This simple wrapper just handles certain retryable error situations.
 * Returns -1 when it fails fatally; the number of bytes read otherwise.
 */
ssize_t xar_read_fd( int fd, void * buffer, size_t nbytes ) {
	ssize_t rb;
	ssize_t off = 0;

	while ( (size_t)off < nbytes ) {
		rb = read(fd, ((char *)buffer)+off, nbytes-off);
		if( (rb < 1 ) && (errno != EINTR) && (errno != EAGAIN) )
			return -1;
		off += rb;
	}

	return off;
}

/* xar_write_fd
 * Summary: Writes from a buffer to a file descriptor.  Like xar_read_fd it
 * also just handles certain retryable error situations.
 * Returs -1 when it fails fatally; the number of bytes written otherwise.
 */
ssize_t xar_write_fd( int fd, void * buffer, size_t nbytes ) {
	ssize_t rb;
	ssize_t off = 0;

	while ( (size_t)off < nbytes ) {
		rb = write(fd, ((char *)buffer)+off, nbytes-off);
		if( (rb < 1 ) && (errno != EINTR) && (errno != EAGAIN) )
			return -1;
		off += rb;
	}

	return off;
}

dev_t xar_makedev(uint32_t major, uint32_t minor)
{
#ifdef makedev
	return makedev(major, minor);
#else
	return (major << 8) | minor;
#endif
}

void xar_devmake(dev_t dev, uint32_t *out_major, uint32_t *out_minor)
{
#ifdef major
	*out_major = major(dev);
#else
	*out_major = (dev >> 8) & 0xFF;
#endif
#ifdef minor
	*out_minor = minor(dev);
#else
	*out_minor = dev & 0xFF;
#endif
	return;
}


char *xar_get_type(xar_t x, xar_file_t f) {
	const char *type = NULL;
	(void)x;
	xar_prop_get(f, "type", &type);
	if( type == NULL )
		type = "unknown";
	return strdup(type);
}

char *xar_get_size(xar_t x, xar_file_t f) {
	const char *size = NULL;
	const char *type = NULL;

	xar_prop_get(f, "type", &type);
	if( type != NULL ) {
		if( strcmp(type, "hardlink") == 0 ) {
			const char *link = NULL;
			link = xar_attr_get(f, "type", "link");
			if( link ) {
				if( strcmp(link, "original") != 0 ) {
					xar_iter_t i;
					i = xar_iter_new();
					if( i ) {
						xar_file_t tmpf;
						for(tmpf = xar_file_first(x, i); tmpf; tmpf = xar_file_next(i)) {
							const char *id;
							id = xar_attr_get(tmpf, NULL, "id");
							if( !id ) continue;
							if( strcmp(id, link) == 0 ) {
								f = tmpf;
								break;
							}
						}
					}
					xar_iter_free(i);
				}
			}
		}
	}
	xar_prop_get(f, "data/size", &size);
	if( size == NULL )
		size = "0";
	return strdup(size);
}

char *xar_get_mode(xar_t x, xar_file_t f) {
	const char *mode = NULL;
	const char *type = NULL;
	char *ret;
	mode_t m = 0;
	int gotmode = 0;
	int gottype = 0;

	(void)x;
	xar_prop_get(f, "mode", &mode);
	if (mode) {
		long long strmode;
		errno = 0;
		strmode = strtoll(mode, 0, 8);
		if (!errno) {
			m = (mode_t)strmode;
			gotmode = 1;
		}
	}

	xar_prop_get(f, "type", &type);
	if (type) {
		if( strcmp(type, "file") == 0 )
			m |= S_IFREG;
		else if( strcmp(type, "hardlink") == 0 )
			m |= S_IFREG;
		else if( strcmp(type, "directory") == 0 )
			m |= S_IFDIR;
		else if( strcmp(type, "symlink") == 0 )
			m |= S_IFLNK;
		else if( strcmp(type, "fifo") == 0 )
			m |= S_IFIFO;
		else if( strcmp(type, "character special") == 0 )
			m |= S_IFCHR;
		else if( strcmp(type, "block special") == 0 )
			m |= S_IFBLK;
		else if( strcmp(type, "socket") == 0 )
			m |= S_IFSOCK;
#ifdef S_IFWHT
		else if( strcmp(type, "whiteout") == 0 )
			m |= S_IFWHT;
#endif
		gottype = 1;
	}
	ret = calloc(1, 12);
	if (ret) {
		strmode(m, ret);
		if (!gottype)
			ret[0] = '?';
		if (!gotmode)
			memset(ret+1, '?', 9);
	}

	return ret;
}

char *xar_get_owner(xar_t x, xar_file_t f) {
	const char *user = NULL;

	(void)x;
	xar_prop_get(f, "user", &user);
	if( !user )
		return strdup("unknown");
	return strdup(user);
}

char *xar_get_group(xar_t x, xar_file_t f) {
	const char *group = NULL;

	(void)x;
	xar_prop_get(f, "group", &group);
	if( !group )
		return strdup("unknown");
	return strdup(group);
}

char *xar_get_mtime(xar_t x, xar_file_t f) {
	const char *mtime = NULL;
	char *tmp;
	struct tm tm;

	(void)x;
	xar_prop_get(f, "mtime", &mtime);
	if( !mtime )
		mtime = "1970-01-01T00:00:00Z";

	strptime(mtime, "%Y-%m-%dT%H:%M:%S", &tm);
	tmp = calloc(128,1);
	strftime(tmp, 127, "%Y-%m-%d %H:%M:%S", &tm);
	return tmp;
}
