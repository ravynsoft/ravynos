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

static void callback(
    AVAHI_GCC_UNUSED AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    AVAHI_GCC_UNUSED const AvahiAddress *a,
    AVAHI_GCC_UNUSED uint16_t port,
    AVAHI_GCC_UNUSED AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void *userdata) {

    fprintf(stderr, "%i name=%s type=%s domain=%s host=%s\n", event, name, type, domain, host_name);
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {

    AvahiSimplePoll *simple_poll;
    const AvahiPoll *poll_api;
    AvahiClient *client;
    AvahiServiceResolver *r;

    simple_poll = avahi_simple_poll_new();
    assert(simple_poll);

    poll_api = avahi_simple_poll_get(simple_poll);
    assert(poll_api);

    client = avahi_client_new(poll_api, 0, NULL, NULL, NULL);
    assert(client);

    r = avahi_service_resolver_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, "_domain._udp", "0pointer.de", AVAHI_PROTO_UNSPEC, AVAHI_LOOKUP_NO_TXT, callback, simple_poll);
    assert(r);

    avahi_simple_poll_loop(simple_poll);

    avahi_client_free(client);
    avahi_simple_poll_free(simple_poll);

    return 0;
}
