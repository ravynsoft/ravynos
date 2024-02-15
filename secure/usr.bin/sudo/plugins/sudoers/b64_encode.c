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

static const unsigned char base64enc_tab[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t
base64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_len)
{
    size_t ii, io;
    unsigned int rem, v;
    debug_decl(base64_encode, SUDOERS_DEBUG_MATCH);

    for (io = 0, ii = 0, v = 0, rem = 0; ii < in_len; ii++) {
	unsigned char ch = in[ii];
	v = (v << 8) | ch;
	rem += 8;
	while (rem >= 6) {
	    rem -= 6;
	    if (io >= out_len)
		debug_return_size_t((size_t)-1); /* truncation is failure */
	    out[io++] = (char)base64enc_tab[(v >> rem) & 63];
	}
    }
    if (rem != 0) {
	v <<= (6 - rem);
	if (io >= out_len)
	    debug_return_size_t((size_t)-1); /* truncation is failure */
	out[io++] = (char)base64enc_tab[v&63];
    }
    while (io & 3) {
	if (io >= out_len)
	    debug_return_size_t((size_t)-1); /* truncation is failure */
	out[io++] = '=';
    }
    if (io >= out_len)
	debug_return_size_t((size_t)-1); /* no room for NUL terminator */
    out[io] = '\0';
    debug_return_size_t(io);
}
