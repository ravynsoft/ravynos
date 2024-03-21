#ifndef fooaddresshfoo
#define fooaddresshfoo

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

/** \file address.h Definitions and functions to manipulate IP addresses. */

#include <inttypes.h>
#include <sys/types.h>

#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

/** Protocol family specification, takes the values AVAHI_PROTO_INET, AVAHI_PROTO_INET6, AVAHI_PROTO_UNSPEC */
typedef int AvahiProtocol;

/** Numeric network interface index. Takes OS dependent values and the special constant AVAHI_IF_UNSPEC  */
typedef int AvahiIfIndex;

/** Values for AvahiProtocol */
enum {
    AVAHI_PROTO_INET = 0,     /**< IPv4 */
    AVAHI_PROTO_INET6 = 1,   /**< IPv6 */
    AVAHI_PROTO_UNSPEC = -1  /**< Unspecified/all protocol(s) */
};

/** Special values for AvahiIfIndex */
enum {
    AVAHI_IF_UNSPEC = -1       /**< Unspecified/all interface(s) */
};

/** Maximum size of an address in string form */
#define AVAHI_ADDRESS_STR_MAX 40 /* IPv6 Max = 4*8 + 7 + 1 for NUL */

/** Return TRUE if the specified interface index is valid */
#define AVAHI_IF_VALID(ifindex) (((ifindex) >= 0) || ((ifindex) == AVAHI_IF_UNSPEC))

/** Return TRUE if the specified protocol is valid */
#define AVAHI_PROTO_VALID(protocol) (((protocol) == AVAHI_PROTO_INET) || ((protocol) == AVAHI_PROTO_INET6) || ((protocol) == AVAHI_PROTO_UNSPEC))

/** An IPv4 address */
typedef struct AvahiIPv4Address {
    uint32_t address; /**< Address data in network byte order. */
} AvahiIPv4Address;

/** An IPv6 address */
typedef struct AvahiIPv6Address {
    uint8_t address[16]; /**< Address data */
} AvahiIPv6Address;

/** Protocol (address family) independent address structure */
typedef struct AvahiAddress {
    AvahiProtocol proto; /**< Address family */

    union {
        AvahiIPv6Address ipv6;  /**< Address when IPv6 */
        AvahiIPv4Address ipv4;  /**< Address when IPv4 */
        uint8_t data[1];        /**< Type-independent data field */
    } data;
} AvahiAddress;

/** @{ \name Comparison */

/** Compare two addresses. Returns 0 when equal, a negative value when a < b, a positive value when a > b. */
int avahi_address_cmp(const AvahiAddress *a, const AvahiAddress *b);

/** @} */

/** @{ \name String conversion */

/** Convert the specified address *a to a human readable character string, use AVAHI_ADDRESS_STR_MAX to allocate an array of the right size */
char *avahi_address_snprint(char *ret_s, size_t length, const AvahiAddress *a);

/** Convert the specified human readable character string to an
 * address structure. Set af to AVAHI_UNSPEC for automatic address
 * family detection. */
AvahiAddress *avahi_address_parse(const char *s, AvahiProtocol af, AvahiAddress *ret_addr);

/** @} */

/** \cond fulldocs */
/** Generate the DNS reverse lookup name for an IPv4 or IPv6 address. */
char* avahi_reverse_lookup_name(const AvahiAddress *a, char *ret_s, size_t length);
/** \endcond */

/** @{ \name Protocol/address family handling */

/** Map AVAHI_PROTO_xxx constants to Unix AF_xxx constants */
int avahi_proto_to_af(AvahiProtocol proto);

/** Map Unix AF_xxx constants to AVAHI_PROTO_xxx constants */
AvahiProtocol avahi_af_to_proto(int af);

/** Return a textual representation of the specified protocol number. i.e. "IPv4", "IPv6" or "UNSPEC" */
const char* avahi_proto_to_string(AvahiProtocol proto);

/** @} */

AVAHI_C_DECL_END

#endif
