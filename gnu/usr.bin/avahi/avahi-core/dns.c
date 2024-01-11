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
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <sys/types.h>
#include <netinet/in.h>

#include <avahi-common/defs.h>
#include <avahi-common/domain.h>
#include <avahi-common/malloc.h>

#include "dns.h"
#include "log.h"

AvahiDnsPacket* avahi_dns_packet_new(unsigned mtu) {
    AvahiDnsPacket *p;
    size_t max_size;

    if (mtu <= 0)
        max_size = AVAHI_DNS_PACKET_SIZE_MAX;
    else if (mtu >= AVAHI_DNS_PACKET_EXTRA_SIZE)
        max_size = mtu - AVAHI_DNS_PACKET_EXTRA_SIZE;
    else
        max_size = 0;

    if (max_size < AVAHI_DNS_PACKET_HEADER_SIZE)
        max_size = AVAHI_DNS_PACKET_HEADER_SIZE;

    if (!(p = avahi_malloc(sizeof(AvahiDnsPacket) + max_size)))
        return p;

    p->size = p->rindex = AVAHI_DNS_PACKET_HEADER_SIZE;
    p->max_size = max_size;
    p->res_size = 0;
    p->name_table = NULL;
    p->data = NULL;

    memset(AVAHI_DNS_PACKET_DATA(p), 0, p->size);
    return p;
}

AvahiDnsPacket* avahi_dns_packet_new_query(unsigned mtu) {
    AvahiDnsPacket *p;

    if (!(p = avahi_dns_packet_new(mtu)))
        return NULL;

    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_FLAGS, AVAHI_DNS_FLAGS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    return p;
}

AvahiDnsPacket* avahi_dns_packet_new_response(unsigned mtu, int aa) {
    AvahiDnsPacket *p;

    if (!(p = avahi_dns_packet_new(mtu)))
        return NULL;

    avahi_dns_packet_set_field(p, AVAHI_DNS_FIELD_FLAGS, AVAHI_DNS_FLAGS(1, 0, aa, 0, 0, 0, 0, 0, 0, 0));
    return p;
}

AvahiDnsPacket* avahi_dns_packet_new_reply(AvahiDnsPacket* p, unsigned mtu, int copy_queries, int aa) {
    AvahiDnsPacket *r;
    assert(p);

    if (!(r = avahi_dns_packet_new_response(mtu, aa)))
        return NULL;

    if (copy_queries) {
        unsigned saved_rindex;
        uint32_t n;

        saved_rindex = p->rindex;
        p->rindex = AVAHI_DNS_PACKET_HEADER_SIZE;

        for (n = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_QDCOUNT); n > 0; n--) {
            AvahiKey *k;
            int unicast_response;

            if ((k = avahi_dns_packet_consume_key(p, &unicast_response))) {
                avahi_dns_packet_append_key(r, k, unicast_response);
                avahi_key_unref(k);
            }
        }

        p->rindex = saved_rindex;

        avahi_dns_packet_set_field(r, AVAHI_DNS_FIELD_QDCOUNT, avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_QDCOUNT));
    }

    avahi_dns_packet_set_field(r, AVAHI_DNS_FIELD_ID, avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_ID));

    avahi_dns_packet_set_field(r, AVAHI_DNS_FIELD_FLAGS,
                               (avahi_dns_packet_get_field(r, AVAHI_DNS_FIELD_FLAGS) & ~AVAHI_DNS_FLAG_OPCODE) |
                               (avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_FLAGS) & AVAHI_DNS_FLAG_OPCODE));

    return r;
}


void avahi_dns_packet_free(AvahiDnsPacket *p) {
    assert(p);

    if (p->name_table)
        avahi_hashmap_free(p->name_table);

    avahi_free(p);
}

void avahi_dns_packet_set_field(AvahiDnsPacket *p, unsigned idx, uint16_t v) {
    assert(p);
    assert(idx < AVAHI_DNS_PACKET_HEADER_SIZE);

    ((uint16_t*) AVAHI_DNS_PACKET_DATA(p))[idx] = htons(v);
}

uint16_t avahi_dns_packet_get_field(AvahiDnsPacket *p, unsigned idx) {
    assert(p);
    assert(idx < AVAHI_DNS_PACKET_HEADER_SIZE);

    return ntohs(((uint16_t*) AVAHI_DNS_PACKET_DATA(p))[idx]);
}

