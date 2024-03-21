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

static sw_result reply(
    AVAHI_GCC_UNUSED sw_discovery discovery,
    AVAHI_GCC_UNUSED sw_discovery_oid oid,
    sw_discovery_browse_status status,
    AVAHI_GCC_UNUSED sw_uint32 interface_index,
    AVAHI_GCC_UNUSED sw_const_string name,
    AVAHI_GCC_UNUSED sw_const_string type,
    sw_const_string domain,
    AVAHI_GCC_UNUSED sw_opaque extra) {

    switch (status) {
        case SW_DISCOVERY_BROWSE_ADD_DOMAIN:
            fprintf(stderr, "new domain %s\n", domain);
            break;

        case SW_DISCOVERY_BROWSE_REMOVE_DOMAIN:
            fprintf(stderr, "removed domain %s\n", domain);
            break;

        case SW_DISCOVERY_BROWSE_INVALID:
            fprintf(stderr, "some kind of failure happened: %s\n", domain);
            break;

        default:
            abort();
    }

    return SW_OKAY;
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    sw_discovery discovery;
    sw_discovery_oid oid;

    ASSERT_SW_OKAY(sw_discovery_init(&discovery));

    ASSERT_SW_OKAY(sw_discovery_browse_domains(discovery, 0, reply, NULL, &oid));

    ASSERT_SW_OKAY(sw_discovery_run(discovery));

    return 0;
}
