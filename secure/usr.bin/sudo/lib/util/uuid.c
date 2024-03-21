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

#include <stdlib.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <string.h>
#include <arpa/inet.h>

#include <sudo_compat.h>
#include <sudo_util.h>
#include <sudo_rand.h>

struct uuid {
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_hi_and_version;
    uint8_t clock_seq_hi_and_reserved;
    uint8_t clock_seq_low;
    uint8_t node[6];
};

/*
 * Create a type 4 (random), variant 1 universally unique identifier (UUID).
 * As per RFC 4122 section 4.4.
 */
void
sudo_uuid_create_v1(unsigned char uuid_out[16])
{
    struct uuid uuid;

    arc4random_buf(&uuid, sizeof(uuid));

    /* Set version to 4 (random), 4 most significant bits (12-15) are 0010. */
    uuid.time_hi_and_version &= 0x0fff;
    uuid.time_hi_and_version |= 0x4000;

    /* Set variant to 1: two most significant bits (6 and 7) are 01. */
    uuid.clock_seq_hi_and_reserved &= 0x3f;
    uuid.clock_seq_hi_and_reserved |= 0x80;

    memcpy(uuid_out, &uuid, 16);
}

/*
 * Format a uuid as a 36-byte string (plus one for the NUL).
 */
char *
sudo_uuid_to_string_v1(unsigned char uuid[16], char *dst, size_t dstsiz)
{
    const char hex[] = "0123456789abcdef";
    char *cp = dst;
    unsigned int i;

    if (dstsiz < sizeof("123e4567-e89b-12d3-a456-426655440000"))
	return NULL;

    for (i = 0; i < 16; i++) {
	*cp++ = hex[uuid[i] >> 4];
	*cp++ = hex[uuid[i] & 0x0f];

	switch (i) {
	case 4:
	case 6:
	case 8:
	case 10:
	    *cp++ = '-';
	    break;
	}
    }
    *cp = '\0';

    return dst;
}