void avahi_dns_packet_inc_field(AvahiDnsPacket *p, unsigned idx) {
    assert(p);
    assert(idx < AVAHI_DNS_PACKET_HEADER_SIZE);

    avahi_dns_packet_set_field(p, idx, avahi_dns_packet_get_field(p, idx) + 1);
}


static void name_table_cleanup(void *key, void *value, void *user_data) {
    AvahiDnsPacket *p = user_data;

    if ((uint8_t*) value >= AVAHI_DNS_PACKET_DATA(p) + p->size)
        avahi_hashmap_remove(p->name_table, key);
}

void avahi_dns_packet_cleanup_name_table(AvahiDnsPacket *p) {
    if (p->name_table)
        avahi_hashmap_foreach(p->name_table, name_table_cleanup, p);
}

uint8_t* avahi_dns_packet_append_name(AvahiDnsPacket *p, const char *name) {
    uint8_t *d, *saved_ptr = NULL;
    size_t saved_size;

    assert(p);
    assert(name);

    saved_size = p->size;
    saved_ptr = avahi_dns_packet_extend(p, 0);

    while (*name) {
        uint8_t* prev;
        const char *pname;
        char label[64], *u;

        /* Check whether we can compress this name. */

        if (p->name_table && (prev = avahi_hashmap_lookup(p->name_table, name))) {
            unsigned idx;

            assert(prev >= AVAHI_DNS_PACKET_DATA(p));
            idx = (unsigned) (prev - AVAHI_DNS_PACKET_DATA(p));

            assert(idx < p->size);

            if (idx < 0x4000) {
                uint8_t *t;
                if (!(t = (uint8_t*) avahi_dns_packet_extend(p, sizeof(uint16_t))))
                    return NULL;

		t[0] = (uint8_t) ((0xC000 | idx) >> 8);
		t[1] = (uint8_t) idx;
                return saved_ptr;
            }
        }

        pname = name;

        if (!(avahi_unescape_label(&name, label, sizeof(label))))
            goto fail;

        if (!(d = avahi_dns_packet_append_string(p, label)))
            goto fail;

        if (!p->name_table)
            /* This works only for normalized domain names */
            p->name_table = avahi_hashmap_new(avahi_string_hash, avahi_string_equal, avahi_free, NULL);

        if (!(u = avahi_strdup(pname)))
            avahi_log_error("avahi_strdup() failed.");
        else
            avahi_hashmap_insert(p->name_table, u, d);
    }

    if (!(d = avahi_dns_packet_extend(p, 1)))
        goto fail;

    *d = 0;

    return saved_ptr;

fail:
    p->size = saved_size;
    avahi_dns_packet_cleanup_name_table(p);

    return NULL;
}

uint8_t* avahi_dns_packet_append_uint16(AvahiDnsPacket *p, uint16_t v) {
    uint8_t *d;
    assert(p);

    if (!(d = avahi_dns_packet_extend(p, sizeof(uint16_t))))
        return NULL;

    d[0] = (uint8_t) (v >> 8);
    d[1] = (uint8_t) v;
    return d;
}

uint8_t *avahi_dns_packet_append_uint32(AvahiDnsPacket *p, uint32_t v) {
    uint8_t *d;
    assert(p);

    if (!(d = avahi_dns_packet_extend(p, sizeof(uint32_t))))
        return NULL;

    d[0] = (uint8_t) (v >> 24);
    d[1] = (uint8_t) (v >> 16);
    d[2] = (uint8_t) (v >> 8);
    d[3] = (uint8_t) v;

    return d;
}

uint8_t *avahi_dns_packet_append_bytes(AvahiDnsPacket  *p, const void *b, size_t l) {
    uint8_t* d;

    assert(p);
    assert(b);
    assert(l);

    if (!(d = avahi_dns_packet_extend(p, l)))
        return NULL;

    memcpy(d, b, l);
    return d;
}

