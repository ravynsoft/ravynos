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

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>

#include <avahi-common/domain.h>
#include <avahi-common/malloc.h>
#include <avahi-common/defs.h>

#include "rr.h"
#include "log.h"
#include "util.h"
#include "hashmap.h"
#include "domain-util.h"
#include "rr-util.h"
#include "addr-util.h"

AvahiKey *avahi_key_new(const char *name, uint16_t class, uint16_t type) {
    AvahiKey *k;
    assert(name);

    if (!(k = avahi_new(AvahiKey, 1))) {
        avahi_log_error("avahi_new() failed.");
        return NULL;
    }

    if (!(k->name = avahi_normalize_name_strdup(name))) {
        avahi_log_error("avahi_normalize_name() failed.");
        avahi_free(k);
        return NULL;
    }

    k->ref = 1;
    k->clazz = class;
    k->type = type;

    return k;
}

AvahiKey *avahi_key_new_cname(AvahiKey *key) {
    assert(key);

    if (key->clazz != AVAHI_DNS_CLASS_IN)
        return NULL;

    if (key->type == AVAHI_DNS_TYPE_CNAME)
        return NULL;

    return avahi_key_new(key->name, key->clazz, AVAHI_DNS_TYPE_CNAME);
}

AvahiKey *avahi_key_ref(AvahiKey *k) {
    assert(k);
    assert(k->ref >= 1);

    k->ref++;

    return k;
}

void avahi_key_unref(AvahiKey *k) {
    assert(k);
    assert(k->ref >= 1);

    if ((--k->ref) <= 0) {
        avahi_free(k->name);
        avahi_free(k);
    }
}

AvahiRecord *avahi_record_new(AvahiKey *k, uint32_t ttl) {
    AvahiRecord *r;

    assert(k);

    if (!(r = avahi_new(AvahiRecord, 1))) {
        avahi_log_error("avahi_new() failed.");
        return NULL;
    }

    r->ref = 1;
    r->key = avahi_key_ref(k);

    memset(&r->data, 0, sizeof(r->data));

    r->ttl = ttl != (uint32_t) -1 ? ttl : AVAHI_DEFAULT_TTL;

    return r;
}

AvahiRecord *avahi_record_new_full(const char *name, uint16_t class, uint16_t type, uint32_t ttl) {
    AvahiRecord *r;
    AvahiKey *k;

    assert(name);

    if (!(k = avahi_key_new(name, class, type))) {
        avahi_log_error("avahi_key_new() failed.");
        return NULL;
    }

    r = avahi_record_new(k, ttl);
    avahi_key_unref(k);

    if (!r) {
        avahi_log_error("avahi_record_new() failed.");
        return NULL;
    }

    return r;
}

AvahiRecord *avahi_record_ref(AvahiRecord *r) {
    assert(r);
    assert(r->ref >= 1);

    r->ref++;
    return r;
}

void avahi_record_unref(AvahiRecord *r) {
    assert(r);
    assert(r->ref >= 1);

    if ((--r->ref) <= 0) {
        switch (r->key->type) {

            case AVAHI_DNS_TYPE_SRV:
                avahi_free(r->data.srv.name);
                break;

            case AVAHI_DNS_TYPE_PTR:
            case AVAHI_DNS_TYPE_CNAME:
            case AVAHI_DNS_TYPE_NS:
                avahi_free(r->data.ptr.name);
                break;

            case AVAHI_DNS_TYPE_HINFO:
                avahi_free(r->data.hinfo.cpu);
                avahi_free(r->data.hinfo.os);
                break;

            case AVAHI_DNS_TYPE_TXT:
                avahi_string_list_free(r->data.txt.string_list);
                break;

            case AVAHI_DNS_TYPE_A:
            case AVAHI_DNS_TYPE_AAAA:
                break;

            default:
                avahi_free(r->data.generic.data);
        }

        avahi_key_unref(r->key);
        avahi_free(r);
    }
}

