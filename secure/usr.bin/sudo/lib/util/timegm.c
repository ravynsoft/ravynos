/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2017, 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef HAVE_TIMEGM

#include <stdio.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>

/*
 * Returns the offset from GMT in seconds (algorithm taken from sendmail).
 */
#ifdef HAVE_STRUCT_TM_TM_GMTOFF
static long
get_gmtoff(time_t *when)
{
    struct tm local;

    if (localtime_r(when, &local) == NULL)
	return 0;

    /* Adjust for DST. */
    if (local.tm_isdst != 0)
	local.tm_gmtoff -= local.tm_isdst * 3600;

    return local.tm_gmtoff;
}
#else
static long
get_gmtoff(time_t *when)
{
    struct tm gmt, local;
    long offset;

    if (gmtime_r(when, &gmt) == NULL)
	return 0;
    if (localtime_r(when, &local) == NULL)
	return 0;

    offset = (local.tm_sec - gmt.tm_sec) +
	((local.tm_min - gmt.tm_min) * 60) +
	((local.tm_hour - gmt.tm_hour) * 3600);

    /* Timezone may cause year rollover to happen on a different day. */
    if (local.tm_year < gmt.tm_year)
	offset -= 24 * 3600;
    else if (local.tm_year > gmt.tm_year)
	offset -= 24 * 3600;
    else if (local.tm_yday < gmt.tm_yday)
	offset -= 24 * 3600;
    else if (local.tm_yday > gmt.tm_yday)
	offset += 24 * 3600;

    /* Adjust for DST. */
    if (local.tm_isdst != 0)
	offset -= local.tm_isdst * 3600;

    return offset;
}
#endif /* HAVE_TM_GMTOFF */

time_t
sudo_timegm(struct tm *tm)
{
    time_t result;

    tm->tm_isdst = 0;
    result = mktime(tm);
    if (result != -1)
	result += get_gmtoff(&result);

    return result;
}

#endif /* HAVE_TIMEGM */
