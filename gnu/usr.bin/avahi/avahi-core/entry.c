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
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <arpa/inet.h>

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <avahi-common/domain.h>
#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/domain.h>

#include "internal.h"
#include "iface.h"
#include "socket.h"
#include "browse.h"
#include "log.h"
#include "util.h"
#include "dns-srv-rr.h"
#include "rr-util.h"
#include "domain-util.h"

static void transport_flags_from_domain(AvahiServer *s, AvahiPublishFlags *flags, const char *domain) {
    assert(flags);
    assert(domain);

    assert(!((*flags & AVAHI_PUBLISH_USE_MULTICAST) && (*flags & AVAHI_PUBLISH_USE_WIDE_AREA)));

    if (*flags & (AVAHI_PUBLISH_USE_MULTICAST|AVAHI_PUBLISH_USE_WIDE_AREA))
        return;

    if (!s->wide_area_lookup_engine ||
        !avahi_wide_area_has_servers(s->wide_area_lookup_engine) ||
        avahi_domain_ends_with(domain, AVAHI_MDNS_SUFFIX_LOCAL) ||
        avahi_domain_ends_with(domain, AVAHI_MDNS_SUFFIX_ADDR_IPV4) ||
        avahi_domain_ends_with(domain, AVAHI_MDNS_SUFFIX_ADDR_IPV6))
        *flags |= AVAHI_PUBLISH_USE_MULTICAST;
    else
        *flags |= AVAHI_PUBLISH_USE_WIDE_AREA;
}

void avahi_entry_free(AvahiServer*s, AvahiEntry *e) {
    AvahiEntry *t;

    assert(s);
    assert(e);

    avahi_goodbye_entry(s, e, 1, 1);

    /* Remove from linked list */
    AVAHI_LLIST_REMOVE(AvahiEntry, entries, s->entries, e);

    /* Remove from hash table indexed by name */
    t = avahi_hashmap_lookup(s->entries_by_key, e->record->key);
    AVAHI_LLIST_REMOVE(AvahiEntry, by_key, t, e);
    if (t)
        avahi_hashmap_replace(s->entries_by_key, t->record->key, t);
    else
        avahi_hashmap_remove(s->entries_by_key, e->record->key);

    /* Remove from associated group */
    if (e->group)
        AVAHI_LLIST_REMOVE(AvahiEntry, by_group, e->group->entries, e);

    avahi_record_unref(e->record);
    avahi_free(e);
}

void avahi_entry_group_free(AvahiServer *s, AvahiSEntryGroup *g) {
    assert(s);
    assert(g);

    while (g->entries)
        avahi_entry_free(s, g->entries);

    if (g->register_time_event)
        avahi_time_event_free(g->register_time_event);

    AVAHI_LLIST_REMOVE(AvahiSEntryGroup, groups, s->groups, g);
    avahi_free(g);
}

void avahi_cleanup_dead_entries(AvahiServer *s) {
    assert(s);

    if (s->need_group_cleanup) {
        AvahiSEntryGroup *g, *next;

        for (g = s->groups; g; g = next) {
            next = g->groups_next;

            if (g->dead)
                avahi_entry_group_free(s, g);
        }

        s->need_group_cleanup = 0;
    }

    if (s->need_entry_cleanup) {
        AvahiEntry *e, *next;

        for (e = s->entries; e; e = next) {
            next = e->entries_next;

            if (e->dead)
                avahi_entry_free(s, e);
        }

        s->need_entry_cleanup = 0;
    }

    if (s->need_browser_cleanup)
        avahi_browser_cleanup(s);

    if (s->cleanup_time_event) {
        avahi_time_event_free(s->cleanup_time_event);
        s->cleanup_time_event = NULL;
    }
}

static int check_record_conflict(AvahiServer *s, AvahiIfIndex interface, AvahiProtocol protocol, AvahiRecord *r, AvahiPublishFlags flags) {
    AvahiEntry *e;

    assert(s);
    assert(r);

    for (e = avahi_hashmap_lookup(s->entries_by_key, r->key); e; e = e->by_key_next) {
        if (e->dead)
            continue;

        if (!(flags & AVAHI_PUBLISH_UNIQUE) && !(e->flags & AVAHI_PUBLISH_UNIQUE))
            continue;

        if ((flags & AVAHI_PUBLISH_ALLOW_MULTIPLE) && (e->flags & AVAHI_PUBLISH_ALLOW_MULTIPLE) )
            continue;

        if (avahi_record_equal_no_ttl(r, e->record)) {
            /* The records are the same, not a conflict in any case */
            continue;
        }

        if ((interface <= 0 ||
             e->interface <= 0 ||
             e->interface == interface) &&
            (protocol == AVAHI_PROTO_UNSPEC ||
             e->protocol == AVAHI_PROTO_UNSPEC ||
             e->protocol == protocol))

            return -1;
    }

    return 0;
}

