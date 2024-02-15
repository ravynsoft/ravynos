/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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
#endif
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_iolog.h>

/*
 * Map IOFD_* -> name.
 */
const char *
iolog_fd_to_name(int iofd)
{
    const char *ret;
    debug_decl(iolog_fd_to_name, SUDO_DEBUG_UTIL);

    switch (iofd) {
    case IOFD_STDIN:
	ret = "stdin";
	break;
    case IOFD_STDOUT:
	ret = "stdout";
	break;
    case IOFD_STDERR:
	ret = "stderr";
	break;
    case IOFD_TTYIN:
	ret = "ttyin";
	break;
    case IOFD_TTYOUT:
	ret = "ttyout";
	break;
    case IOFD_TIMING:
	ret = "timing";
	break;
    default:
	ret = "unknown";
	sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: unexpected iofd %d",
	    __func__, iofd);
	break;
    }
    debug_return_const_str(ret);
}