uint8_t* avahi_dns_packet_append_string(AvahiDnsPacket *p, const char *s) {
    uint8_t* d;
    size_t k;

    assert(p);
    assert(s);

    if ((k = strlen(s)) >= 255)
        k = 255;

    if (!(d = avahi_dns_packet_extend(p, k+1)))
        return NULL;

    *d = (uint8_t) k;
    memcpy(d+1, s, k);

    return d;
}

uint8_t *avahi_dns_packet_extend(AvahiDnsPacket *p, size_t l) {
    uint8_t *d;

    assert(p);

    if (p->size+l > p->max_size)
        return NULL;

    d = AVAHI_DNS_PACKET_DATA(p) + p->size;
    p->size += l;

    return d;
}

int avahi_dns_packet_check_valid(AvahiDnsPacket *p) {
    uint16_t flags;
    assert(p);

    if (p->size < AVAHI_DNS_PACKET_HEADER_SIZE)
        return -1;

    flags = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_FLAGS);

    if (flags & AVAHI_DNS_FLAG_OPCODE)
        return -1;

    return 0;
}

int avahi_dns_packet_check_valid_multicast(AvahiDnsPacket *p) {
    uint16_t flags;
    assert(p);

    if (avahi_dns_packet_check_valid(p) < 0)
        return -1;

    flags = avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_FLAGS);

    if (flags & AVAHI_DNS_FLAG_RCODE)
        return -1;

    return 0;
}

int avahi_dns_packet_is_query(AvahiDnsPacket *p) {
    assert(p);

    return !(avahi_dns_packet_get_field(p, AVAHI_DNS_FIELD_FLAGS) & AVAHI_DNS_FLAG_QR);
}

static int consume_labels(AvahiDnsPacket *p, unsigned idx, char *ret_name, size_t l) {
    int ret = 0;
    int compressed = 0;
    int first_label = 1;
    unsigned label_ptr;
    int i;
    assert(p && ret_name && l);

    for (i = 0; i < AVAHI_DNS_LABELS_MAX; i++) {
        uint8_t n;

        if (idx+1 > p->size)
            return -1;

        n = AVAHI_DNS_PACKET_DATA(p)[idx];

        if (!n) {
            idx++;
            if (!compressed)
                ret++;

            if (l < 1)
                return -1;
            *ret_name = 0;

            return ret;

        } else if (n <= 63) {
            /* Uncompressed label */
            idx++;
            if (!compressed)
                ret++;

            if (idx + n > p->size)
                return -1;

            if ((size_t) n + 1 > l)
                return -1;

            if (!first_label) {
                *(ret_name++) = '.';
                l--;
            } else
                first_label = 0;

            if (!(avahi_escape_label((char*) AVAHI_DNS_PACKET_DATA(p) + idx, n, &ret_name, &l)))
                return -1;

            idx += n;

            if (!compressed)
                ret += n;
        } else if ((n & 0xC0) == 0xC0) {
            /* Compressed label */

            if (idx+2 > p->size)
                return -1;

            label_ptr = ((unsigned) (AVAHI_DNS_PACKET_DATA(p)[idx] & ~0xC0)) << 8 | AVAHI_DNS_PACKET_DATA(p)[idx+1];

            if ((label_ptr < AVAHI_DNS_PACKET_HEADER_SIZE) || (label_ptr >= idx))
                return -1;

            idx = label_ptr;

            if (!compressed)
                ret += 2;

            compressed = 1;
        } else
            return -1;
    }

    return -1;
}

int avahi_dns_packet_consume_name(AvahiDnsPacket *p, char *ret_name, size_t l) {
    int r;

    if ((r = consume_labels(p, p->rindex, ret_name, l)) < 0)
        return -1;

    p->rindex += r;
    return 0;
}

int avahi_dns_packet_consume_uint16(AvahiDnsPacket *p, uint16_t *ret_v) {
    uint8_t *d;

    assert(p);
    assert(ret_v);

    if (p->rindex + sizeof(uint16_t) > p->size)
        return -1;

    d = (uint8_t*) (AVAHI_DNS_PACKET_DATA(p) + p->rindex);
    *ret_v = (d[0] << 8) | d[1];
    p->rindex += sizeof(uint16_t);

    return 0;
}

