/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>

#include <sudoers.h>

/*
 * Like strlcpy() but collapses non-space chars escaped with a backslash.
 */
size_t
strlcpy_unescape(char *restrict dst, const char *restrict src, size_t size)
{
    size_t len = 0;
    char ch;
    debug_decl(strlcpy_unescape, SUDOERS_DEBUG_UTIL);

    while ((ch = *src++) != '\0') {
	if (ch == '\\' && *src != '\0' && !isspace((unsigned char)*src))
	    ch = *src++;
	if (size > 1) {
	    *dst++ = ch;
	    size--;
	}
	len++;
    }
    if (size > 0)
	*dst = '\0';

    debug_return_size_t(len);
}
