/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2007-2019
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <string.h>
#include <syslog.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

/*
 * For converting between syslog numbers and strings.
 */
struct strmap {
    const char *name;
    int num;
};

static const struct strmap facilities[] = {
#ifdef LOG_AUTHPRIV
	{ "authpriv",	LOG_AUTHPRIV },
#endif
	{ "auth",	LOG_AUTH },
	{ "daemon",	LOG_DAEMON },
	{ "user",	LOG_USER },
	{ "local0",	LOG_LOCAL0 },
	{ "local1",	LOG_LOCAL1 },
	{ "local2",	LOG_LOCAL2 },
	{ "local3",	LOG_LOCAL3 },
	{ "local4",	LOG_LOCAL4 },
	{ "local5",	LOG_LOCAL5 },
	{ "local6",	LOG_LOCAL6 },
	{ "local7",	LOG_LOCAL7 },
	{ NULL,		-1 }
};

bool
sudo_str2logfac_v1(const char *str, int *logfac)
{
    const struct strmap *fac;
    debug_decl(sudo_str2logfac, SUDO_DEBUG_UTIL);

    for (fac = facilities; fac->name != NULL; fac++) {
	if (strcmp(str, fac->name) == 0) {
	    *logfac = fac->num;
	    debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

const char *
sudo_logfac2str_v1(int num)
{
    const struct strmap *fac;
    debug_decl(sudo_logfac2str, SUDO_DEBUG_UTIL);

    for (fac = facilities; fac->name != NULL; fac++) {
	if (fac->num == num)
	    break;
    }
    debug_return_const_str(fac->name);
}
