/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2021 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_iolog.h>

/*
 * Read from a (possibly compressed) I/O log file.
 */
ssize_t
iolog_read(struct iolog_file *iol, void *buf, size_t nbytes,
    const char **errstr)
{
    ssize_t nread;
    debug_decl(iolog_read, SUDO_DEBUG_UTIL);

    if (nbytes > UINT_MAX || nbytes > SSIZE_MAX) { // -V590
	errno = EINVAL;
	if (errstr != NULL)
	    *errstr = strerror(errno);
	debug_return_ssize_t(-1);
    }

#ifdef HAVE_ZLIB_H
    if (iol->compressed) {
	if ((nread = gzread(iol->fd.g, buf, (unsigned int)nbytes)) == -1) {
	    if (errstr != NULL) {
		int errnum;
		*errstr = gzerror(iol->fd.g, &errnum);
		if (errnum == Z_ERRNO)
		    *errstr = strerror(errno);
	    }
	}
    } else
#endif
    {
	nread = (ssize_t)fread(buf, 1, nbytes, iol->fd.f);
	if (nread <= 0 && ferror(iol->fd.f)) {
	    nread = -1;
	    if (errstr != NULL)
		*errstr = strerror(errno);
	}
    }
    debug_return_ssize_t(nread);
}
