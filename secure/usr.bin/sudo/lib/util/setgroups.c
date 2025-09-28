/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2012, 2014-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

int
sudo_setgroups_v1(int ngids, const GETGROUPS_T *gids)
{
    long maxgids;
    int ret;
    debug_decl(sudo_setgroups, SUDO_DEBUG_UTIL);

    if (ngids < 0)
	debug_return_int(-1);

    ret = setgroups(ngids, (GETGROUPS_T *)gids);
    if (ret == -1 && errno == EINVAL) {
	/* Too many groups, try again with fewer. */
	maxgids = sysconf(_SC_NGROUPS_MAX);
	if (maxgids == -1)
	    maxgids = NGROUPS_MAX;
	if (ngids > maxgids)
	    ret = setgroups(maxgids, (GETGROUPS_T *)gids);
    }
    debug_return_int(ret);
}