const char *avahi_dns_class_to_string(uint16_t class) {
    if (class & AVAHI_DNS_CACHE_FLUSH)
        return "FLUSH";

    switch (class) {
        case AVAHI_DNS_CLASS_IN:
            return "IN";
        case AVAHI_DNS_CLASS_ANY:
            return "ANY";
        default:
            return NULL;
    }
}

const char *avahi_dns_type_to_string(uint16_t type) {
    switch (type) {
        case AVAHI_DNS_TYPE_CNAME:
            return "CNAME";
        case AVAHI_DNS_TYPE_A:
            return "A";
        case AVAHI_DNS_TYPE_AAAA:
            return "AAAA";
        case AVAHI_DNS_TYPE_PTR:
            return "PTR";
        case AVAHI_DNS_TYPE_HINFO:
            return "HINFO";
        case AVAHI_DNS_TYPE_TXT:
            return "TXT";
        case AVAHI_DNS_TYPE_SRV:
            return "SRV";
        case AVAHI_DNS_TYPE_ANY:
            return "ANY";
        case AVAHI_DNS_TYPE_SOA:
            return "SOA";
        case AVAHI_DNS_TYPE_NS:
            return "NS";
        default:
            return NULL;
    }
}

char *avahi_key_to_string(const AvahiKey *k) {
    char class[16], type[16];
    const char *c, *t;

    assert(k);
    assert(k->ref >= 1);

    /* According to RFC3597 */

    if (!(c = avahi_dns_class_to_string(k->clazz))) {
        snprintf(class, sizeof(class), "CLASS%u", k->clazz);
        c = class;
    }

    if (!(t = avahi_dns_type_to_string(k->type))) {
        snprintf(type, sizeof(type), "TYPE%u", k->type);
        t = type;
    }

    return avahi_strdup_printf("%s\t%s\t%s", k->name, c, t);
}

char *avahi_record_to_string(const AvahiRecord *r) {
    char *p, *s;
    char buf[1024], *t = NULL, *d = NULL;

    assert(r);
    assert(r->ref >= 1);

    switch (r->key->type) {
        case AVAHI_DNS_TYPE_A:
            inet_ntop(AF_INET, &r->data.a.address.address, t = buf, sizeof(buf));
            break;

        case AVAHI_DNS_TYPE_AAAA:
            inet_ntop(AF_INET6, &r->data.aaaa.address.address, t = buf, sizeof(buf));
            break;

        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:

            t = r->data.ptr.name;
            break;

        case AVAHI_DNS_TYPE_TXT:
            t = d = avahi_string_list_to_string(r->data.txt.string_list);
            break;

        case AVAHI_DNS_TYPE_HINFO:

            snprintf(t = buf, sizeof(buf), "\"%s\" \"%s\"", r->data.hinfo.cpu, r->data.hinfo.os);
            break;

        case AVAHI_DNS_TYPE_SRV:

            snprintf(t = buf, sizeof(buf), "%u %u %u %s",
                     r->data.srv.priority,
                     r->data.srv.weight,
                     r->data.srv.port,
                     r->data.srv.name);

            break;

        default: {

            uint8_t *c;
            uint16_t n;
            int i;
            char *e;

            /* According to RFC3597 */

            snprintf(t = buf, sizeof(buf), "\\# %u", r->data.generic.size);

            e = strchr(t, 0);

            for (c = r->data.generic.data, n = r->data.generic.size, i = 0;
                 n > 0 && i < 20;
                 c ++, n --, i++) {

                sprintf(e, " %02X", *c);
                e = strchr(e, 0);
            }

            break;
        }
    }

    p = avahi_key_to_string(r->key);
    s = avahi_strdup_printf("%s %s ; ttl=%u", p, t, r->ttl);
    avahi_free(p);
    avahi_free(d);

    return s;
}

