/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_iolog.h>
#include <sudo_util.h>

/*
 * Expand any escape sequences in inpath, returning the expanded path.
 */
bool
expand_iolog_path(const char *inpath, char *path, size_t pathlen,
    const struct iolog_path_escape *escapes, void *closure)
{
    char *dst, *pathend, tmpbuf[PATH_MAX];
    const char *endbrace, *src;
    bool strfit = false;
    size_t len;
    debug_decl(expand_iolog_path, SUDO_DEBUG_UTIL);

    /* Collapse multiple leading slashes. */
    while (inpath[0] == '/' && inpath[1] == '/')
	inpath++;

    pathend = path + pathlen;
    for (src = inpath, dst = path; *src != '\0'; src++) {
	if (src[0] == '%') {
	    if (src[1] == '{') {
		endbrace = strchr(src + 2, '}');
		if (endbrace != NULL) {
		    const struct iolog_path_escape *esc;
		    len = (size_t)(endbrace - src - 2);
		    for (esc = escapes; esc->name != NULL; esc++) {
			if (strncmp(src + 2, esc->name, len) == 0 &&
			    esc->name[len] == '\0')
			    break;
		    }
		    if (esc->name != NULL) {
			len = esc->copy_fn(dst, (size_t)(pathend - dst),
			    closure);
			if (len >= (size_t)(pathend - dst))
			    goto bad;
			dst += len;
			src = endbrace;
			continue;
		    }
		}
	    } else if (src[1] == '%') {
		/* Collapse %% -> % */
		src++;
	    } else {
		/* May need strftime() */
		strfit = true;
	    }
	}
	/* Need at least 2 chars, including the NUL terminator. */
	if (dst + 1 >= pathend)
	    goto bad;
	*dst++ = *src;
    }

    /* Trim trailing slashes and NUL terminate. */
    while (dst > path && dst[-1] == '/')
	dst--;
    *dst = '\0';

    /* Expand strftime escapes as needed. */
    if (strfit) {
	struct tm tm;
	time_t now;

	time(&now);
	if (localtime_r(&now, &tm) == NULL)
	    goto bad;

	/* We only call strftime() on the current part of the buffer. */
	tmpbuf[sizeof(tmpbuf) - 1] = '\0';
	len = strftime(tmpbuf, sizeof(tmpbuf), path, &tm);

	if (len == 0 || tmpbuf[sizeof(tmpbuf) - 1] != '\0')
	    goto bad;		/* strftime() failed, buf too small? */

	if (len >= (size_t)(pathend - path))
	    goto bad;		/* expanded buffer too big to fit. */
	memcpy(path, tmpbuf, len);
	dst = path + len;
	*dst = '\0';
    }

    debug_return_bool(true);
bad:
    debug_return_bool(false);
}
