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

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>

static void hexdump(const void* p, size_t size) {
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

static void callback(
    AVAHI_GCC_UNUSED AvahiRecordBrowser *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    uint16_t clazz,
    uint16_t type,
    const void *rdata,
    size_t rdata_size,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void *userdata) {

    fprintf(stderr, "%i name=%s class=%u type=%u\n", event, name, clazz, type);

    if (rdata)
        hexdump(rdata, rdata_size);
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {

    AvahiSimplePoll *simple_poll;
    const AvahiPoll *poll_api;
    AvahiClient *client;
    AvahiRecordBrowser *r;

    simple_poll = avahi_simple_poll_new();
    assert(simple_poll);

    poll_api = avahi_simple_poll_get(simple_poll);
    assert(poll_api);

    client = avahi_client_new(poll_api, 0, NULL, NULL, NULL);
    assert(client);

    r = avahi_record_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "ecstasy.local", AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_HINFO, 0, callback, simple_poll);
    assert(r);

    avahi_simple_poll_loop(simple_poll);

    avahi_client_free(client);
    avahi_simple_poll_free(simple_poll);

    return 0;
}
