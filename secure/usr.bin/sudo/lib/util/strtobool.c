/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

int
sudo_strtobool_v1(const char *str)
{
    debug_decl(sudo_strtobool, SUDO_DEBUG_UTIL);

    switch (*str) {
	case '0':
	case '1':
	    if (str[1] == '\0')
		debug_return_int(*str - '0');
	    break;
	case 'y':
	case 'Y':
	    if (strcasecmp(str, "yes") == 0)
		debug_return_int(1);
	    break;
	case 't':
	case 'T':
	    if (strcasecmp(str, "true") == 0)
		debug_return_int(1);
	    break;
	case 'o':
	case 'O':
	    if (strcasecmp(str, "on") == 0)
		debug_return_int(1);
	    if (strcasecmp(str, "off") == 0)
		debug_return_int(0);
	    break;
	case 'n':
	case 'N':
	    if (strcasecmp(str, "no") == 0)
		debug_return_int(0);
	    break;
	case 'f':
	case 'F':
	    if (strcasecmp(str, "false") == 0)
		debug_return_int(0);
	    break;
    }
    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	"invalid boolean value \"%s\"", str);

    debug_return_int(-1);
}
