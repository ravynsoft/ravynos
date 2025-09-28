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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <avahi-common/simple-watch.h>
#include <avahi-core/core.h>

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char*argv[]) {
    AvahiServer *server;
    AvahiServerConfig config;
    int error;
    AvahiSimplePoll *simple_poll;

    simple_poll = avahi_simple_poll_new();

    avahi_server_config_init(&config);
    config.publish_hinfo = 0;
    config.publish_addresses = 0;
    config.publish_workstation = 0;
    config.publish_domain = 0;
    config.use_ipv6 = 0;
    config.enable_reflector = 1;

    server = avahi_server_new(avahi_simple_poll_get(simple_poll), &config, NULL, NULL, &error);
    avahi_server_config_free(&config);

    for (;;)
        if (avahi_simple_poll_iterate(simple_poll, -1) != 0)
            break;

    avahi_server_free(server);
    avahi_simple_poll_free(simple_poll);

    return 0;
}
