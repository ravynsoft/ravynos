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
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#include "addr-util.h"

AvahiAddress *avahi_address_from_sockaddr(const struct sockaddr* sa, AvahiAddress *ret_addr) {
    assert(sa);
    assert(ret_addr);

    assert(sa->sa_family == AF_INET || sa->sa_family == AF_INET6);

    ret_addr->proto = avahi_af_to_proto(sa->sa_family);

    if (sa->sa_family == AF_INET)
        memcpy(&ret_addr->data.ipv4, &((const struct sockaddr_in*) sa)->sin_addr, sizeof(ret_addr->data.ipv4));
    else
        memcpy(&ret_addr->data.ipv6, &((const struct sockaddr_in6*) sa)->sin6_addr, sizeof(ret_addr->data.ipv6));

    return ret_addr;
}

uint16_t avahi_port_from_sockaddr(const struct sockaddr* sa) {
    assert(sa);

    assert(sa->sa_family == AF_INET || sa->sa_family == AF_INET6);

    if (sa->sa_family == AF_INET)
        return ntohs(((const struct sockaddr_in*) sa)->sin_port);
    else
        return ntohs(((const struct sockaddr_in6*) sa)->sin6_port);
}

int avahi_address_is_ipv4_in_ipv6(const AvahiAddress *a) {

    static const uint8_t ipv4_in_ipv6[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF
    };

    assert(a);

    if (a->proto != AVAHI_PROTO_INET6)
        return 0;

    return memcmp(a->data.ipv6.address, ipv4_in_ipv6, sizeof(ipv4_in_ipv6)) == 0;
}

#define IPV4LL_NETWORK 0xA9FE0000L
#define IPV4LL_NETMASK 0xFFFF0000L
#define IPV6LL_NETWORK 0xFE80
#define IPV6LL_NETMASK 0xFFC0

int avahi_address_is_link_local(const AvahiAddress *a) {
    assert(a);

    if (a->proto == AVAHI_PROTO_INET) {
        uint32_t n = ntohl(a->data.ipv4.address);
        return (n & IPV4LL_NETMASK) == IPV4LL_NETWORK;
    }
    else if (a->proto == AVAHI_PROTO_INET6) {
        unsigned n = (a->data.ipv6.address[0] << 8) | (a->data.ipv6.address[1] << 0);
        return (n & IPV6LL_NETMASK) == IPV6LL_NETWORK;
    }

    return 0;
}
