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

#ifndef HAVE_PIPE2

#include <fcntl.h>
#include <unistd.h>

#include <sudo_compat.h>

int
sudo_pipe2(int fildes[2], int flags)
{
    if (pipe(fildes) != 0)
	return -1;

    if (ISSET(flags, O_CLOEXEC)) {
	if (fcntl(fildes[0], F_SETFD, FD_CLOEXEC) == -1)
	    goto bad;
	if (fcntl(fildes[1], F_SETFD, FD_CLOEXEC) == -1)
	    goto bad;
    }
    if (ISSET(flags, O_NONBLOCK)) {
	int oflags = fcntl(fildes[0], F_GETFL, 0);
	if (oflags == -1)
	    goto bad;
	if (fcntl(fildes[0], F_SETFL, oflags | O_NONBLOCK) == -1)
	    goto bad;
	oflags = fcntl(fildes[1], F_GETFL, 0);
	if (oflags == -1)
	    goto bad;
	if (fcntl(fildes[1], F_SETFL, oflags | O_NONBLOCK) == -1)
	    goto bad;
    }
    return 0;
bad:
    close(fildes[0]);
    close(fildes[1]);
    return -1;
}

#endif /* HAVE_PIPE2 */