static AvahiEntry * server_add_internal(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    AvahiRecord *r) {

    AvahiEntry *e;

    assert(s);
    assert(r);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, s->state != AVAHI_SERVER_FAILURE && s->state != AVAHI_SERVER_INVALID, AVAHI_ERR_BAD_STATE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, AVAHI_FLAGS_VALID(
                                         flags,
                                         AVAHI_PUBLISH_NO_ANNOUNCE|
                                         AVAHI_PUBLISH_NO_PROBE|
                                         AVAHI_PUBLISH_UNIQUE|
                                         AVAHI_PUBLISH_ALLOW_MULTIPLE|
                                         AVAHI_PUBLISH_UPDATE|
                                         AVAHI_PUBLISH_USE_WIDE_AREA|
                                         AVAHI_PUBLISH_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, avahi_is_valid_domain_name(r->key->name), AVAHI_ERR_INVALID_HOST_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, r->ttl != 0, AVAHI_ERR_INVALID_TTL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, !avahi_key_is_pattern(r->key), AVAHI_ERR_IS_PATTERN);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, avahi_record_is_valid(r), AVAHI_ERR_INVALID_RECORD);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, r->key->clazz == AVAHI_DNS_CLASS_IN, AVAHI_ERR_INVALID_DNS_CLASS);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s,
                                     (r->key->type != 0) &&
                                     (r->key->type != AVAHI_DNS_TYPE_ANY) &&
                                     (r->key->type != AVAHI_DNS_TYPE_OPT) &&
                                     (r->key->type != AVAHI_DNS_TYPE_TKEY) &&
                                     (r->key->type != AVAHI_DNS_TYPE_TSIG) &&
                                     (r->key->type != AVAHI_DNS_TYPE_IXFR) &&
                                     (r->key->type != AVAHI_DNS_TYPE_AXFR), AVAHI_ERR_INVALID_DNS_TYPE);

    transport_flags_from_domain(s, &flags, r->key->name);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, flags & AVAHI_PUBLISH_USE_MULTICAST, AVAHI_ERR_NOT_SUPPORTED);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, !s->config.disable_publishing, AVAHI_ERR_NOT_PERMITTED);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s,
                                     !g ||
                                     (g->state != AVAHI_ENTRY_GROUP_ESTABLISHED && g->state != AVAHI_ENTRY_GROUP_REGISTERING) ||
                                     (flags & AVAHI_PUBLISH_UPDATE), AVAHI_ERR_BAD_STATE);

    if (flags & AVAHI_PUBLISH_UPDATE) {
        AvahiRecord *old_record;
        int is_first = 1;

        /* Update and existing record */

        /* Find the first matching entry */
        for (e = avahi_hashmap_lookup(s->entries_by_key, r->key); e; e = e->by_key_next) {
            if (!e->dead && e->group == g && e->interface == interface && e->protocol == protocol)
                break;

            is_first = 0;
        }

        /* Hmm, nothing found? */
        if (!e) {
            avahi_server_set_errno(s, AVAHI_ERR_NOT_FOUND);
            return NULL;
        }

        /* Update the entry */
        old_record = e->record;
        e->record = avahi_record_ref(r);
        e->flags = flags;

        /* Announce our changes when needed */
        if (!avahi_record_equal_no_ttl(old_record, r) && (!g || g->state != AVAHI_ENTRY_GROUP_UNCOMMITED)) {

            /* Remove the old entry from all caches, if needed */
            if (!(e->flags & AVAHI_PUBLISH_UNIQUE))
                avahi_goodbye_entry(s, e, 1, 0);

            /* Reannounce our updated entry */
            avahi_reannounce_entry(s, e);
        }

        /* If we were the first entry in the list, we need to update the key */
        if (is_first)
            avahi_hashmap_replace(s->entries_by_key, e->record->key, e);

        avahi_record_unref(old_record);

    } else {
        AvahiEntry *t;

        /* Add a new record */

        if (check_record_conflict(s, interface, protocol, r, flags) < 0) {
            avahi_server_set_errno(s, AVAHI_ERR_COLLISION);
            return NULL;
        }

        if (!(e = avahi_new(AvahiEntry, 1))) {
            avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
            return NULL;
        }

        e->server = s;
        e->record = avahi_record_ref(r);
        e->group = g;
        e->interface = interface;
        e->protocol = protocol;
        e->flags = flags;
        e->dead = 0;

        AVAHI_LLIST_HEAD_INIT(AvahiAnnouncer, e->announcers);

        AVAHI_LLIST_PREPEND(AvahiEntry, entries, s->entries, e);

        /* Insert into hash table indexed by name */
        t = avahi_hashmap_lookup(s->entries_by_key, e->record->key);
        AVAHI_LLIST_PREPEND(AvahiEntry, by_key, t, e);
        avahi_hashmap_replace(s->entries_by_key, e->record->key, t);

        /* Insert into group list */
        if (g)
            AVAHI_LLIST_PREPEND(AvahiEntry, by_group, g->entries, e);

        avahi_announce_entry(s, e);
    }

    return e;
}

