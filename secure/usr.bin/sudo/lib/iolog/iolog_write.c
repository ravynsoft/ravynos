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
 * Write to an I/O log, optionally compressing.
 */
ssize_t
iolog_write(struct iolog_file *iol, const void *buf, size_t len,
    const char **errstr)
{
    ssize_t ret;
    debug_decl(iolog_write, SUDO_DEBUG_UTIL);

    if (len > UINT_MAX || len > SSIZE_MAX) { // -V590
	errno = EINVAL;
	if (errstr != NULL)
	    *errstr = strerror(errno);
	debug_return_ssize_t(-1);
    }

#ifdef HAVE_ZLIB_H
    if (iol->compressed) {
	int errnum;

	ret = gzwrite(iol->fd.g, buf, (unsigned int)len);
	if (ret == 0) {
	    ret = -1;
	    if (errstr != NULL) {
		*errstr = gzerror(iol->fd.g, &errnum);
		if (errnum == Z_ERRNO)
		    *errstr = strerror(errno);
	    }
	    goto done;
	}
	if (iolog_get_flush()) {
	    if (gzflush(iol->fd.g, Z_SYNC_FLUSH) != Z_OK) {
		ret = -1;
		if (errstr != NULL) {
		    *errstr = gzerror(iol->fd.g, &errnum);
		    if (errnum == Z_ERRNO)
			*errstr = strerror(errno);
		}
		goto done;
	    }
	}
    } else
#endif
    {
	ret = (ssize_t)fwrite(buf, 1, len, iol->fd.f);
	if (ret <= 0) {
	    ret = -1;
	    if (errstr != NULL)
		*errstr = strerror(errno);
	    goto done;
	}
	if (iolog_get_flush()) {
	    if (fflush(iol->fd.f) != 0) {
		ret = -1;
		if (errstr != NULL)
		    *errstr = strerror(errno);
		goto done;
	    }
	}
    }

done:
    debug_return_ssize_t(ret);
}
