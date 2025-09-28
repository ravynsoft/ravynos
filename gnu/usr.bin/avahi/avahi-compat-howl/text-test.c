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

#include <assert.h>
#include <stdio.h>

#include <avahi-common/gccmacro.h>
#include "howl.h"

#define ASSERT_SW_OKAY(t) { sw_result _r; _r = (t); assert(_r == SW_OKAY); }
#define ASSERT_NOT_NULL(t) { const void* _r; r = (t); assert(_r); }

static void hexdump(const void* p, size_t size) {
    const uint8_t *c = p;
    assert(p);

    printf("Dumping %zu bytes from %p:\n", size, p);

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

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    sw_text_record r;
    sw_text_record_iterator it;
    char key[255];
    uint8_t val[255];
    sw_ulong val_len;

    ASSERT_SW_OKAY(sw_text_record_init(&r));
    ASSERT_SW_OKAY(sw_text_record_add_string(r, "foo=bar"));
    ASSERT_SW_OKAY(sw_text_record_add_string(r, "waldo=baz"));
    ASSERT_SW_OKAY(sw_text_record_add_key_and_string_value(r, "quux", "nimpf"));
    ASSERT_SW_OKAY(sw_text_record_add_key_and_binary_value(r, "quux", (void*) "\0\0\0\0", 4));

    hexdump(sw_text_record_bytes(r), sw_text_record_len(r));

    ASSERT_SW_OKAY(sw_text_record_iterator_init(&it, sw_text_record_bytes(r), sw_text_record_len(r)));

    while (sw_text_record_iterator_next(it, key, val, &val_len) == SW_OKAY) {
        printf("key=%s\n", key);
        hexdump(val, val_len);
    }

    ASSERT_SW_OKAY(sw_text_record_iterator_fina(it));




    ASSERT_SW_OKAY(sw_text_record_fina(r));

    return 0;
}