int avahi_server_add(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    AvahiRecord *r) {

    if (!server_add_internal(s, g, interface, protocol, flags, r))
        return avahi_server_errno(s);

    return AVAHI_OK;
}

const AvahiRecord *avahi_server_iterate(AvahiServer *s, AvahiSEntryGroup *g, void **state) {
    AvahiEntry **e = (AvahiEntry**) state;
    assert(s);
    assert(e);

    if (!*e)
        *e = g ? g->entries : s->entries;

    while (*e && (*e)->dead)
        *e = g ? (*e)->by_group_next : (*e)->entries_next;

    if (!*e)
        return NULL;

    return avahi_record_ref((*e)->record);
}

int avahi_server_dump(AvahiServer *s, AvahiDumpCallback callback, void* userdata) {
    AvahiEntry *e;

    assert(s);
    assert(callback);

    callback(";;; ZONE DUMP FOLLOWS ;;;", userdata);

    for (e = s->entries; e; e = e->entries_next) {
        char *t;
        char ln[256];

        if (e->dead)
            continue;

        if (!(t = avahi_record_to_string(e->record)))
            return avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);

        snprintf(ln, sizeof(ln), "%s ; iface=%i proto=%i", t, e->interface, e->protocol);
        avahi_free(t);

        callback(ln, userdata);
    }

    avahi_dump_caches(s->monitor, callback, userdata);

    if (s->wide_area_lookup_engine)
        avahi_wide_area_cache_dump(s->wide_area_lookup_engine, callback, userdata);
    return AVAHI_OK;
}

static AvahiEntry *server_add_ptr_internal(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    uint32_t ttl,
    const char *name,
    const char *dest) {

    AvahiRecord *r;
    AvahiEntry *e;

    assert(s);
    assert(dest);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, !name || avahi_is_valid_domain_name(name), AVAHI_ERR_INVALID_HOST_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, avahi_is_valid_domain_name(dest), AVAHI_ERR_INVALID_HOST_NAME);

    if (!name)
        name = s->host_name_fqdn;

    if (!(r = avahi_record_new_full(name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_PTR, ttl))) {
        avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    r->data.ptr.name = avahi_normalize_name_strdup(dest);
    e = server_add_internal(s, g, interface, protocol, flags, r);
    avahi_record_unref(r);
    return e;
}

int avahi_server_add_ptr(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    uint32_t ttl,
    const char *name,
    const char *dest) {

    AvahiEntry *e;

    assert(s);

    if (!(e = server_add_ptr_internal(s, g, interface, protocol, flags, ttl, name, dest)))
        return avahi_server_errno(s);

    return AVAHI_OK;
}

