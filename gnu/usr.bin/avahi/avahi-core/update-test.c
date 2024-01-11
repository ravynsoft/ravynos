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
#include <stdlib.h>

#include <avahi-common/error.h>
#include <avahi-common/watch.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/alternative.h>
#include <avahi-common/timeval.h>

#include <avahi-core/core.h>
#include <avahi-core/log.h>
#include <avahi-core/publish.h>
#include <avahi-core/lookup.h>

static AvahiSEntryGroup *group = NULL;

static void server_callback(AvahiServer *s, AvahiServerState state, AVAHI_GCC_UNUSED void* userdata) {

    avahi_log_debug("server state: %i", state);

    if (state == AVAHI_SERVER_RUNNING) {
        int ret;

        group = avahi_s_entry_group_new(s, NULL, NULL);
        assert(group);

        ret = avahi_server_add_service(s, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, "foo", "_http._tcp", NULL, NULL, 80, "test1", NULL);
        assert(ret == AVAHI_OK);

        avahi_s_entry_group_commit(group);
    }
}

static void modify_txt_callback(AVAHI_GCC_UNUSED AvahiTimeout *e, void *userdata) {
    int ret;
    AvahiServer *s = userdata;

    avahi_log_debug("modifying");

    ret = avahi_server_update_service_txt(s, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, "foo", "_http._tcp", NULL, "test2", NULL);
    assert(ret == AVAHI_OK);
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    AvahiSimplePoll *simple_poll;
    const AvahiPoll *poll_api;
    AvahiServer *server;
    struct timeval tv;
    AvahiServerConfig config;

    simple_poll = avahi_simple_poll_new();
    assert(simple_poll);

    poll_api = avahi_simple_poll_get(simple_poll);
    assert(poll_api);

    avahi_server_config_init(&config);
    config.publish_domain = config.publish_workstation = config.use_ipv6 = config.publish_hinfo = 0;
    server = avahi_server_new(poll_api, &config, server_callback, NULL, NULL);
    assert(server);
    avahi_server_config_free(&config);

    poll_api->timeout_new(poll_api, avahi_elapse_time(&tv, 1000*10, 0), modify_txt_callback, server);

    avahi_simple_poll_loop(simple_poll);
    return 0;
}
