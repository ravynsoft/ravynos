#ifndef foosockethfoo
#define foosockethfoo

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

#include <inttypes.h>

#include "dns.h"

#define AVAHI_MDNS_PORT 5353
#define AVAHI_DNS_PORT 53
#define AVAHI_IPV4_MCAST_GROUP "224.0.0.251"
#define AVAHI_IPV6_MCAST_GROUP "ff02::fb"

int avahi_open_socket_ipv4(int no_reuse);
int avahi_open_socket_ipv6(int no_reuse);

int avahi_open_unicast_socket_ipv4(void);
int avahi_open_unicast_socket_ipv6(void);

int avahi_send_dns_packet_ipv4(int fd, AvahiIfIndex iface, AvahiDnsPacket *p, const AvahiIPv4Address *src_address, const AvahiIPv4Address *dst_address, uint16_t dst_port);
int avahi_send_dns_packet_ipv6(int fd, AvahiIfIndex iface, AvahiDnsPacket *p, const AvahiIPv6Address *src_address, const AvahiIPv6Address *dst_address, uint16_t dst_port);

AvahiDnsPacket *avahi_recv_dns_packet_ipv4(int fd, AvahiIPv4Address *ret_src_address, uint16_t *ret_src_port, AvahiIPv4Address *ret_dst_address, AvahiIfIndex *ret_iface, uint8_t *ret_ttl);
AvahiDnsPacket *avahi_recv_dns_packet_ipv6(int fd, AvahiIPv6Address *ret_src_address, uint16_t *ret_src_port, AvahiIPv6Address *ret_dst_address, AvahiIfIndex *ret_iface, uint8_t *ret_ttl);

int avahi_mdns_mcast_join_ipv4(int fd, const AvahiIPv4Address *local_address, int iface, int join);
int avahi_mdns_mcast_join_ipv6(int fd, const AvahiIPv6Address *local_address, int iface, int join);

#endif