int avahi_server_add_address(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    AvahiAddress *a) {

    char n[AVAHI_DOMAIN_NAME_MAX];
    int ret = AVAHI_OK;
    AvahiEntry *entry = NULL, *reverse = NULL;
    AvahiRecord  *r;

    assert(s);
    assert(a);

    AVAHI_CHECK_VALIDITY(s, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY(s, AVAHI_PROTO_VALID(protocol) && AVAHI_PROTO_VALID(a->proto), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY(s, AVAHI_FLAGS_VALID(flags,
                                              AVAHI_PUBLISH_NO_REVERSE|
                                              AVAHI_PUBLISH_NO_ANNOUNCE|
                                              AVAHI_PUBLISH_NO_PROBE|
                                              AVAHI_PUBLISH_UPDATE|
                                              AVAHI_PUBLISH_USE_WIDE_AREA|
                                              AVAHI_PUBLISH_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY(s, !name || avahi_is_valid_fqdn(name), AVAHI_ERR_INVALID_HOST_NAME);

    /* Prepare the host naem */

    if (!name)
        name = s->host_name_fqdn;
    else {
        AVAHI_ASSERT_TRUE(avahi_normalize_name(name, n, sizeof(n)));
        name = n;
    }

    transport_flags_from_domain(s, &flags, name);
    AVAHI_CHECK_VALIDITY(s, flags & AVAHI_PUBLISH_USE_MULTICAST, AVAHI_ERR_NOT_SUPPORTED);

    /* Create the A/AAAA record */

    if (a->proto == AVAHI_PROTO_INET) {

        if (!(r = avahi_record_new_full(name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_A, AVAHI_DEFAULT_TTL_HOST_NAME))) {
            ret = avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
            goto finish;
        }

        r->data.a.address = a->data.ipv4;

    } else {
        assert(a->proto == AVAHI_PROTO_INET6);

        if (!(r = avahi_record_new_full(name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_AAAA, AVAHI_DEFAULT_TTL_HOST_NAME))) {
            ret = avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
            goto finish;
        }

        r->data.aaaa.address = a->data.ipv6;
    }

    entry = server_add_internal(s, g, interface, protocol, (flags & ~ AVAHI_PUBLISH_NO_REVERSE) | AVAHI_PUBLISH_UNIQUE | AVAHI_PUBLISH_ALLOW_MULTIPLE, r);
    avahi_record_unref(r);

    if (!entry) {
        ret = avahi_server_errno(s);
        goto finish;
    }

    /* Create the reverse lookup entry */

    if (!(flags & AVAHI_PUBLISH_NO_REVERSE)) {
        char reverse_n[AVAHI_DOMAIN_NAME_MAX];
        avahi_reverse_lookup_name(a, reverse_n, sizeof(reverse_n));

        if (!(reverse = server_add_ptr_internal(s, g, interface, protocol, flags | AVAHI_PUBLISH_UNIQUE, AVAHI_DEFAULT_TTL_HOST_NAME, reverse_n, name))) {
            ret = avahi_server_errno(s);
            goto finish;
        }
    }

finish:

    if (ret != AVAHI_OK && !(flags & AVAHI_PUBLISH_UPDATE)) {
        if (entry)
            avahi_entry_free(s, entry);
        if (reverse)
            avahi_entry_free(s, reverse);
    }

    return ret;
}

static AvahiEntry *server_add_txt_strlst_nocopy(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    uint32_t ttl,
    const char *name,
    AvahiStringList *strlst) {

    AvahiRecord *r;
    AvahiEntry *e;

    assert(s);

    if (!(r = avahi_record_new_full(name ? name : s->host_name_fqdn, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_TXT, ttl))) {
        avahi_string_list_free(strlst);
        avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    r->data.txt.string_list = strlst;
    e = server_add_internal(s, g, interface, protocol, flags, r);
    avahi_record_unref(r);

    return e;
}

static AvahiStringList *add_magic_cookie(
    AvahiServer *s,
    AvahiStringList *strlst) {

    assert(s);

    if (!s->config.add_service_cookie)
        return strlst;

    if (avahi_string_list_find(strlst, AVAHI_SERVICE_COOKIE))
        /* This string list already contains a magic cookie */
        return strlst;

    return avahi_string_list_add_printf(strlst, AVAHI_SERVICE_COOKIE"=%u", s->local_service_cookie);
}

static int server_add_service_strlst_nocopy(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *host,
    uint16_t port,
    AvahiStringList *strlst) {

    char ptr_name[AVAHI_DOMAIN_NAME_MAX], svc_name[AVAHI_DOMAIN_NAME_MAX], enum_ptr[AVAHI_DOMAIN_NAME_MAX], *h = NULL;
    AvahiRecord *r = NULL;
    int ret = AVAHI_OK;
    AvahiEntry *srv_entry = NULL, *txt_entry = NULL, *ptr_entry = NULL, *enum_entry = NULL;

    assert(s);
    assert(type);
    assert(name);

    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_FLAGS_VALID(flags,
                                                                AVAHI_PUBLISH_NO_COOKIE|
                                                                AVAHI_PUBLISH_UPDATE|
                                                                AVAHI_PUBLISH_USE_WIDE_AREA|
                                                                AVAHI_PUBLISH_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, avahi_is_valid_service_name(name), AVAHI_ERR_INVALID_SERVICE_NAME);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, avahi_is_valid_service_type_strict(type), AVAHI_ERR_INVALID_SERVICE_TYPE);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, !host || avahi_is_valid_fqdn(host), AVAHI_ERR_INVALID_HOST_NAME);

    if (!domain)
        domain = s->domain_name;

    if (!host)
        host = s->host_name_fqdn;

    transport_flags_from_domain(s, &flags, domain);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, flags & AVAHI_PUBLISH_USE_MULTICAST, AVAHI_ERR_NOT_SUPPORTED);

    if (!(h = avahi_normalize_name_strdup(host))) {
        ret = avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if ((ret = avahi_service_name_join(svc_name, sizeof(svc_name), name, type, domain)) < 0 ||
        (ret = avahi_service_name_join(ptr_name, sizeof(ptr_name), NULL, type, domain)) < 0 ||
        (ret = avahi_service_name_join(enum_ptr, sizeof(enum_ptr), NULL, "_services._dns-sd._udp", domain)) < 0) {
        avahi_server_set_errno(s, ret);
        goto fail;
    }

    /* Add service enumeration PTR record */

    if (!(ptr_entry = server_add_ptr_internal(s, g, interface, protocol, 0, AVAHI_DEFAULT_TTL, ptr_name, svc_name))) {
        ret = avahi_server_errno(s);
        goto fail;
    }

    /* Add SRV record */

    if (!(r = avahi_record_new_full(svc_name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_SRV, AVAHI_DEFAULT_TTL_HOST_NAME))) {
        ret = avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    r->data.srv.priority = 0;
    r->data.srv.weight = 0;
    r->data.srv.port = port;
    r->data.srv.name = h;
    h = NULL;
    srv_entry = server_add_internal(s, g, interface, protocol, AVAHI_PUBLISH_UNIQUE, r);
    avahi_record_unref(r);

    if (!srv_entry) {
        ret = avahi_server_errno(s);
        goto fail;
    }

    /* Add TXT record */

    if (!(flags & AVAHI_PUBLISH_NO_COOKIE))
        strlst = add_magic_cookie(s, strlst);

    txt_entry = server_add_txt_strlst_nocopy(s, g, interface, protocol, AVAHI_PUBLISH_UNIQUE, AVAHI_DEFAULT_TTL, svc_name, strlst);
    strlst = NULL;

    if (!txt_entry) {
        ret = avahi_server_errno(s);
        goto fail;
    }

    /* Add service type enumeration record */

    if (!(enum_entry = server_add_ptr_internal(s, g, interface, protocol, 0, AVAHI_DEFAULT_TTL, enum_ptr, ptr_name))) {
        ret = avahi_server_errno(s);
        goto fail;
    }

fail:
    if (ret != AVAHI_OK && !(flags & AVAHI_PUBLISH_UPDATE)) {
        if (srv_entry)
            avahi_entry_free(s, srv_entry);
        if (txt_entry)
            avahi_entry_free(s, txt_entry);
        if (ptr_entry)
            avahi_entry_free(s, ptr_entry);
        if (enum_entry)
            avahi_entry_free(s, enum_entry);
    }

    avahi_string_list_free(strlst);
    avahi_free(h);

    return ret;
}

int avahi_server_add_service_strlst(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *host,
    uint16_t port,
    AvahiStringList *strlst) {

    assert(s);
    assert(type);
    assert(name);

    return server_add_service_strlst_nocopy(s, g, interface, protocol, flags, name, type, domain, host, port, avahi_string_list_copy(strlst));
}

int avahi_server_add_service(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *host,
    uint16_t port,
    ... ){

    va_list va;
    int ret;

    va_start(va, port);
    ret = server_add_service_strlst_nocopy(s, g, interface, protocol, flags, name, type, domain, host, port, avahi_string_list_new_va(va));
    va_end(va);

    return ret;
}

static int server_update_service_txt_strlst_nocopy(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    AvahiStringList *strlst) {

    char svc_name[AVAHI_DOMAIN_NAME_MAX];
    int ret = AVAHI_OK;
    AvahiEntry *e;

    assert(s);
    assert(type);
    assert(name);

    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_FLAGS_VALID(flags,
                                                                AVAHI_PUBLISH_NO_COOKIE|
                                                                AVAHI_PUBLISH_USE_WIDE_AREA|
                                                                AVAHI_PUBLISH_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, avahi_is_valid_service_name(name), AVAHI_ERR_INVALID_SERVICE_NAME);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, avahi_is_valid_service_type_strict(type), AVAHI_ERR_INVALID_SERVICE_TYPE);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);

    if (!domain)
        domain = s->domain_name;

    transport_flags_from_domain(s, &flags, domain);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, flags & AVAHI_PUBLISH_USE_MULTICAST, AVAHI_ERR_NOT_SUPPORTED);

    if ((ret = avahi_service_name_join(svc_name, sizeof(svc_name), name, type, domain)) < 0) {
        avahi_server_set_errno(s, ret);
        goto fail;
    }

    /* Add TXT record */
    if (!(flags & AVAHI_PUBLISH_NO_COOKIE))
        strlst = add_magic_cookie(s, strlst);

    e = server_add_txt_strlst_nocopy(s, g, interface, protocol, AVAHI_PUBLISH_UNIQUE | AVAHI_PUBLISH_UPDATE, AVAHI_DEFAULT_TTL, svc_name, strlst);
    strlst = NULL;

    if (!e)
        ret = avahi_server_errno(s);

fail:

    avahi_string_list_free(strlst);

    return ret;
}

int avahi_server_update_service_txt_strlst(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    AvahiStringList *strlst) {

    return server_update_service_txt_strlst_nocopy(s, g, interface, protocol, flags, name, type, domain, avahi_string_list_copy(strlst));
}

/** Update the TXT record for a service with the NULL termonate list of strings */
int avahi_server_update_service_txt(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    ...) {

    va_list va;
    int ret;

    va_start(va, domain);
    ret = server_update_service_txt_strlst_nocopy(s, g, interface, protocol, flags, name, type, domain, avahi_string_list_new_va(va));
    va_end(va);

    return ret;
}

int avahi_server_add_service_subtype(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *subtype) {

    int ret = AVAHI_OK;
    char svc_name[AVAHI_DOMAIN_NAME_MAX], ptr_name[AVAHI_DOMAIN_NAME_MAX];

    assert(name);
    assert(type);
    assert(subtype);

    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, AVAHI_FLAGS_VALID(flags, AVAHI_PUBLISH_USE_MULTICAST|AVAHI_PUBLISH_USE_WIDE_AREA), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, avahi_is_valid_service_name(name), AVAHI_ERR_INVALID_SERVICE_NAME);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, avahi_is_valid_service_type_strict(type), AVAHI_ERR_INVALID_SERVICE_TYPE);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, avahi_is_valid_service_subtype(subtype), AVAHI_ERR_INVALID_SERVICE_SUBTYPE);

    if (!domain)
        domain = s->domain_name;

    transport_flags_from_domain(s, &flags, domain);
    AVAHI_CHECK_VALIDITY_SET_RET_GOTO_FAIL(s, flags & AVAHI_PUBLISH_USE_MULTICAST, AVAHI_ERR_NOT_SUPPORTED);

    if ((ret = avahi_service_name_join(svc_name, sizeof(svc_name), name, type, domain)) < 0 ||
        (ret = avahi_service_name_join(ptr_name, sizeof(ptr_name), NULL, subtype, domain)) < 0) {
        avahi_server_set_errno(s, ret);
        goto fail;
    }

    if ((ret = avahi_server_add_ptr(s, g, interface, protocol, 0, AVAHI_DEFAULT_TTL, ptr_name, svc_name)) < 0)
        goto fail;

