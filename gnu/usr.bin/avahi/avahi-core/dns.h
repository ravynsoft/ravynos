#ifndef foodnshfoo
#define foodnshfoo

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

#include "rr.h"
#include "hashmap.h"

#define AVAHI_DNS_PACKET_HEADER_SIZE 12
#define AVAHI_DNS_PACKET_EXTRA_SIZE 48
#define AVAHI_DNS_LABELS_MAX 127
#define AVAHI_DNS_RDATA_MAX 0xFFFF
#define AVAHI_DNS_PACKET_SIZE_MAX (AVAHI_DNS_PACKET_HEADER_SIZE + 256 + 2 + 2 + 4 + 2 + AVAHI_DNS_RDATA_MAX)

typedef struct AvahiDnsPacket {
    size_t size, rindex, max_size, res_size;
    AvahiHashmap *name_table; /* for name compression */
    uint8_t *data;
} AvahiDnsPacket;

#define AVAHI_DNS_PACKET_DATA(p) ((p)->data ? (p)->data : ((uint8_t*) p) + sizeof(AvahiDnsPacket))

AvahiDnsPacket* avahi_dns_packet_new(unsigned mtu);
AvahiDnsPacket* avahi_dns_packet_new_query(unsigned mtu);
AvahiDnsPacket* avahi_dns_packet_new_response(unsigned mtu, int aa);

AvahiDnsPacket* avahi_dns_packet_new_reply(AvahiDnsPacket* p, unsigned mtu, int copy_queries, int aa);

void avahi_dns_packet_free(AvahiDnsPacket *p);
void avahi_dns_packet_set_field(AvahiDnsPacket *p, unsigned idx, uint16_t v);
uint16_t avahi_dns_packet_get_field(AvahiDnsPacket *p, unsigned idx);
void avahi_dns_packet_inc_field(AvahiDnsPacket *p, unsigned idx);

uint8_t *avahi_dns_packet_extend(AvahiDnsPacket *p, size_t l);

void avahi_dns_packet_cleanup_name_table(AvahiDnsPacket *p);

uint8_t *avahi_dns_packet_append_uint16(AvahiDnsPacket *p, uint16_t v);
uint8_t *avahi_dns_packet_append_uint32(AvahiDnsPacket *p, uint32_t v);
uint8_t *avahi_dns_packet_append_name(AvahiDnsPacket *p, const char *name);
uint8_t *avahi_dns_packet_append_bytes(AvahiDnsPacket  *p, const void *d, size_t l);
uint8_t* avahi_dns_packet_append_key(AvahiDnsPacket *p, AvahiKey *k, int unicast_response);
uint8_t* avahi_dns_packet_append_record(AvahiDnsPacket *p, AvahiRecord *r, int cache_flush, unsigned max_ttl);
uint8_t* avahi_dns_packet_append_string(AvahiDnsPacket *p, const char *s);

int avahi_dns_packet_is_query(AvahiDnsPacket *p);
int avahi_dns_packet_check_valid(AvahiDnsPacket *p);
int avahi_dns_packet_check_valid_multicast(AvahiDnsPacket *p);

int avahi_dns_packet_consume_uint16(AvahiDnsPacket *p, uint16_t *ret_v);
int avahi_dns_packet_consume_uint32(AvahiDnsPacket *p, uint32_t *ret_v);
int avahi_dns_packet_consume_name(AvahiDnsPacket *p, char *ret_name, size_t l);
int avahi_dns_packet_consume_bytes(AvahiDnsPacket *p, void* ret_data, size_t l);
AvahiKey* avahi_dns_packet_consume_key(AvahiDnsPacket *p, int *ret_unicast_response);
AvahiRecord* avahi_dns_packet_consume_record(AvahiDnsPacket *p, int *ret_cache_flush);
int avahi_dns_packet_consume_string(AvahiDnsPacket *p, char *ret_string, size_t l);

const void* avahi_dns_packet_get_rptr(AvahiDnsPacket *p);

int avahi_dns_packet_skip(AvahiDnsPacket *p, size_t length);

int avahi_dns_packet_is_empty(AvahiDnsPacket *p);
size_t avahi_dns_packet_space(AvahiDnsPacket *p);
size_t avahi_dns_packet_reserve_size(AvahiDnsPacket *p, size_t res_size);
size_t avahi_dns_packet_reserved_space(AvahiDnsPacket *p);

#define AVAHI_DNS_FIELD_ID 0
#define AVAHI_DNS_FIELD_FLAGS 1
#define AVAHI_DNS_FIELD_QDCOUNT 2
#define AVAHI_DNS_FIELD_ANCOUNT 3
#define AVAHI_DNS_FIELD_NSCOUNT 4
#define AVAHI_DNS_FIELD_ARCOUNT 5

#define AVAHI_DNS_FLAG_QR (1 << 15)
#define AVAHI_DNS_FLAG_OPCODE (15 << 11)
#define AVAHI_DNS_FLAG_RCODE (15)
#define AVAHI_DNS_FLAG_TC (1 << 9)
#define AVAHI_DNS_FLAG_AA (1 << 10)

#define AVAHI_DNS_FLAGS(qr, opcode, aa, tc, rd, ra, z, ad, cd, rcode) \
        (((uint16_t) !!qr << 15) |  \
         ((uint16_t) (opcode & 15) << 11) | \
         ((uint16_t) !!aa << 10) | \
         ((uint16_t) !!tc << 9) | \
         ((uint16_t) !!rd << 8) | \
         ((uint16_t) !!ra << 7) | \
         ((uint16_t) !!ad << 5) | \
         ((uint16_t) !!cd << 4) | \
         ((uint16_t) (rcode & 15)))

#define AVAHI_MDNS_SUFFIX_LOCAL "local"
#define AVAHI_MDNS_SUFFIX_ADDR_IPV4 "254.169.in-addr.arpa"
#define AVAHI_MDNS_SUFFIX_ADDR_IPV6 "0.8.e.f.ip6.arpa"

#endif

