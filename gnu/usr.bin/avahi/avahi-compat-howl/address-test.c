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

#define ASSERT_SW_OKAY(t) { sw_result r; r = (t); assert(r == SW_OKAY); }
#define ASSERT_NOT_NULL(t) { const void* r; r = (t); assert(r); }

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    sw_ipv4_address a;
    char t[256];
    uint8_t a1, a2, a3, a4;

    ASSERT_SW_OKAY(sw_ipv4_address_init_from_name(&a, "heise.de"));
    ASSERT_NOT_NULL(sw_ipv4_address_name(a, t, sizeof(t)));
    printf("%s\n", t);

    ASSERT_SW_OKAY(sw_ipv4_address_init_from_this_host(&a));
    ASSERT_NOT_NULL(sw_ipv4_address_name(a, t, sizeof(t)));
    printf("%s\n", t);

    ASSERT_SW_OKAY(sw_ipv4_address_decompose(a, &a1, &a2, &a3, &a4));
    printf("%i.%i.%i.%i\n", a1, a2, a3, a4);

    return 0;
}