int avahi_dns_packet_consume_uint32(AvahiDnsPacket *p, uint32_t *ret_v) {
    uint8_t* d;

    assert(p);
    assert(ret_v);

    if (p->rindex + sizeof(uint32_t) > p->size)
        return -1;

    d = (uint8_t*) (AVAHI_DNS_PACKET_DATA(p) + p->rindex);
    *ret_v = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3];
    p->rindex += sizeof(uint32_t);

    return 0;
}

int avahi_dns_packet_consume_bytes(AvahiDnsPacket *p, void * ret_data, size_t l) {
    assert(p);
    assert(ret_data);
    assert(l > 0);

    if (p->rindex + l > p->size)
        return -1;

    memcpy(ret_data, AVAHI_DNS_PACKET_DATA(p) + p->rindex, l);
    p->rindex += l;

    return 0;
}

int avahi_dns_packet_consume_string(AvahiDnsPacket *p, char *ret_string, size_t l) {
    size_t k;

    assert(p);
    assert(ret_string);
    assert(l > 0);

    if (p->rindex >= p->size)
        return -1;

    k = AVAHI_DNS_PACKET_DATA(p)[p->rindex];

    if (p->rindex+1+k > p->size)
        return -1;

    if (l > k+1)
        l = k+1;

    memcpy(ret_string, AVAHI_DNS_PACKET_DATA(p)+p->rindex+1, l-1);
    ret_string[l-1] = 0;

    p->rindex += 1+k;

    return 0;
}

const void* avahi_dns_packet_get_rptr(AvahiDnsPacket *p) {
    assert(p);

    if (p->rindex > p->size)
        return NULL;

    return AVAHI_DNS_PACKET_DATA(p) + p->rindex;
}

int avahi_dns_packet_skip(AvahiDnsPacket *p, size_t length) {
    assert(p);

    if (p->rindex + length > p->size)
        return -1;

    p->rindex += length;
    return 0;
}

static int parse_rdata(AvahiDnsPacket *p, AvahiRecord *r, uint16_t rdlength) {
    char buf[AVAHI_DOMAIN_NAME_MAX];
    const void* start;

    assert(p);
    assert(r);

    start = avahi_dns_packet_get_rptr(p);

    switch (r->key->type) {
        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:

            if (avahi_dns_packet_consume_name(p, buf, sizeof(buf)) < 0)
                return -1;

            r->data.ptr.name = avahi_strdup(buf);
            break;


        case AVAHI_DNS_TYPE_SRV:

            if (avahi_dns_packet_consume_uint16(p, &r->data.srv.priority) < 0 ||
                avahi_dns_packet_consume_uint16(p, &r->data.srv.weight) < 0 ||
                avahi_dns_packet_consume_uint16(p, &r->data.srv.port) < 0 ||
                avahi_dns_packet_consume_name(p, buf, sizeof(buf)) < 0)
                return -1;

            r->data.srv.name = avahi_strdup(buf);
            break;

        case AVAHI_DNS_TYPE_HINFO:

            if (avahi_dns_packet_consume_string(p, buf, sizeof(buf)) < 0)
                return -1;

            r->data.hinfo.cpu = avahi_strdup(buf);

            if (avahi_dns_packet_consume_string(p, buf, sizeof(buf)) < 0)
                return -1;

            r->data.hinfo.os = avahi_strdup(buf);
            break;

        case AVAHI_DNS_TYPE_TXT:

            if (rdlength > 0) {
                if (avahi_string_list_parse(avahi_dns_packet_get_rptr(p), rdlength, &r->data.txt.string_list) < 0)
                    return -1;

                if (avahi_dns_packet_skip(p, rdlength) < 0)
                    return -1;
            } else
                r->data.txt.string_list = NULL;

            break;

        case AVAHI_DNS_TYPE_A:

/*             avahi_log_debug("A"); */

            if (avahi_dns_packet_consume_bytes(p, &r->data.a.address, sizeof(AvahiIPv4Address)) < 0)
                return -1;

            break;

        case AVAHI_DNS_TYPE_AAAA:

/*             avahi_log_debug("aaaa"); */

            if (avahi_dns_packet_consume_bytes(p, &r->data.aaaa.address, sizeof(AvahiIPv6Address)) < 0)
                return -1;

            break;

        default:

/*             avahi_log_debug("generic"); */

            if (rdlength > 0) {

                r->data.generic.data = avahi_memdup(avahi_dns_packet_get_rptr(p), rdlength);
                r->data.generic.size = rdlength;

                if (avahi_dns_packet_skip(p, rdlength) < 0)
                    return -1;
            }

            break;
    }

    /* Check if we read enough data */
    if ((const uint8_t*) avahi_dns_packet_get_rptr(p) - (const uint8_t*) start != rdlength)
        return -1;

    return 0;
}

