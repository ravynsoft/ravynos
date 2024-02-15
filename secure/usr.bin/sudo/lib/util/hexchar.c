/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015, 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

/*
 * Converts a two-byte hex string to decimal.
 * Returns a value 0-255 on success or -1 for invalid input.
 */
int
sudo_hexchar_v1(const char *s)
{
    unsigned char result[2];
    unsigned int i;
    debug_decl(sudo_hexchar, SUDO_DEBUG_UTIL);

    for (i = 0; i < 2; i++) {
	switch (s[i]) {
	case '0':
	    result[i] = 0;
	    break;
	case '1':
	    result[i] = 1;
	    break;
	case '2':
	    result[i] = 2;
	    break;
	case '3':
	    result[i] = 3;
	    break;
	case '4':
	    result[i] = 4;
	    break;
	case '5':
	    result[i] = 5;
	    break;
	case '6':
	    result[i] = 6;
	    break;
	case '7':
	    result[i] = 7;
	    break;
	case '8':
	    result[i] = 8;
	    break;
	case '9':
	    result[i] = 9;
	    break;
	case 'A':
	case 'a':
	    result[i] = 10;
	    break;
	case 'B':
	case 'b':
	    result[i] = 11;
	    break;
	case 'C':
	case 'c':
	    result[i] = 12;
	    break;
	case 'D':
	case 'd':
	    result[i] = 13;
	    break;
	case 'E':
	case 'e':
	    result[i] = 14;
	    break;
	case 'F':
	case 'f':
	    result[i] = 15;
	    break;
	default:
	    /* Invalid input. */
	    debug_return_int(-1);
	}
    }
    debug_return_int((result[0] << 4) | result[1]);
}
