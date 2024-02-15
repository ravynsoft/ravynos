/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdlib.h>
#include <grp.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_util.h>

/*
 * Parse a comma-separated list of gids into an allocated array of GETGROUPS_T.
 * If a pointer to the base gid is specified, it is stored as the first element
 * in the array.
 * Returns the number of gids in the allocated array.
 */
int
sudo_parse_gids_v1(const char *gidstr, const gid_t *basegid, GETGROUPS_T **gidsp)
{
    int ngids = 0;
    GETGROUPS_T *gids;
    const char *cp = gidstr;
    const char *errstr;
    char *ep;
    debug_decl(sudo_parse_gids, SUDO_DEBUG_UTIL);

    /* Count groups. */
    if (*cp != '\0') {
	ngids++;
	do {
	    if (*cp++ == ',')
		ngids++;
	} while (*cp != '\0');
    }
    /* Base gid is optional. */
    if (basegid != NULL)
	ngids++;
    /* Allocate and fill in array. */
    if (ngids != 0) {
	gids = reallocarray(NULL, (size_t)ngids, sizeof(GETGROUPS_T));
	if (gids == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_int(-1);
	}
	ngids = 0;
	if (basegid != NULL)
	    gids[ngids++] = *basegid;
	cp = gidstr;
	do {
	    gids[ngids] = (GETGROUPS_T) sudo_strtoidx(cp, ",", &ep, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), cp, U_(errstr));
		free(gids);
		debug_return_int(-1);
	    }
	    if (basegid == NULL || gids[ngids] != *basegid)
		ngids++;
	    cp = ep + 1;
	} while (*ep != '\0');
	*gidsp = gids;
    }
    debug_return_int(ngids);
}