int avahi_key_equal(const AvahiKey *a, const AvahiKey *b) {
    assert(a);
    assert(b);

    if (a == b)
        return 1;

    return avahi_domain_equal(a->name, b->name) &&
        a->type == b->type &&
        a->clazz == b->clazz;
}

int avahi_key_pattern_match(const AvahiKey *pattern, const AvahiKey *k) {
    assert(pattern);
    assert(k);

    assert(!avahi_key_is_pattern(k));

    if (pattern == k)
        return 1;

    return avahi_domain_equal(pattern->name, k->name) &&
        (pattern->type == k->type || pattern->type == AVAHI_DNS_TYPE_ANY) &&
        (pattern->clazz == k->clazz || pattern->clazz == AVAHI_DNS_CLASS_ANY);
}

int avahi_key_is_pattern(const AvahiKey *k) {
    assert(k);

    return
        k->type == AVAHI_DNS_TYPE_ANY ||
        k->clazz == AVAHI_DNS_CLASS_ANY;
}

unsigned avahi_key_hash(const AvahiKey *k) {
    assert(k);

    return
        avahi_domain_hash(k->name) +
        k->type +
        k->clazz;
}

static int rdata_equal(const AvahiRecord *a, const AvahiRecord *b) {
    assert(a);
    assert(b);
    assert(a->key->type == b->key->type);

    switch (a->key->type) {
        case AVAHI_DNS_TYPE_SRV:
            return
                a->data.srv.priority == b->data.srv.priority &&
                a->data.srv.weight == b->data.srv.weight &&
                a->data.srv.port == b->data.srv.port &&
                avahi_domain_equal(a->data.srv.name, b->data.srv.name);

        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:
            return avahi_domain_equal(a->data.ptr.name, b->data.ptr.name);

        case AVAHI_DNS_TYPE_HINFO:
            return
                !strcmp(a->data.hinfo.cpu, b->data.hinfo.cpu) &&
                !strcmp(a->data.hinfo.os, b->data.hinfo.os);

        case AVAHI_DNS_TYPE_TXT:
            return avahi_string_list_equal(a->data.txt.string_list, b->data.txt.string_list);

        case AVAHI_DNS_TYPE_A:
            return memcmp(&a->data.a.address, &b->data.a.address, sizeof(AvahiIPv4Address)) == 0;

        case AVAHI_DNS_TYPE_AAAA:
            return memcmp(&a->data.aaaa.address, &b->data.aaaa.address, sizeof(AvahiIPv6Address)) == 0;

        default:
            return a->data.generic.size == b->data.generic.size &&
                (a->data.generic.size == 0 || memcmp(a->data.generic.data, b->data.generic.data, a->data.generic.size) == 0);
    }

}

int avahi_record_equal_no_ttl(const AvahiRecord *a, const AvahiRecord *b) {
    assert(a);
    assert(b);

    if (a == b)
        return 1;

    return
        avahi_key_equal(a->key, b->key) &&
        rdata_equal(a, b);
}


AvahiRecord *avahi_record_copy(AvahiRecord *r) {
    AvahiRecord *copy;

    if (!(copy = avahi_new(AvahiRecord, 1))) {
        avahi_log_error("avahi_new() failed.");
        return NULL;
    }

    copy->ref = 1;
    copy->key = avahi_key_ref(r->key);
    copy->ttl = r->ttl;

    switch (r->key->type) {
        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:
            if (!(copy->data.ptr.name = avahi_strdup(r->data.ptr.name)))
                goto fail;
            break;

        case AVAHI_DNS_TYPE_SRV:
            copy->data.srv.priority = r->data.srv.priority;
            copy->data.srv.weight = r->data.srv.weight;
            copy->data.srv.port = r->data.srv.port;
            if (!(copy->data.srv.name = avahi_strdup(r->data.srv.name)))
                goto fail;
            break;

        case AVAHI_DNS_TYPE_HINFO:
            if (!(copy->data.hinfo.os = avahi_strdup(r->data.hinfo.os)))
                goto fail;

            if (!(copy->data.hinfo.cpu = avahi_strdup(r->data.hinfo.cpu))) {
                avahi_free(r->data.hinfo.os);
                goto fail;
            }
            break;

        case AVAHI_DNS_TYPE_TXT:
            copy->data.txt.string_list = avahi_string_list_copy(r->data.txt.string_list);
            break;

        case AVAHI_DNS_TYPE_A:
            copy->data.a.address = r->data.a.address;
            break;

        case AVAHI_DNS_TYPE_AAAA:
            copy->data.aaaa.address = r->data.aaaa.address;
            break;

        default:
            if (!(copy->data.generic.data = avahi_memdup(r->data.generic.data, r->data.generic.size)))
                goto fail;
            copy->data.generic.size = r->data.generic.size;
            break;

    }

    return copy;

fail:
    avahi_log_error("Failed to allocate memory");

    avahi_key_unref(copy->key);
    avahi_free(copy);

    return NULL;
}


