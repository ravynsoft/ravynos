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

#include <stdio.h>
#include <assert.h>

#include "strlst.h"
#include "malloc.h"

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    char *t, *v;
    uint8_t data[1024];
    AvahiStringList *a = NULL, *b, *p;
    size_t size, n;
    int r;

    a = avahi_string_list_new("prefix", "a", "b", NULL);

    a = avahi_string_list_add(a, "start");
    a = avahi_string_list_add(a, "foo=99");
    a = avahi_string_list_add(a, "bar");
    a = avahi_string_list_add(a, "");
    a = avahi_string_list_add(a, "");
    a = avahi_string_list_add(a, "quux");
    a = avahi_string_list_add(a, "");
    a = avahi_string_list_add_arbitrary(a, (const uint8_t*) "null\0null", 9);
    a = avahi_string_list_add_printf(a, "seven=%i %c", 7, 'x');
    a = avahi_string_list_add_pair(a, "blubb", "blaa");
    a = avahi_string_list_add_pair(a, "uxknurz", NULL);
    a = avahi_string_list_add_pair_arbitrary(a, "uxknurz2", (const uint8_t*) "blafasel\0oerks", 14);
    a = avahi_string_list_add(a, "i am a \"string\" with embedded double-quotes (\\\")\nand newlines (\\n).");

    a = avahi_string_list_add(a, "end");

    t = avahi_string_list_to_string(a);
    printf("--%s--\n", t);
    avahi_free(t);

    n = avahi_string_list_serialize(a, NULL, 0);
    size = avahi_string_list_serialize(a, data, sizeof(data));
    assert(size == n);

    printf("%zu\n", size);

    for (t = (char*) data, n = 0; n < size; n++, t++) {
        if (*t <= 32)
            printf("(%u)", *t);
        else
            printf("%c", *t);
    }

    printf("\n");

    assert(avahi_string_list_parse(data, size, &b) == 0);

    printf("equal: %i\n", avahi_string_list_equal(a, b));

    t = avahi_string_list_to_string(b);
    printf("--%s--\n", t);
    avahi_free(t);

    avahi_string_list_free(b);

    b = avahi_string_list_copy(a);

    assert(avahi_string_list_equal(a, b));

    t = avahi_string_list_to_string(b);
    printf("--%s--\n", t);
    avahi_free(t);

    p = avahi_string_list_find(a, "seven");
    assert(p);

    r = avahi_string_list_get_pair(p, &t, &v, NULL);
    assert(r >= 0);
    assert(t);
    assert(v);

    printf("<%s>=<%s>\n", t, v);
    avahi_free(t);
    avahi_free(v);

    p = avahi_string_list_find(a, "quux");
    assert(p);

    r = avahi_string_list_get_pair(p, &t, &v, NULL);
    assert(r >= 0);
    assert(t);
    assert(!v);

    printf("<%s>=<%s>\n", t, v);
    avahi_free(t);
    avahi_free(v);

    avahi_string_list_free(a);
    avahi_string_list_free(b);

    n = avahi_string_list_serialize(NULL, NULL, 0);
    size = avahi_string_list_serialize(NULL, data, sizeof(data));
    assert(size == 1);
    assert(size == n);

    assert(avahi_string_list_parse(data, size, &a) == 0);
    assert(!a);

    return 0;
}
