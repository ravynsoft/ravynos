/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <sudoers.h>

bool
parser_vwarnx(const struct sudoers_context *ctx, const char *file, int line,
    int column, bool strict, bool quiet, const char * restrict fmt, va_list ap)
{
    bool ret = true;
    debug_decl(parser_warnx, SUDOERS_DEBUG_DEFAULTS);

    if (strict && sudoers_error_hook != NULL) {
	va_list ap2;

	va_copy(ap2, ap);
	ret = sudoers_error_hook(ctx, file, line, column, fmt, ap2);
	va_end(ap2);
    }

    if (!quiet) {
	int oldlocale;
	char *errstr;

	sudoers_setlocale(SUDOERS_LOCALE_USER, &oldlocale);
	if (vasprintf(&errstr, _(fmt), ap) == -1) {
	    errstr = NULL;
	    ret = false;
	} else if (line > 0) {
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("%s:%d:%d: %s\n"), file,
		line, column, errstr);
	} else {
	    sudo_printf(SUDO_CONV_ERROR_MSG, _("%s: %s\n"), file, errstr);
	}
	sudoers_setlocale(oldlocale, NULL);

	free(errstr);
    }

    debug_return_bool(ret);
}

bool
parser_warnx(const struct sudoers_context *ctx, const char *file, int line,
    int column, bool strict, bool quiet, const char * restrict fmt, ...)
{
    va_list ap;
    bool ret;
    debug_decl(parser_warnx, SUDOERS_DEBUG_DEFAULTS);

    va_start(ap, fmt);
    ret = parser_vwarnx(ctx, file, line, column, strict, quiet, fmt, ap);
    va_end(ap);

    debug_return_bool(ret);
}