size_t avahi_key_get_estimate_size(AvahiKey *k) {
    assert(k);

    return strlen(k->name)+1+4;
}

size_t avahi_record_get_estimate_size(AvahiRecord *r) {
    size_t n;
    assert(r);

    n = avahi_key_get_estimate_size(r->key) + 4 + 2;

    switch (r->key->type) {
        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:
            n += strlen(r->data.ptr.name) + 1;
            break;

        case AVAHI_DNS_TYPE_SRV:
            n += 6 + strlen(r->data.srv.name) + 1;
            break;

        case AVAHI_DNS_TYPE_HINFO:
            n += strlen(r->data.hinfo.os) + 1 + strlen(r->data.hinfo.cpu) + 1;
            break;

        case AVAHI_DNS_TYPE_TXT:
            n += avahi_string_list_serialize(r->data.txt.string_list, NULL, 0);
            break;

        case AVAHI_DNS_TYPE_A:
            n += sizeof(AvahiIPv4Address);
            break;

        case AVAHI_DNS_TYPE_AAAA:
            n += sizeof(AvahiIPv6Address);
            break;

        default:
            n += r->data.generic.size;
    }

    return n;
}

static int lexicographical_memcmp(const void* a, size_t al, const void* b, size_t bl) {
    size_t c;
    int ret;

    assert(a);
    assert(b);

    c = al < bl ? al : bl;
    if ((ret = memcmp(a, b, c)))
        return ret;

    if (al == bl)
        return 0;
    else
        return al == c ? 1 : -1;
}

static int uint16_cmp(uint16_t a, uint16_t b) {
    return a == b ? 0 : (a < b ? -1 : 1);
}

