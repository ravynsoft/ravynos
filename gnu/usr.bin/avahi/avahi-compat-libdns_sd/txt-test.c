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

#include <sys/types.h>
#include <assert.h>
#include <stdio.h>

#include <avahi-common/gccmacro.h>
#include <dns_sd.h>

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
    const char *r;
    TXTRecordRef ref;
    uint8_t l;
    const void *p;
    char k[256];

    TXTRecordCreate(&ref, 0, NULL);

    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "foo", 7, "lennart");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "waldo", 5, "rocks");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "quux", 9, "the_house");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "yeah", 0, NULL);
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "waldo", 6, "rocked");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordRemoveValue(&ref, "foo");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordRemoveValue(&ref, "waldo");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "kawumm", 6, "bloerb");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "one", 1, "1");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "two", 1, "2");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    TXTRecordSetValue(&ref, "three", 1, "3");
    hexdump(TXTRecordGetBytesPtr(&ref), TXTRecordGetLength(&ref));

    assert(TXTRecordContainsKey(TXTRecordGetLength(&ref), TXTRecordGetBytesPtr(&ref), "two"));
    assert(!TXTRecordContainsKey(TXTRecordGetLength(&ref), TXTRecordGetBytesPtr(&ref), "four"));

    r = TXTRecordGetValuePtr(TXTRecordGetLength(&ref), TXTRecordGetBytesPtr(&ref), "kawumm", &l);

    hexdump(r, l);

    assert(TXTRecordGetCount(TXTRecordGetLength(&ref), TXTRecordGetBytesPtr(&ref)) == 6);

    TXTRecordGetItemAtIndex(TXTRecordGetLength(&ref), TXTRecordGetBytesPtr(&ref), 2, sizeof(k), k, &l, &p);

    fprintf(stderr, "key=<%s>\n", k);

    hexdump(p, l);

    assert(TXTRecordGetItemAtIndex(TXTRecordGetLength(&ref), TXTRecordGetBytesPtr(&ref), 20, sizeof(k), k, &l, &p) == kDNSServiceErr_Invalid);

    TXTRecordDeallocate(&ref);
}
