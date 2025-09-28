/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

/*
 * Like strtok_r but non-destructive and works w/o a NUL terminator.
 * TODO: Optimize by storing current char in a variable ch
 */
const char *
sudo_strsplit_v1(const char *str, const char *endstr, const char *sep, const char **last)
{
    const char *cp, *s;
    debug_decl(sudo_strsplit, SUDO_DEBUG_UTIL);

    /* If no str specified, use last ptr (if any). */
    if (str == NULL)
	str = *last;

    /* Skip leading separator characters. */
    while (str < endstr) {
	for (s = sep; *s != '\0'; s++) {
	    if (*str == *s) {
		str++;
		break;
	    }
	}
	if (*s == '\0')
	    break;
    }

    /* Empty string? */
    if (str >= endstr) {
	*last = endstr;
	debug_return_ptr(NULL);
    }

    /* Scan str until we hit a char from sep. */
    for (cp = str; cp < endstr; cp++) {
	for (s = sep; *s != '\0'; s++) {
	    if (*cp == *s)
		break;
	}
	if (*s != '\0')
	    break;
    }
    *last = cp;
    debug_return_const_ptr(str);
}