AvahiRecord* avahi_dns_packet_consume_record(AvahiDnsPacket *p, int *ret_cache_flush) {
    char name[AVAHI_DOMAIN_NAME_MAX];
    uint16_t type, class;
    uint32_t ttl;
    uint16_t rdlength;
    AvahiRecord *r = NULL;

    assert(p);

    if (avahi_dns_packet_consume_name(p, name, sizeof(name)) < 0 ||
        avahi_dns_packet_consume_uint16(p, &type) < 0 ||
        avahi_dns_packet_consume_uint16(p, &class) < 0 ||
        avahi_dns_packet_consume_uint32(p, &ttl) < 0 ||
        avahi_dns_packet_consume_uint16(p, &rdlength) < 0 ||
        p->rindex + rdlength > p->size)
        goto fail;

    if (ret_cache_flush)
        *ret_cache_flush = !!(class & AVAHI_DNS_CACHE_FLUSH);
    class &= ~AVAHI_DNS_CACHE_FLUSH;

    if (!(r = avahi_record_new_full(name, class, type, ttl)))
        goto fail;

    if (parse_rdata(p, r, rdlength) < 0)
        goto fail;

    if (!avahi_record_is_valid(r))
        goto fail;

    return r;

fail:
    if (r)
        avahi_record_unref(r);

    return NULL;
}

AvahiKey* avahi_dns_packet_consume_key(AvahiDnsPacket *p, int *ret_unicast_response) {
    char name[256];
    uint16_t type, class;
    AvahiKey *k;

    assert(p);

    if (avahi_dns_packet_consume_name(p, name, sizeof(name)) < 0 ||
        avahi_dns_packet_consume_uint16(p, &type) < 0 ||
        avahi_dns_packet_consume_uint16(p, &class) < 0)
        return NULL;

    if (ret_unicast_response)
        *ret_unicast_response = !!(class & AVAHI_DNS_UNICAST_RESPONSE);

    class &= ~AVAHI_DNS_UNICAST_RESPONSE;

    if (!(k = avahi_key_new(name, class, type)))
        return NULL;

    if (!avahi_key_is_valid(k)) {
        avahi_key_unref(k);
        return NULL;
    }

    return k;
}

uint8_t* avahi_dns_packet_append_key(AvahiDnsPacket *p, AvahiKey *k, int unicast_response) {
    uint8_t *t;
    size_t size;

    assert(p);
    assert(k);

    size = p->size;

    if (!(t = avahi_dns_packet_append_name(p, k->name)) ||
        !avahi_dns_packet_append_uint16(p, k->type) ||
        !avahi_dns_packet_append_uint16(p, k->clazz | (unicast_response ? AVAHI_DNS_UNICAST_RESPONSE : 0))) {
        p->size = size;
        avahi_dns_packet_cleanup_name_table(p);

        return NULL;
    }

    return t;
}

