/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sudoers.h>

/*
 * Derived from code with the following declaration:
 *	PUBLIC DOMAIN - Jon Mayo - November 13, 2003 
 */

static const unsigned char base64dec_tab[256]= {
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
    255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
    255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

/*
 * Decode a NUL-terminated string in base64 format and store the
 * result in dst.
 */
size_t
base64_decode(const char *in, unsigned char *out, size_t out_size)
{
    unsigned char *out_end = out + out_size;
    const unsigned char *out0 = out;
    unsigned int rem, v;
    debug_decl(base64_decode, SUDOERS_DEBUG_MATCH);

    for (v = 0, rem = 0; *in != '\0' && *in != '='; in++) {
	unsigned char ch = base64dec_tab[(unsigned char)*in];
	if (ch == 255)
	    debug_return_size_t((size_t)-1);
	v = (v << 6) | ch;
	rem += 6;
	if (rem >= 8) {
	    rem -= 8;
	    if (out >= out_end)
		debug_return_size_t((size_t)-1);
	    *out++ = (v >> rem) & 0xff;
	}
    }
    if (rem >= 8) {
	    if (out >= out_end)
		debug_return_size_t((size_t)-1);
	    *out++ = (v >> rem) & 0xff;
    }
    debug_return_size_t((size_t)(out - out0));
}