fail:

    return ret;
}

static void hexstring(char *s, size_t sl, const void *p, size_t pl) {
    static const char hex[] = "0123456789abcdef";
    int b = 0;
    const uint8_t *k = p;

    while (sl > 1 && pl > 0) {
        *(s++) = hex[(b ? *k : *k >> 4) & 0xF];

        if (b) {
            k++;
            pl--;
        }

        b = !b;

        sl--;
    }

    if (sl > 0)
        *s = 0;
}

static AvahiEntry *server_add_dns_server_name(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *domain,
    AvahiDNSServerType type,
    const char *name,
    uint16_t port /** should be 53 */) {

    AvahiEntry *e;
    char t[AVAHI_DOMAIN_NAME_MAX], normalized_d[AVAHI_DOMAIN_NAME_MAX], *n;

    AvahiRecord *r;

    assert(s);
    assert(name);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, AVAHI_FLAGS_VALID(flags, AVAHI_PUBLISH_USE_WIDE_AREA|AVAHI_PUBLISH_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, type == AVAHI_DNS_SERVER_UPDATE || type == AVAHI_DNS_SERVER_RESOLVE, AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, port != 0, AVAHI_ERR_INVALID_PORT);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, avahi_is_valid_fqdn(name), AVAHI_ERR_INVALID_HOST_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);

    if (!domain)
        domain = s->domain_name;

    transport_flags_from_domain(s, &flags, domain);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(s, flags & AVAHI_PUBLISH_USE_MULTICAST, AVAHI_ERR_NOT_SUPPORTED);

    if (!(n = avahi_normalize_name_strdup(name))) {
        avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    AVAHI_ASSERT_TRUE(avahi_normalize_name(domain, normalized_d, sizeof(normalized_d)));

    snprintf(t, sizeof(t), "%s.%s", type == AVAHI_DNS_SERVER_RESOLVE ? "_domain._udp" : "_dns-update._udp", normalized_d);

    if (!(r = avahi_record_new_full(t, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_SRV, AVAHI_DEFAULT_TTL_HOST_NAME))) {
        avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
        avahi_free(n);
        return NULL;
    }

    r->data.srv.priority = 0;
    r->data.srv.weight = 0;
    r->data.srv.port = port;
    r->data.srv.name = n;
    e = server_add_internal(s, g, interface, protocol, 0, r);
    avahi_record_unref(r);

    return e;
}

