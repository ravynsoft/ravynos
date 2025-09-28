#ifndef fooaddrutilhfoo
#define fooaddrutilhfoo

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
#include <sys/socket.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/address.h>

AVAHI_C_DECL_BEGIN

/** Make an address structture of a sockaddr structure */
AvahiAddress *avahi_address_from_sockaddr(const struct sockaddr* sa, AvahiAddress *ret_addr);

/** Return the port number of a sockaddr structure (either IPv4 or IPv6) */
uint16_t avahi_port_from_sockaddr(const struct sockaddr* sa);

/** Check whether the specified IPv6 address is in fact an
 * encapsulated IPv4 address, returns 1 if yes, 0 otherwise */
int avahi_address_is_ipv4_in_ipv6(const AvahiAddress *a);

/** Check whether the specified address is a link-local IPv4 or IPv6 address;
 * returns 1 if yes, 0 otherwise */
int avahi_address_is_link_local(const AvahiAddress *a);

AVAHI_C_DECL_END

#endif
