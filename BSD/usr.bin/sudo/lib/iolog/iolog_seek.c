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
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_iolog.h>

/*
 * I/O log wrapper for fseek/gzseek.
 */
off_t
iolog_seek(struct iolog_file *iol, off_t offset, int whence)
{
    off_t ret;
    //debug_decl(iolog_seek, SUDO_DEBUG_UTIL);

#ifdef HAVE_ZLIB_H
    if (iol->compressed)
	ret = gzseek(iol->fd.g, offset, whence);
    else
#endif
	ret = fseeko(iol->fd.f, offset, whence);

    //debug_return_off_t(ret);
    return ret;
}

/*
 * I/O log wrapper for rewind/gzrewind.
 */
void
iolog_rewind(struct iolog_file *iol)
{
    debug_decl(iolog_rewind, SUDO_DEBUG_UTIL);

#ifdef HAVE_ZLIB_H
    if (iol->compressed)
	(void)gzrewind(iol->fd.g);
    else
#endif
	rewind(iol->fd.f);

    debug_return;
}