static int append_rdata(AvahiDnsPacket *p, AvahiRecord *r) {
    assert(p);
    assert(r);

    switch (r->key->type) {

        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:

            if (!(avahi_dns_packet_append_name(p, r->data.ptr.name)))
                return -1;

            break;

        case AVAHI_DNS_TYPE_SRV:

            if (!avahi_dns_packet_append_uint16(p, r->data.srv.priority) ||
                !avahi_dns_packet_append_uint16(p, r->data.srv.weight) ||
                !avahi_dns_packet_append_uint16(p, r->data.srv.port) ||
                !avahi_dns_packet_append_name(p, r->data.srv.name))
                return -1;

            break;

        case AVAHI_DNS_TYPE_HINFO:
            if (!avahi_dns_packet_append_string(p, r->data.hinfo.cpu) ||
                !avahi_dns_packet_append_string(p, r->data.hinfo.os))
                return -1;

            break;

        case AVAHI_DNS_TYPE_TXT: {

            uint8_t *data;
            size_t n;

            n = avahi_string_list_serialize(r->data.txt.string_list, NULL, 0);

            if (!(data = avahi_dns_packet_extend(p, n)))
                return -1;

            avahi_string_list_serialize(r->data.txt.string_list, data, n);
            break;
        }


        case AVAHI_DNS_TYPE_A:

            if (!avahi_dns_packet_append_bytes(p, &r->data.a.address, sizeof(r->data.a.address)))
                return -1;

            break;

        case AVAHI_DNS_TYPE_AAAA:

            if (!avahi_dns_packet_append_bytes(p, &r->data.aaaa.address, sizeof(r->data.aaaa.address)))
                return -1;

            break;

        default:

            if (r->data.generic.size)
                if (!avahi_dns_packet_append_bytes(p, r->data.generic.data, r->data.generic.size))
                    return -1;

            break;
    }

    return 0;
}


uint8_t* avahi_dns_packet_append_record(AvahiDnsPacket *p, AvahiRecord *r, int cache_flush, unsigned max_ttl) {
    uint8_t *t, *l, *start;
    size_t size;

    assert(p);
    assert(r);

    size = p->size;

    if (!(t = avahi_dns_packet_append_name(p, r->key->name)) ||
        !avahi_dns_packet_append_uint16(p, r->key->type) ||
        !avahi_dns_packet_append_uint16(p, cache_flush ? (r->key->clazz | AVAHI_DNS_CACHE_FLUSH) : (r->key->clazz &~ AVAHI_DNS_CACHE_FLUSH)) ||
        !avahi_dns_packet_append_uint32(p, (max_ttl && r->ttl > max_ttl) ? max_ttl : r->ttl) ||
        !(l = avahi_dns_packet_append_uint16(p, 0)))
        goto fail;

    start = avahi_dns_packet_extend(p, 0);

    if (append_rdata(p, r) < 0)
        goto fail;

    size = avahi_dns_packet_extend(p, 0) - start;
    assert(size <= AVAHI_DNS_RDATA_MAX);

/*     avahi_log_debug("appended %u", size); */

    l[0] = (uint8_t) ((uint16_t) size >> 8);
    l[1] = (uint8_t) ((uint16_t) size);

    return t;


fail:
    p->size = size;
    avahi_dns_packet_cleanup_name_table(p);

    return NULL;
}

int avahi_dns_packet_is_empty(AvahiDnsPacket *p) {
    assert(p);

    return p->size <= AVAHI_DNS_PACKET_HEADER_SIZE;
}

size_t avahi_dns_packet_space(AvahiDnsPacket *p) {
    assert(p);

    assert(p->size <= p->max_size);

    return p->max_size - p->size;
}

size_t avahi_dns_packet_reserve_size(AvahiDnsPacket *p, size_t res_size) {
    assert(p);

    assert(p->size + p->res_size <= p->max_size);

    if ((p->size + p->res_size + res_size) <= p->max_size)
   p->res_size += res_size;

    return p->res_size;
}

size_t avahi_dns_packet_reserved_space(AvahiDnsPacket *p) {
    assert(p);

    assert(p->size + p->res_size <= p->max_size);

    return p->max_size - p->size - p->res_size;
}

int avahi_rdata_parse(AvahiRecord *record, const void* rdata, size_t size) {
    int ret;
    AvahiDnsPacket p;

    assert(record);
    assert(rdata);

    p.data = (void*) rdata;
    p.max_size = p.size = size;
    p.rindex = 0;
    p.name_table = NULL;

    ret = parse_rdata(&p, record, size);

    assert(!p.name_table);

    return ret;
}

size_t avahi_rdata_serialize(AvahiRecord *record, void *rdata, size_t max_size) {
    int ret;
    AvahiDnsPacket p;

    assert(record);
    assert(rdata);
    assert(max_size > 0);

    p.data = (void*) rdata;
    p.max_size = max_size;
    p.size = p.rindex = 0;
    p.name_table = NULL;

    ret = append_rdata(&p, record);

    if (p.name_table)
         avahi_hashmap_free(p.name_table);

    if (ret < 0)
        return (size_t) -1;

    return p.size;
}
