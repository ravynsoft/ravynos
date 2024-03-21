/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef HAVE_ARC4RANDOM_BUF

#include <stdlib.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif

#include <sudo_compat.h>
#include <sudo_rand.h>

#define minimum(a, b) ((a) < (b) ? (a) : (b))

/*
 * Call arc4random() repeatedly to fill buf with n bytes of random data.
 */
void
sudo_arc4random_buf(void *buf, size_t n)
{
	char *cp = buf;

	while (n != 0) {
		size_t m = minimum(n, 4);
		uint32_t val = arc4random();

		switch (m) {
		case 4:
			*cp++ = (val >> 24) & 0xff;
			FALLTHROUGH;
		case 3:
			*cp++ = (val >> 16) & 0xff;
			FALLTHROUGH;
		case 2:
			*cp++ = (val >> 8) & 0xff;
			FALLTHROUGH;
		case 1:
			*cp++ = val & 0xff;
			break;
		}
		n -= m;
	}
}

#endif /* HAVE_ARC4RANDOM_BUF */
