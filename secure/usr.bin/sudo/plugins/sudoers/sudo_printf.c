/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2012 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <stdarg.h>
#include <errno.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_plugin.h>
#include <sudo_debug.h>
#include <pathnames.h>

static int
sudo_printf_int(int msg_type, const char * restrict fmt, ...)
{
    FILE *fp = stdout;
    FILE *ttyfp = NULL;
    char sbuf[8192];
    char *buf = sbuf;
    va_list ap;
    int len;

    if (ISSET(msg_type, SUDO_CONV_PREFER_TTY)) {
	/* Try writing to /dev/tty first. */
	ttyfp = fopen(_PATH_TTY, "w");
    }

    switch (msg_type & 0xff) {
    case SUDO_CONV_ERROR_MSG:
	fp = stderr;
	FALLTHROUGH;
    case SUDO_CONV_INFO_MSG:
	/*
	 * We use vsnprintf() instead of vfprintf() here to avoid
	 * problems on systems where the system printf(3) is not
	 * C99-compliant.  We use our own snprintf() on such systems.
	 */
	va_start(ap, fmt);
	len = vsnprintf(sbuf, sizeof(sbuf), fmt, ap);
	va_end(ap);
	if (len < 0 || len >= ssizeof(sbuf)) {
	    /* Try again with a dynamically-sized buffer. */
	    va_start(ap, fmt);
	    len = vasprintf(&buf, fmt, ap);
	    va_end(ap);
	}
	if (len != -1) {
	    if (fwrite(buf, 1, len, ttyfp ? ttyfp : fp) == 0)
		len = -1;
	    if (buf != sbuf)
		free(buf);
	}
	break;
    default:
	len = -1;
	errno = EINVAL;
	break;
    }

    if (ttyfp != NULL)
	fclose(ttyfp);

    return len;
}

sudo_printf_t sudo_printf = sudo_printf_int;