int avahi_record_lexicographical_compare(AvahiRecord *a, AvahiRecord *b) {
    int r;
/*      char *t1, *t2; */

    assert(a);
    assert(b);

/*     t1 = avahi_record_to_string(a); */
/*     t2 = avahi_record_to_string(b); */
/*     g_message("lexicocmp: %s %s", t1, t2); */
/*     avahi_free(t1); */
/*     avahi_free(t2); */

    if (a == b)
        return 0;

    if ((r = uint16_cmp(a->key->clazz, b->key->clazz)) ||
        (r = uint16_cmp(a->key->type, b->key->type)))
        return r;

    switch (a->key->type) {

        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:
            return avahi_binary_domain_cmp(a->data.ptr.name, b->data.ptr.name);

        case AVAHI_DNS_TYPE_SRV: {
            if ((r = uint16_cmp(a->data.srv.priority, b->data.srv.priority)) == 0 &&
                (r = uint16_cmp(a->data.srv.weight, b->data.srv.weight)) == 0 &&
                (r = uint16_cmp(a->data.srv.port, b->data.srv.port)) == 0)
                r = avahi_binary_domain_cmp(a->data.srv.name, b->data.srv.name);

            return r;
        }

        case AVAHI_DNS_TYPE_HINFO: {

            if ((r = strcmp(a->data.hinfo.cpu, b->data.hinfo.cpu)) ||
                (r = strcmp(a->data.hinfo.os, b->data.hinfo.os)))
                return r;

            return 0;

        }

        case AVAHI_DNS_TYPE_TXT: {

            uint8_t *ma = NULL, *mb = NULL;
            size_t asize, bsize;

            asize = avahi_string_list_serialize(a->data.txt.string_list, NULL, 0);
            bsize = avahi_string_list_serialize(b->data.txt.string_list, NULL, 0);

            if (asize > 0 && !(ma = avahi_new(uint8_t, asize)))
                goto fail;

            if (bsize > 0 && !(mb = avahi_new(uint8_t, bsize))) {
                avahi_free(ma);
                goto fail;
            }

            avahi_string_list_serialize(a->data.txt.string_list, ma, asize);
            avahi_string_list_serialize(b->data.txt.string_list, mb, bsize);

            if (asize && bsize)
                r = lexicographical_memcmp(ma, asize, mb, bsize);
            else if (asize && !bsize)
                r = 1;
            else if (!asize && bsize)
                r = -1;
            else
                r = 0;

            avahi_free(ma);
            avahi_free(mb);

            return r;
        }

        case AVAHI_DNS_TYPE_A:
            return memcmp(&a->data.a.address, &b->data.a.address, sizeof(AvahiIPv4Address));

        case AVAHI_DNS_TYPE_AAAA:
            return memcmp(&a->data.aaaa.address, &b->data.aaaa.address, sizeof(AvahiIPv6Address));

        default:
            return lexicographical_memcmp(a->data.generic.data, a->data.generic.size,
                                          b->data.generic.data, b->data.generic.size);
    }


fail:
    avahi_log_error(__FILE__": Out of memory");
    return -1; /* or whatever ... */
}

int avahi_record_is_goodbye(AvahiRecord *r) {
    assert(r);

    return r->ttl == 0;
}

int avahi_key_is_valid(AvahiKey *k) {
    assert(k);

    if (!avahi_is_valid_domain_name(k->name))
        return 0;

    return 1;
}

int avahi_record_is_valid(AvahiRecord *r) {
    assert(r);

    if (!avahi_key_is_valid(r->key))
        return 0;

    switch (r->key->type) {

        case AVAHI_DNS_TYPE_PTR:
        case AVAHI_DNS_TYPE_CNAME:
        case AVAHI_DNS_TYPE_NS:
            return avahi_is_valid_domain_name(r->data.ptr.name);

        case AVAHI_DNS_TYPE_SRV:
            return avahi_is_valid_domain_name(r->data.srv.name);

        case AVAHI_DNS_TYPE_HINFO:
            return
                strlen(r->data.hinfo.os) <= 255 &&
                strlen(r->data.hinfo.cpu) <= 255;

        case AVAHI_DNS_TYPE_TXT: {

            AvahiStringList *strlst;

            for (strlst = r->data.txt.string_list; strlst; strlst = strlst->next)
                if (strlst->size > 255 || strlst->size <= 0)
                    return 0;

            return 1;
        }
    }

    return 1;
}

static AvahiAddress *get_address(const AvahiRecord *r, AvahiAddress *a) {
    assert(r);

    switch (r->key->type) {
        case AVAHI_DNS_TYPE_A:
            a->proto = AVAHI_PROTO_INET;
            a->data.ipv4 = r->data.a.address;
            break;

        case AVAHI_DNS_TYPE_AAAA:
            a->proto = AVAHI_PROTO_INET6;
            a->data.ipv6 = r->data.aaaa.address;
            break;

        default:
            return NULL;
    }

    return a;
}

int avahi_record_is_link_local_address(const AvahiRecord *r) {
    AvahiAddress a;

    assert(r);

    if (!get_address(r, &a))
        return 0;

    return avahi_address_is_link_local(&a);
}