int avahi_server_add_dns_server_address(
    AvahiServer *s,
    AvahiSEntryGroup *g,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *domain,
    AvahiDNSServerType type,
    const AvahiAddress *address,
    uint16_t port /** should be 53 */) {

    AvahiRecord *r;
    char n[64], h[64];
    AvahiEntry *a_entry, *s_entry;

    assert(s);
    assert(address);

    AVAHI_CHECK_VALIDITY(s, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY(s, AVAHI_PROTO_VALID(protocol) && AVAHI_PROTO_VALID(address->proto), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY(s, AVAHI_FLAGS_VALID(flags, AVAHI_PUBLISH_USE_MULTICAST|AVAHI_PUBLISH_USE_WIDE_AREA), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY(s, type == AVAHI_DNS_SERVER_UPDATE || type == AVAHI_DNS_SERVER_RESOLVE, AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY(s, port != 0, AVAHI_ERR_INVALID_PORT);
    AVAHI_CHECK_VALIDITY(s, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);

    if (!domain)
        domain = s->domain_name;

    transport_flags_from_domain(s, &flags, domain);
    AVAHI_CHECK_VALIDITY(s, flags & AVAHI_PUBLISH_USE_MULTICAST, AVAHI_ERR_NOT_SUPPORTED);

    if (address->proto == AVAHI_PROTO_INET) {
        hexstring(h, sizeof(h), &address->data, sizeof(AvahiIPv4Address));
        snprintf(n, sizeof(n), "ip-%s.%s", h, domain);
        r = avahi_record_new_full(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_A, AVAHI_DEFAULT_TTL_HOST_NAME);
        r->data.a.address = address->data.ipv4;
    } else {
        hexstring(h, sizeof(h), &address->data, sizeof(AvahiIPv6Address));
        snprintf(n, sizeof(n), "ip6-%s.%s", h, domain);
        r = avahi_record_new_full(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_AAAA, AVAHI_DEFAULT_TTL_HOST_NAME);
        r->data.aaaa.address = address->data.ipv6;
    }

    if (!r)
        return avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);

    a_entry = server_add_internal(s, g, interface, protocol, AVAHI_PUBLISH_UNIQUE | AVAHI_PUBLISH_ALLOW_MULTIPLE, r);
    avahi_record_unref(r);

    if (!a_entry)
        return avahi_server_errno(s);

    if (!(s_entry = server_add_dns_server_name(s, g, interface, protocol, flags, domain, type, n, port))) {
        if (!(flags & AVAHI_PUBLISH_UPDATE))
            avahi_entry_free(s, a_entry);
        return avahi_server_errno(s);
    }

    return AVAHI_OK;
}

void avahi_s_entry_group_change_state(AvahiSEntryGroup *g, AvahiEntryGroupState state) {
    assert(g);

    if (g->state == state)
        return;

    assert(state <= AVAHI_ENTRY_GROUP_COLLISION);

    if (g->state == AVAHI_ENTRY_GROUP_ESTABLISHED) {

        /* If the entry group was established for a time longer then
         * 5s, reset the establishment trial counter */

        if (avahi_age(&g->established_at) > 5000000)
            g->n_register_try = 0;
    } else if (g->state == AVAHI_ENTRY_GROUP_REGISTERING) {
        if (g->register_time_event) {
            avahi_time_event_free(g->register_time_event);
            g->register_time_event = NULL;
        }
    }

    if (state == AVAHI_ENTRY_GROUP_ESTABLISHED)

        /* If the entry group is now established, remember the time
         * this happened */

        gettimeofday(&g->established_at, NULL);

    g->state = state;

    if (g->callback)
        g->callback(g->server, g, state, g->userdata);
}

AvahiSEntryGroup *avahi_s_entry_group_new(AvahiServer *s, AvahiSEntryGroupCallback callback, void* userdata) {
    AvahiSEntryGroup *g;

    assert(s);

    if (!(g = avahi_new(AvahiSEntryGroup, 1))) {
        avahi_server_set_errno(s, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    g->server = s;
    g->callback = callback;
    g->userdata = userdata;
    g->dead = 0;
    g->state = AVAHI_ENTRY_GROUP_UNCOMMITED;
    g->n_probing = 0;
    g->n_register_try = 0;
    g->register_time_event = NULL;
    g->register_time.tv_sec = 0;
    g->register_time.tv_usec = 0;
    AVAHI_LLIST_HEAD_INIT(AvahiEntry, g->entries);

    AVAHI_LLIST_PREPEND(AvahiSEntryGroup, groups, s->groups, g);
    return g;
}

static void cleanup_time_event_callback(AVAHI_GCC_UNUSED AvahiTimeEvent *e, void* userdata) {
    AvahiServer *s = userdata;

    assert(s);

    avahi_cleanup_dead_entries(s);
}

static void schedule_cleanup(AvahiServer *s) {
    struct timeval tv;

    assert(s);

    if (!s->cleanup_time_event)
        s->cleanup_time_event = avahi_time_event_new(s->time_event_queue, avahi_elapse_time(&tv, 1000, 0), &cleanup_time_event_callback, s);
}

void avahi_s_entry_group_free(AvahiSEntryGroup *g) {
    AvahiEntry *e;

    assert(g);
    assert(g->server);

    for (e = g->entries; e; e = e->by_group_next) {
        if (!e->dead) {
            avahi_goodbye_entry(g->server, e, 1, 1);
            e->dead = 1;
        }
    }

    if (g->register_time_event) {
        avahi_time_event_free(g->register_time_event);
        g->register_time_event = NULL;
    }

    g->dead = 1;

    g->server->need_group_cleanup = 1;
    g->server->need_entry_cleanup = 1;

    schedule_cleanup(g->server);
}

static void entry_group_commit_real(AvahiSEntryGroup *g) {
    assert(g);

    gettimeofday(&g->register_time, NULL);

    avahi_s_entry_group_change_state(g, AVAHI_ENTRY_GROUP_REGISTERING);

    if (g->dead)
        return;

    avahi_announce_group(g->server, g);
    avahi_s_entry_group_check_probed(g, 0);
}

static void entry_group_register_time_event_callback(AVAHI_GCC_UNUSED AvahiTimeEvent *e, void* userdata) {
    AvahiSEntryGroup *g = userdata;
    assert(g);

    avahi_time_event_free(g->register_time_event);
    g->register_time_event = NULL;

    /* Holdoff time passed, so let's start probing */
    entry_group_commit_real(g);
}

int avahi_s_entry_group_commit(AvahiSEntryGroup *g) {
    struct timeval now;

    assert(g);
    assert(!g->dead);

    if (g->state != AVAHI_ENTRY_GROUP_UNCOMMITED && g->state != AVAHI_ENTRY_GROUP_COLLISION)
        return avahi_server_set_errno(g->server, AVAHI_ERR_BAD_STATE);

    if (avahi_s_entry_group_is_empty(g))
        return avahi_server_set_errno(g->server, AVAHI_ERR_IS_EMPTY);

    g->n_register_try++;

    avahi_timeval_add(&g->register_time,
                      1000*(g->n_register_try >= AVAHI_RR_RATE_LIMIT_COUNT ?
                            AVAHI_RR_HOLDOFF_MSEC_RATE_LIMIT :
                            AVAHI_RR_HOLDOFF_MSEC));

    gettimeofday(&now, NULL);

    if (avahi_timeval_compare(&g->register_time, &now) <= 0) {

        /* Holdoff time passed, so let's start probing */
        entry_group_commit_real(g);
    } else {

         /* Holdoff time has not yet passed, so let's wait */
        assert(!g->register_time_event);
        g->register_time_event = avahi_time_event_new(g->server->time_event_queue, &g->register_time, entry_group_register_time_event_callback, g);

        avahi_s_entry_group_change_state(g, AVAHI_ENTRY_GROUP_REGISTERING);
    }

    return AVAHI_OK;
}

void avahi_s_entry_group_reset(AvahiSEntryGroup *g) {
    AvahiEntry *e;
    assert(g);

    for (e = g->entries; e; e = e->by_group_next) {
        if (!e->dead) {
            avahi_goodbye_entry(g->server, e, 1, 1);
            e->dead = 1;
        }
    }
    g->server->need_entry_cleanup = 1;

    g->n_probing = 0;

    avahi_s_entry_group_change_state(g, AVAHI_ENTRY_GROUP_UNCOMMITED);

    schedule_cleanup(g->server);
}

int avahi_entry_is_commited(AvahiEntry *e) {
    assert(e);
    assert(!e->dead);

    return !e->group ||
        e->group->state == AVAHI_ENTRY_GROUP_REGISTERING ||
        e->group->state == AVAHI_ENTRY_GROUP_ESTABLISHED;
}

AvahiEntryGroupState avahi_s_entry_group_get_state(AvahiSEntryGroup *g) {
    assert(g);
    assert(!g->dead);

    return g->state;
}

void avahi_s_entry_group_set_data(AvahiSEntryGroup *g, void* userdata) {
    assert(g);

    g->userdata = userdata;
}

void* avahi_s_entry_group_get_data(AvahiSEntryGroup *g) {
    assert(g);

    return g->userdata;
}

int avahi_s_entry_group_is_empty(AvahiSEntryGroup *g) {
    AvahiEntry *e;
    assert(g);

    /* Look for an entry that is not dead */
    for (e = g->entries; e; e = e->by_group_next)
        if (!e->dead)
            return 0;

    return 1;
}
