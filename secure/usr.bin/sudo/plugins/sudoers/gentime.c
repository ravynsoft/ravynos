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

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudoers_debug.h>
#include <parse.h>

/* Since timegm() is only used in one place we keep the macro local. */
#ifndef HAVE_TIMEGM
# define timegm(_t)	sudo_timegm(_t)
#endif

/*
 * Parse a timestamp in Generalized Time format as per RFC4517.
 * E.g. yyyymmddHHMMSS.FZ or yyyymmddHHMMSS.F[+-]TZOFF
 * where minutes, seconds and fraction are optional.
 * Returns the time in Unix time format or -1 on error.
 */
time_t
parse_gentime(const char *timestr)
{
    char tcopy[sizeof("yyyymmddHHMMSS")];
    const char *cp;
    time_t result;
    struct tm tm;
    size_t len;
    int items, tzoff = 0;
    bool islocal = false;
    debug_decl(parse_gentime, SUDOERS_DEBUG_PARSER);

    /* Make a copy of the non-fractional time without zone for easy parsing. */
    len = strspn(timestr, "0123456789");
    if (len >= sizeof(tcopy) || len < sizeof("yyyymmddHH") -1 || (len & 1)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to parse general time string %s", timestr);
	debug_return_time_t(-1);
    }
    memcpy(tcopy, timestr, len);
    tcopy[len] = '\0';

    /* Parse general time, ignoring the timezone for now. */
    memset(&tm, 0, sizeof(tm));
    items = sscanf(tcopy, "%4d%2d%2d%2d%2d%2d", &tm.tm_year, &tm.tm_mon,
	&tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    if (items == EOF || items < 4) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "only parsed %d items in general time string %s", items, timestr); 
	debug_return_time_t(-1);
    }

    /* Parse optional fractional hours/minute/second if present. */
    cp = timestr + len;
    if ((cp[0] == '.' || cp[0] == ',') && isdigit((unsigned char)cp[1])) {
	int frac = cp[1] - '0';
	switch (items) {
	case 4:
	    /* convert fractional hour -> minutes */
	    tm.tm_min += 60 / 10 * frac;
	    break;
	case 5:
	    /* convert fractional minute -> seconds */
	    tm.tm_sec += 60 / 10 * frac;
	    break;
	case 6:
	    /* ignore fractional second */
	    break;
	}
	cp += 2;	/* skip over radix and fraction */
    }

    /* Parse optional time zone. */
    switch (*cp) {
    case '-':
    case '+': {
	int hour = 0, min = 0;

	/* No DST */
	tm.tm_isdst = 0;
	/* time zone offset must be hh or hhmm */
	len = strspn(cp + 1, "0123456789");
	if (len != 2 && len != 4) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to parse time zone offset in %s, bad tz offset",
		timestr);
	    debug_return_time_t(-1);
	}
	/* parse time zone offset */
	items = sscanf(cp + 1, "%2d%2d", &hour, &min);
	if (items == EOF || items < 1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to parse time zone offset in %s, items %d",
		timestr, items);
	    debug_return_time_t(-1);
	}
	if (*cp == '-')
	    tzoff = -((hour * 60) + min) * 60;
	else
	    tzoff = ((hour * 60) + min) * 60;
	cp += 1 + (items * 2);
	break;
    }
    case 'Z':
	/* GMT/UTC, no DST */
	tm.tm_isdst = 0;
	cp++;
	break;
    case '\0':
	/* no zone specified, use local time */
	tm.tm_isdst = -1;
	islocal = true;
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to parse general time string %s", timestr);
	debug_return_time_t(-1);
    }
    if (*cp != '\0') {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "trailing garbage in general time string %s", timestr);
	debug_return_time_t(-1);
    }

    /* Adjust from Generalized Time to struct tm */
    tm.tm_year -= 1900;
    tm.tm_mon--;

    if (islocal) {
	result = mktime(&tm);
    } else {
	result = timegm(&tm);
	if (result != -1) {
	    /* Adjust time based on supplied GMT offset. */
	    result -= tzoff;
	}
    }

    debug_return_time_t(result);
}
