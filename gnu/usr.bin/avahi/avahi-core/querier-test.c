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

#include <stdlib.h>
#include <assert.h>

#include <avahi-common/malloc.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/alternative.h>
#include <avahi-common/timeval.h>

#include <avahi-core/core.h>
#include <avahi-core/log.h>
#include <avahi-core/publish.h>
#include <avahi-core/lookup.h>

#define DOMAIN NULL
#define SERVICE_TYPE "_http._tcp"

static AvahiSServiceBrowser *service_browser1 = NULL, *service_browser2 = NULL;
static const AvahiPoll * poll_api = NULL;
static AvahiServer *server = NULL;
static AvahiSimplePoll *simple_poll;

static const char *browser_event_to_string(AvahiBrowserEvent event) {
    switch (event) {
        case AVAHI_BROWSER_NEW : return "NEW";
        case AVAHI_BROWSER_REMOVE : return "REMOVE";
        case AVAHI_BROWSER_CACHE_EXHAUSTED : return "CACHE_EXHAUSTED";
        case AVAHI_BROWSER_ALL_FOR_NOW : return "ALL_FOR_NOW";
        case AVAHI_BROWSER_FAILURE : return "FAILURE";
    }

    abort();
}

static void sb_callback(
    AvahiSServiceBrowser *b,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *service_type,
    const char *domain,
    AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {
    avahi_log_debug("SB%i: (%i.%s) <%s> as <%s> in <%s> [%s] cached=%i", b == service_browser1 ? 1 : 2, iface, avahi_proto_to_string(protocol), name, service_type, domain, browser_event_to_string(event), !!(flags & AVAHI_LOOKUP_RESULT_CACHED));
}

static void create_second_service_browser(AvahiTimeout *timeout, AVAHI_GCC_UNUSED void* userdata) {

    service_browser2 = avahi_s_service_browser_new(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, SERVICE_TYPE, DOMAIN, 0, sb_callback, NULL);
    assert(service_browser2);

    poll_api->timeout_free(timeout);
}

static void quit(AVAHI_GCC_UNUSED AvahiTimeout *timeout, AVAHI_GCC_UNUSED void *userdata) {
    avahi_simple_poll_quit(simple_poll);
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    struct timeval tv;
    AvahiServerConfig config;

    simple_poll = avahi_simple_poll_new();
    assert(simple_poll);

    poll_api = avahi_simple_poll_get(simple_poll);
    assert(poll_api);

    avahi_server_config_init(&config);
    config.publish_hinfo = 0;
    config.publish_addresses = 0;
    config.publish_workstation = 0;
    config.publish_domain = 0;

    avahi_address_parse("192.168.50.1", AVAHI_PROTO_UNSPEC, &config.wide_area_servers[0]);
    config.n_wide_area_servers = 1;
    config.enable_wide_area = 1;

    server = avahi_server_new(poll_api, &config, NULL, NULL, NULL);
    assert(server);
    avahi_server_config_free(&config);

    service_browser1 = avahi_s_service_browser_new(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, SERVICE_TYPE, DOMAIN, 0, sb_callback, NULL);
    assert(service_browser1);

    poll_api->timeout_new(poll_api, avahi_elapse_time(&tv, 10000, 0), create_second_service_browser, NULL);

    poll_api->timeout_new(poll_api, avahi_elapse_time(&tv, 60000, 0), quit, NULL);


    for (;;)
        if (avahi_simple_poll_iterate(simple_poll, -1) != 0)
            break;

    avahi_server_free(server);
    avahi_simple_poll_free(simple_poll);

    return 0;
}
