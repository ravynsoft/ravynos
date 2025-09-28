/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <avahi-common/malloc.h>
#include "util.h"

void avahi_hexdump(const void* p, size_t size) {
    const uint8_t *c = p;
    assert(p);

    printf("Dumping %lu bytes from %p:\n", (unsigned long) size, p);

    while (size > 0) {
        unsigned i;

        for (i = 0; i < 16; i++) {
            if (i < size)
                printf("%02x ", c[i]);
            else
                printf("   ");
        }

        for (i = 0; i < 16; i++) {
            if (i < size)
                printf("%c", c[i] >= 32 && c[i] < 127 ? c[i] : '.');
            else
                printf(" ");
        }

        printf("\n");

        c += 16;

        if (size <= 16)
            break;

        size -= 16;
    }
}

char *avahi_format_mac_address(char *r, size_t l, const uint8_t* mac, size_t size) {
    char *t = r;
    unsigned i;
    static const char hex[] = "0123456789abcdef";

    assert(r);
    assert(l > 0);
    assert(mac);

    if (size <= 0) {
        *r = 0;
        return r;
    }

    for (i = 0; i < size; i++) {
        if (l < 3)
            break;

        *(t++) = hex[*mac >> 4];
        *(t++) = hex[*mac & 0xF];
        *(t++) = ':';

        l -= 3;

        mac++;
    }

    if (t > r)
        *(t-1) = 0;
    else
        *r = 0;

    return r;
}

char *avahi_strup(char *s) {
    char *c;
    assert(s);

    for (c = s; *c; c++)
        *c = (char) toupper(*c);

    return s;
}

char *avahi_strdown(char *s) {
    char *c;
    assert(s);

    for (c = s; *c; c++)
        *c = (char) tolower(*c);

    return s;
}
