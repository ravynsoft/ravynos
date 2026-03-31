/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999, 2009-2011, 2013-2015, 2017
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <time.h>

#include <sudoers.h>

/*
 * Return a static buffer with the current date + time.
 */
char *
get_timestr(time_t tstamp, int log_year)
{
    static char buf[128];
    struct tm tm;
    size_t len;

    if (localtime_r(&tstamp, &tm) != NULL) {
	/* strftime() does not guarantee to NUL-terminate so we must check. */
	buf[sizeof(buf) - 1] = '\0';
	len = strftime(buf, sizeof(buf), log_year ? "%h %e %T %Y" : "%h %e %T",
	    &tm);
	if (len != 0 && buf[sizeof(buf) - 1] == '\0')
	    return buf;
    }
    return NULL;
}
