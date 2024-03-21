/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2012, 2014-2015, 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>		/* for struct winsize on HP-UX */
#include <limits.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

static int
get_ttysize_ioctl(int fd, int *rowp, int *colp)
{
    struct winsize wsize;
    debug_decl(get_ttysize_ioctl, SUDO_DEBUG_UTIL);

    if (fd != -1 && sudo_isatty(fd, NULL)) {
	if (ioctl(fd, TIOCGWINSZ, &wsize) == 0) {
	    if (wsize.ws_row != 0 && wsize.ws_col != 0) {
		*rowp = wsize.ws_row;
		*colp = wsize.ws_col;
		debug_return_int(0);
	    }
	}
    }
    debug_return_int(-1);
}

void
sudo_get_ttysize_v1(int *rowp, int *colp)
{
    sudo_get_ttysize_v2(STDERR_FILENO, rowp, colp);
}

void
sudo_get_ttysize_v2(int fd, int *rowp, int *colp)
{
    debug_decl(sudo_get_ttysize, SUDO_DEBUG_UTIL);

    if (get_ttysize_ioctl(fd, rowp, colp) == -1) {
	char *p;

	/* Fall back on $LINES and $COLUMNS. */
	if ((p = getenv("LINES")) == NULL ||
	    (*rowp = (int)sudo_strtonum(p, 1, INT_MAX, NULL)) <= 0) {
	    *rowp = 24;
	}
	if ((p = getenv("COLUMNS")) == NULL ||
	    (*colp = (int)sudo_strtonum(p, 1, INT_MAX, NULL)) <= 0) {
	    *colp = 80;
	}
    }

    debug_return;
}
