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
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_iolog.h>

/*
 * Close an I/O log.
 */
bool
iolog_close(struct iolog_file *iol, const char **errstr)
{
    bool ret = true;
    debug_decl(iolog_close, SUDO_DEBUG_UTIL);

#ifdef HAVE_ZLIB_H
    if (iol->compressed) {
	int errnum;

	/* Must check error indicator before closing. */
	if (iol->writable) {
	    if (gzflush(iol->fd.g, Z_SYNC_FLUSH) != Z_OK) {
		ret = false;
		if (errstr != NULL) {
		    *errstr = gzerror(iol->fd.g, &errnum);
		    if (errnum == Z_ERRNO)
			*errstr = strerror(errno);
		}
	    }
	}
	errnum = gzclose(iol->fd.g);
	if (ret && errnum != Z_OK) {
	    ret = false;
	    if (errstr != NULL)
		*errstr = errnum == Z_ERRNO ? strerror(errno) : "unknown error";
	}
    } else
#endif
    if (fclose(iol->fd.f) != 0) {
	ret = false;
	if (errstr != NULL)
	    *errstr = strerror(errno);
    }

    debug_return_bool(ret);
}
