/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef HAVE_DUP3

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sudo_compat.h>

int
sudo_dup3(int oldd, int newd, int flags)
{
    int oflags;

    if (oldd == newd) {
	errno = EINVAL;
	return -1;
    }

    if (dup2(oldd, newd) == -1)
	return -1;

    oflags = fcntl(newd, F_GETFL, 0);
    if (oflags == -1)
	goto bad;

    if (ISSET(flags, O_NONBLOCK)) {
	if (!ISSET(oflags, O_NONBLOCK)) {
	    SET(oflags, O_NONBLOCK);
	    if (fcntl(newd, F_SETFL, oflags) == -1)
		goto bad;
	}
    } else {
	if (ISSET(oflags, O_NONBLOCK)) {
	    CLR(oflags, O_NONBLOCK);
	    if (fcntl(newd, F_SETFL, oflags) == -1)
		goto bad;
	}
    }
    if (ISSET(flags, O_CLOEXEC)) {
	if (fcntl(newd, F_SETFD, FD_CLOEXEC) == -1)
	    goto bad;
    }
    return 0;
bad:
    close(newd);
    return -1;
}

#endif /* HAVE_DUP3 */
