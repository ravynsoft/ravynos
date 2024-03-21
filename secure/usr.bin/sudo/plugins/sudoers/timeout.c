/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2017 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include <sudo_compat.h>
#include <sudoers_debug.h>
#include <parse.h>

/*
 * Parse a command timeout in sudoers in the format 1d2h3m4s
 * (days, hours, minutes, seconds) or a number of seconds with no suffix.
 * Returns the number of seconds or -1 on error.
 */
int
parse_timeout(const char *timestr)
{
    debug_decl(parse_timeout, SUDOERS_DEBUG_PARSER);
    const char suffixes[] = "dhms";
    const char *cp = timestr;
    int timeout = 0;
    int idx = 0;

    do {
	char *ep;
	int ch;
	long l;

	/* Parse number, must be present and positive. */
	errno = 0;
	l = strtol(cp, &ep, 10);
	if (ep == cp) {
	    /* missing timeout */
	    errno = EINVAL;
	    debug_return_int(-1);
	}
	if (errno == ERANGE || l < 0 || l > INT_MAX)
	    goto overflow;

	/* Find a matching suffix or return an error. */
	if (*ep != '\0') {
	    ch = tolower((unsigned char)*ep++);
	    while (suffixes[idx] != ch) {
		if (suffixes[idx] == '\0') {
		    /* parse error */
		    errno = EINVAL;
		    debug_return_int(-1);
		}
		idx++;
	    }

	    /* Apply suffix. */
	    switch (ch) {
	    case 'd':
		if (l > INT_MAX / (24 * 60 * 60))
		    goto overflow;
		l *= 24 * 60 * 60;
		break;
	    case 'h':
		if (l > INT_MAX / (60 * 60))
		    goto overflow;
		l *= 60 * 60;
		break;
	    case 'm':
		if (l > INT_MAX / 60)
		    goto overflow;
		l *= 60;
		break;
	    }
	}
	cp = ep;

	if (l > INT_MAX - timeout)
	    goto overflow;
	timeout += (int)l;
    } while (*cp != '\0');

    debug_return_int(timeout);
overflow:
    errno = ERANGE;
    debug_return_int(-1);
}
