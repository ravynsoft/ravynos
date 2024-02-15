/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2007, 2013-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_debug.h>

/*
 * Read a line of input, honoring line continuation chars.
 * Remove comments and strip off leading and trailing spaces.
 * Returns the line length and updates the buf and bufsize pointers.
 * XXX - just use a struct w/ state, including getdelim buffer?
 *       could also make comment char and line continuation configurable
 */
ssize_t
sudo_parseln_v2(char **bufp, size_t *bufsizep, unsigned int *lineno, FILE *fp, int flags)
{
    ssize_t len, total = 0;
    size_t bufsize, linesize = 0;
    char *cp, *line = NULL;
    bool continued, comment;
    debug_decl(sudo_parseln, SUDO_DEBUG_UTIL);

    do {
	comment = false;
	continued = false;
	len = getdelim(&line, &linesize, '\n', fp);
	if (len == -1)
	    break;
	if (lineno != NULL)
	    (*lineno)++;

	/* Remove trailing newline(s) if present. */
	while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
	    line[--len] = '\0';

	/* Remove comments or check for line continuation (but not both) */
	if ((cp = strchr(line, '#')) != NULL) {
	    if (cp == line || !ISSET(flags, PARSELN_COMM_BOL)) {
		*cp = '\0';
		len = (ssize_t)(cp - line);
		comment = true;
	    }
	}
	if (!comment && !ISSET(flags, PARSELN_CONT_IGN)) {
	    if (len > 0 && line[len - 1] == '\\' && (len == 1 || line[len - 2] != '\\')) {
		line[--len] = '\0';
		continued = true;
	    }
	}

	/* Trim leading and trailing whitespace */
	if (!continued) {
	    while (len > 0 && isblank((unsigned char)line[len - 1]))
		line[--len] = '\0';
	}
	for (cp = line; isblank((unsigned char)*cp); cp++)
	    len--;

	bufsize = (size_t)(total + len + 1);
	if (*bufp == NULL || bufsize > *bufsizep) {
	    const size_t newsize = sudo_pow2_roundup(bufsize);
	    void *newbuf;

	    if (newsize < bufsize) {
		/* overflow */
		errno = ENOMEM;
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable to allocate memory");
		len = -1;
		total = 0;
		break;
	    }
	    if ((newbuf = realloc(*bufp, newsize)) == NULL) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable to allocate memory");
		len = -1;
		total = 0;
		break;
	    }
	    *bufp = newbuf;
	    *bufsizep = newsize;
	}
	memcpy(*bufp + total, cp, (size_t)(len + 1));
	total += len;
    } while (continued);
    free(line);
    if (len == -1 && total == 0)
	debug_return_ssize_t(-1);
    debug_return_ssize_t(total);
}

ssize_t
sudo_parseln_v1(char **bufp, size_t *bufsizep, unsigned int *lineno, FILE *fp)
{
    return sudo_parseln_v2(bufp, bufsizep, lineno, fp, 0);
}
