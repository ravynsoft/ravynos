#ifndef foorrhfoo
#define foorrhfoo

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

/** \file rr.h Functions and definitions for manipulating DNS resource record (RR) data. */

#include <inttypes.h>
#include <sys/types.h>

#include <avahi-common/strlst.h>
#include <avahi-common/address.h>
#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

/** DNS record types, see RFC 1035, in addition to those defined in defs.h */
enum {
    AVAHI_DNS_TYPE_ANY = 0xFF,   /**< Special query type for requesting all records */
    AVAHI_DNS_TYPE_OPT = 41,     /**< EDNS0 option */
    AVAHI_DNS_TYPE_TKEY = 249,
    AVAHI_DNS_TYPE_TSIG = 250,
    AVAHI_DNS_TYPE_IXFR = 251,
    AVAHI_DNS_TYPE_AXFR = 252
};

/** DNS record classes, see RFC 1035, in addition to those defined in defs.h */
enum {
    AVAHI_DNS_CLASS_ANY = 0xFF,         /**< Special query type for requesting all records */
    AVAHI_DNS_CACHE_FLUSH = 0x8000,     /**< Not really a class but a bit which may be set in response packets, see mDNS spec for more information */
    AVAHI_DNS_UNICAST_RESPONSE = 0x8000 /**< Not really a class but a bit which may be set in query packets, see mDNS spec for more information */
};

/** Encapsulates a DNS query key consisting of class, type and
    name. Use avahi_key_ref()/avahi_key_unref() for manipulating the
    reference counter. The structure is intended to be treated as "immutable", no
    changes should be imposed after creation */
typedef struct AvahiKey {
    int ref;           /**< Reference counter */
    char *name;        /**< Record name */
    uint16_t clazz;    /**< Record class, one of the AVAHI_DNS_CLASS_xxx constants */
    uint16_t type;     /**< Record type, one of the AVAHI_DNS_TYPE_xxx constants */
} AvahiKey;

/** Encapsulates a DNS resource record. The structure is intended to
 * be treated as "immutable", no changes should be imposed after
 * creation. */
typedef struct AvahiRecord {
    int ref;         /**< Reference counter */
    AvahiKey *key;   /**< Reference to the query key of this record */

    uint32_t ttl;     /**< DNS TTL of this record */

    union {

        struct {
            void* data;
            uint16_t size;
        } generic; /**< Generic record data for unknown types */

        struct {
            uint16_t priority;
            uint16_t weight;
            uint16_t port;
            char *name;
        } srv; /**< Data for SRV records */

        struct {
            char *name;
        } ptr, ns, cname; /**< Data for PTR, NS and CNAME records */

        struct {
            char *cpu;
            char *os;
        } hinfo; /**< Data for HINFO records */

        struct {
            AvahiStringList *string_list;
        } txt; /**< Data for TXT records */

        struct {
            AvahiIPv4Address address;
        } a; /**< Data for A records */

        struct {
            AvahiIPv6Address address;
        } aaaa; /**< Data for AAAA records */

    } data; /**< Record data */

} AvahiRecord;

/** Create a new AvahiKey object. The reference counter will be set to 1. */
AvahiKey *avahi_key_new(const char *name, uint16_t clazz, uint16_t type);

/** Increase the reference counter of an AvahiKey object by one */
AvahiKey *avahi_key_ref(AvahiKey *k);

/** Decrease the reference counter of an AvahiKey object by one */
void avahi_key_unref(AvahiKey *k);

/** Check whether two AvahiKey object contain the same
 * data. AVAHI_DNS_CLASS_ANY/AVAHI_DNS_TYPE_ANY are treated like any
 * other class/type. */
int avahi_key_equal(const AvahiKey *a, const AvahiKey *b);

/** Return a numeric hash value for a key for usage in hash tables. */
unsigned avahi_key_hash(const AvahiKey *k);

/** Create a new record object. Record data should be filled in right after creation. The reference counter is set to 1. */
AvahiRecord *avahi_record_new(AvahiKey *k, uint32_t ttl);

/** Create a new record object. Record data should be filled in right after creation. The reference counter is set to 1. */
AvahiRecord *avahi_record_new_full(const char *name, uint16_t clazz, uint16_t type, uint32_t ttl);

/** Increase the reference counter of an AvahiRecord by one. */
AvahiRecord *avahi_record_ref(AvahiRecord *r);

/** Decrease the reference counter of an AvahiRecord by one. */
void avahi_record_unref(AvahiRecord *r);

/** Return a textual representation of the specified DNS class. The
 * returned pointer points to a read only internal string. */
const char *avahi_dns_class_to_string(uint16_t clazz);

/** Return a textual representation of the specified DNS class. The
 * returned pointer points to a read only internal string. */
const char *avahi_dns_type_to_string(uint16_t type);

/** Create a textual representation of the specified key. avahi_free() the
 * result! */
char *avahi_key_to_string(const AvahiKey *k);

/** Create a textual representation of the specified record, similar
 * in style to BIND zone file data. avahi_free() the result! */
char *avahi_record_to_string(const AvahiRecord *r);

/** Check whether two records are equal (regardless of the TTL */
int avahi_record_equal_no_ttl(const AvahiRecord *a, const AvahiRecord *b);

/** Check whether the specified key is valid */
int avahi_key_is_valid(AvahiKey *k);

/** Check whether the specified record is valid */
int avahi_record_is_valid(AvahiRecord *r);

/** Parse a binary rdata object and fill it into *record. This function is actually implemented in dns.c */
int avahi_rdata_parse(AvahiRecord *record, const void* rdata, size_t size);

/** Serialize an AvahiRecord object into binary rdata. This function is actually implemented in dns.c */
size_t avahi_rdata_serialize(AvahiRecord *record, void *rdata, size_t max_size);

/** Return TRUE if the AvahiRecord object is a link-local A or AAAA address */
int avahi_record_is_link_local_address(const AvahiRecord *r);

AVAHI_C_DECL_END

#endif
