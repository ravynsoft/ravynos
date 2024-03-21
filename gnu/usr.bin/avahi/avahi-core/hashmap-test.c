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

#include <avahi-common/domain.h>
#include <avahi-common/malloc.h>

#include "hashmap.h"
#include "util.h"

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    unsigned n;
    AvahiHashmap *m;
    const char *t;

    m = avahi_hashmap_new(avahi_string_hash, avahi_string_equal, avahi_free, avahi_free);

    avahi_hashmap_insert(m, avahi_strdup("bla"), avahi_strdup("#1"));
    avahi_hashmap_insert(m, avahi_strdup("bla2"), avahi_strdup("asdf"));
    avahi_hashmap_insert(m, avahi_strdup("gurke"), avahi_strdup("ffsdf"));
    avahi_hashmap_insert(m, avahi_strdup("blubb"), avahi_strdup("sadfsd"));
    avahi_hashmap_insert(m, avahi_strdup("bla"), avahi_strdup("#2"));

    for (n = 0; n < 1000; n ++)
        avahi_hashmap_insert(m, avahi_strdup_printf("key %u", n), avahi_strdup_printf("value %u", n));

    printf("%s\n", (const char*) avahi_hashmap_lookup(m, "bla"));

    avahi_hashmap_replace(m, avahi_strdup("bla"), avahi_strdup("#3"));

    printf("%s\n", (const char*) avahi_hashmap_lookup(m, "bla"));

    avahi_hashmap_remove(m, "bla");

    t = (const char*) avahi_hashmap_lookup(m, "bla");
    printf("%s\n", t ? t : "(null)");

    avahi_hashmap_free(m);

    return 0;
}
