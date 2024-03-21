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

#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/domain.h>
#include <avahi-common/rlist.h>
#include <avahi-common/address.h>

#include "browse.h"
#include "log.h"
#include "querier.h"
#include "domain-util.h"
#include "rr-util.h"

#define AVAHI_LOOKUPS_PER_BROWSER_MAX 15

struct AvahiSRBLookup {
    AvahiSRecordBrowser *record_browser;

    unsigned ref;

    AvahiIfIndex interface;
    AvahiProtocol protocol;
    AvahiLookupFlags flags;

    AvahiKey *key;

    AvahiWideAreaLookup *wide_area;
    AvahiMulticastLookup *multicast;

    AvahiRList *cname_lookups;

    AVAHI_LLIST_FIELDS(AvahiSRBLookup, lookups);
};

static void lookup_handle_cname(AvahiSRBLookup *l, AvahiIfIndex interface, AvahiProtocol protocol, AvahiLookupFlags flags, AvahiRecord *r);
static void lookup_drop_cname(AvahiSRBLookup *l, AvahiIfIndex interface, AvahiProtocol protocol, AvahiLookupFlags flags, AvahiRecord *r);

static void transport_flags_from_domain(AvahiServer *s, AvahiLookupFlags *flags, const char *domain) {
    assert(flags);
    assert(domain);

    assert(!((*flags & AVAHI_LOOKUP_USE_MULTICAST) && (*flags & AVAHI_LOOKUP_USE_WIDE_AREA)));

    if (*flags & (AVAHI_LOOKUP_USE_MULTICAST|AVAHI_LOOKUP_USE_WIDE_AREA))
        return;

    if (!s->wide_area_lookup_engine ||
        !avahi_wide_area_has_servers(s->wide_area_lookup_engine) ||
        avahi_domain_ends_with(domain, AVAHI_MDNS_SUFFIX_LOCAL) ||
        avahi_domain_ends_with(domain, AVAHI_MDNS_SUFFIX_ADDR_IPV4) ||
        avahi_domain_ends_with(domain, AVAHI_MDNS_SUFFIX_ADDR_IPV6))
        *flags |= AVAHI_LOOKUP_USE_MULTICAST;
    else
        *flags |= AVAHI_LOOKUP_USE_WIDE_AREA;
}

static AvahiSRBLookup* lookup_new(
    AvahiSRecordBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiLookupFlags flags,
    AvahiKey *key) {

    AvahiSRBLookup *l;

    assert(b);
    assert(AVAHI_IF_VALID(interface));
    assert(AVAHI_PROTO_VALID(protocol));

    if (b->n_lookups >= AVAHI_LOOKUPS_PER_BROWSER_MAX)
        /* We don't like cyclic CNAMEs */
        return NULL;

    if (!(l = avahi_new(AvahiSRBLookup, 1)))
        return NULL;

    l->ref = 1;
    l->record_browser = b;
    l->interface = interface;
    l->protocol = protocol;
    l->key = avahi_key_ref(key);
    l->wide_area = NULL;
    l->multicast = NULL;
    l->cname_lookups = NULL;
    l->flags = flags;

    transport_flags_from_domain(b->server, &l->flags, key->name);

    AVAHI_LLIST_PREPEND(AvahiSRBLookup, lookups, b->lookups, l);

    b->n_lookups ++;

    return l;
}

static void lookup_unref(AvahiSRBLookup *l) {
    assert(l);
    assert(l->ref >= 1);

    if (--l->ref >= 1)
        return;

    AVAHI_LLIST_REMOVE(AvahiSRBLookup, lookups, l->record_browser->lookups, l);
    l->record_browser->n_lookups --;

    if (l->wide_area) {
        avahi_wide_area_lookup_free(l->wide_area);
        l->wide_area = NULL;
    }

    if (l->multicast) {
        avahi_multicast_lookup_free(l->multicast);
        l->multicast = NULL;
    }

    while (l->cname_lookups) {
        lookup_unref(l->cname_lookups->data);
        l->cname_lookups = avahi_rlist_remove_by_link(l->cname_lookups, l->cname_lookups);
    }

    avahi_key_unref(l->key);
    avahi_free(l);
}

static AvahiSRBLookup* lookup_ref(AvahiSRBLookup *l) {
    assert(l);
    assert(l->ref >= 1);

    l->ref++;
    return l;
}

static AvahiSRBLookup *lookup_find(
    AvahiSRecordBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiLookupFlags flags,
    AvahiKey *key) {

    AvahiSRBLookup *l;

    assert(b);

    for (l = b->lookups; l; l = l->lookups_next) {

        if ((l->interface == AVAHI_IF_UNSPEC || l->interface == interface) &&
            (l->interface == AVAHI_PROTO_UNSPEC || l->protocol == protocol) &&
            l->flags == flags &&
            avahi_key_equal(l->key, key))

            return l;
    }

    return NULL;
}

static void browser_cancel(AvahiSRecordBrowser *b) {
    assert(b);

    if (b->root_lookup) {
        lookup_unref(b->root_lookup);
        b->root_lookup = NULL;
    }

    if (b->defer_time_event) {
        avahi_time_event_free(b->defer_time_event);
        b->defer_time_event = NULL;
    }
}

static void lookup_wide_area_callback(
    AvahiWideAreaLookupEngine *e,
    AvahiBrowserEvent event,
    AvahiLookupResultFlags flags,
    AvahiRecord *r,
    void *userdata) {

    AvahiSRBLookup *l = userdata;
    AvahiSRecordBrowser *b;

    assert(e);
    assert(l);
    assert(l->ref >= 1);

    b = l->record_browser;

    if (b->dead)
        return;

    lookup_ref(l);

    switch (event) {
        case AVAHI_BROWSER_NEW:
            assert(r);

            if (r->key->clazz == AVAHI_DNS_CLASS_IN &&
                r->key->type == AVAHI_DNS_TYPE_CNAME)
                /* It's a CNAME record, so let's follow it. We only follow it on wide area DNS! */
                lookup_handle_cname(l, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_LOOKUP_USE_WIDE_AREA, r);
            else {
                /* It's a normal record, so let's call the user callback */
                assert(avahi_key_equal(r->key, l->key));

                b->callback(b, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, event, r, flags, b->userdata);
            }
            break;

        case AVAHI_BROWSER_REMOVE:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            /* Not defined for wide area DNS */
            abort();

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_FAILURE:

            b->callback(b, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, event, NULL, flags, b->userdata);
            break;
    }

    lookup_unref(l);

}

static void lookup_multicast_callback(
    AvahiMulticastLookupEngine *e,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AvahiLookupResultFlags flags,
    AvahiRecord *r,
    void *userdata) {

    AvahiSRBLookup *l = userdata;
    AvahiSRecordBrowser *b;

    assert(e);
    assert(l);

    b = l->record_browser;

    if (b->dead)
        return;

    lookup_ref(l);

    switch (event) {
        case AVAHI_BROWSER_NEW:
            assert(r);

            if (r->key->clazz == AVAHI_DNS_CLASS_IN &&
                r->key->type == AVAHI_DNS_TYPE_CNAME)
                /* It's a CNAME record, so let's follow it. We allow browsing on both multicast and wide area. */
                lookup_handle_cname(l, interface, protocol, b->flags, r);
            else {
                /* It's a normal record, so let's call the user callback */

                if (avahi_server_is_record_local(b->server, interface, protocol, r))
                    flags |= AVAHI_LOOKUP_RESULT_LOCAL;

                b->callback(b, interface, protocol, event, r, flags, b->userdata);
            }
            break;

        case AVAHI_BROWSER_REMOVE:
            assert(r);

            if (r->key->clazz == AVAHI_DNS_CLASS_IN &&
                r->key->type == AVAHI_DNS_TYPE_CNAME)
                /* It's a CNAME record, so let's drop that query! */
                lookup_drop_cname(l, interface, protocol, 0, r);
            else {
                /* It's a normal record, so let's call the user callback */
                assert(avahi_key_equal(b->key, l->key));

                b->callback(b, interface, protocol, event, r, flags, b->userdata);
            }
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:

            b->callback(b, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, event, NULL, flags, b->userdata);
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_FAILURE:
            /* Not defined for multicast DNS */
            abort();

    }

    lookup_unref(l);
}

static int lookup_start(AvahiSRBLookup *l) {
    assert(l);

    assert(!(l->flags & AVAHI_LOOKUP_USE_WIDE_AREA) != !(l->flags & AVAHI_LOOKUP_USE_MULTICAST));
    assert(!l->wide_area && !l->multicast);

    if (l->flags & AVAHI_LOOKUP_USE_WIDE_AREA) {

        if (!(l->wide_area = avahi_wide_area_lookup_new(l->record_browser->server->wide_area_lookup_engine, l->key, lookup_wide_area_callback, l)))
            return -1;

    } else {
        assert(l->flags & AVAHI_LOOKUP_USE_MULTICAST);

        if (!(l->multicast = avahi_multicast_lookup_new(l->record_browser->server->multicast_lookup_engine, l->interface, l->protocol, l->key, lookup_multicast_callback, l)))
            return -1;
    }

    return 0;
}

static int lookup_scan_cache(AvahiSRBLookup *l) {
    int n = 0;

    assert(l);

    assert(!(l->flags & AVAHI_LOOKUP_USE_WIDE_AREA) != !(l->flags & AVAHI_LOOKUP_USE_MULTICAST));


    if (l->flags & AVAHI_LOOKUP_USE_WIDE_AREA) {
        n = (int) avahi_wide_area_scan_cache(l->record_browser->server->wide_area_lookup_engine, l->key, lookup_wide_area_callback, l);

    } else {
        assert(l->flags & AVAHI_LOOKUP_USE_MULTICAST);
        n = (int) avahi_multicast_lookup_engine_scan_cache(l->record_browser->server->multicast_lookup_engine, l->interface, l->protocol, l->key, lookup_multicast_callback, l);
    }

    return n;
}

static AvahiSRBLookup* lookup_add(AvahiSRecordBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol, AvahiLookupFlags flags, AvahiKey *key) {
    AvahiSRBLookup *l;

    assert(b);
    assert(!b->dead);

    if ((l = lookup_find(b, interface, protocol, flags, key)))
        return lookup_ref(l);

    if (!(l = lookup_new(b, interface, protocol, flags, key)))
        return NULL;

    return l;
}

static int lookup_go(AvahiSRBLookup *l) {
    int n = 0;
    assert(l);

    if (l->record_browser->dead)
        return 0;

    lookup_ref(l);

    /* Browse the cache for the root request */
    n = lookup_scan_cache(l);

    /* Start the lookup */
    if (!l->record_browser->dead && l->ref > 1) {

        if ((l->flags & AVAHI_LOOKUP_USE_MULTICAST) || n == 0)
            /* We do no start a query if the cache contained entries and we're on wide area */

            if (lookup_start(l) < 0)
                n = -1;
    }

    lookup_unref(l);

    return n;
}

static void lookup_handle_cname(AvahiSRBLookup *l, AvahiIfIndex interface, AvahiProtocol protocol, AvahiLookupFlags flags, AvahiRecord *r) {
    AvahiKey *k;
    AvahiSRBLookup *n;

    assert(l);
    assert(r);

    assert(r->key->clazz == AVAHI_DNS_CLASS_IN);
    assert(r->key->type == AVAHI_DNS_TYPE_CNAME);

    k = avahi_key_new(r->data.ptr.name, l->record_browser->key->clazz, l->record_browser->key->type);
    n = lookup_add(l->record_browser, interface, protocol, flags, k);
    avahi_key_unref(k);

    if (!n) {
        avahi_log_debug(__FILE__": Failed to create SRBLookup.");
        return;
    }

    l->cname_lookups = avahi_rlist_prepend(l->cname_lookups, lookup_ref(n));

    lookup_go(n);
    lookup_unref(n);
}

static void lookup_drop_cname(AvahiSRBLookup *l, AvahiIfIndex interface, AvahiProtocol protocol, AvahiLookupFlags flags, AvahiRecord *r) {
    AvahiKey *k;
    AvahiSRBLookup *n = NULL;
    AvahiRList *rl;

    assert(r->key->clazz == AVAHI_DNS_CLASS_IN);
    assert(r->key->type == AVAHI_DNS_TYPE_CNAME);

    k = avahi_key_new(r->data.ptr.name, l->record_browser->key->clazz, l->record_browser->key->type);

    for (rl = l->cname_lookups; rl; rl = rl->rlist_next) {
        n = rl->data;

        assert(n);

        if ((n->interface == AVAHI_IF_UNSPEC || n->interface == interface) &&
            (n->interface == AVAHI_PROTO_UNSPEC || n->protocol == protocol) &&
            n->flags == flags &&
            avahi_key_equal(n->key, k))
            break;
    }

    avahi_key_unref(k);

    if (rl) {
        l->cname_lookups = avahi_rlist_remove_by_link(l->cname_lookups, rl);
        lookup_unref(n);
    }
}

static void defer_callback(AVAHI_GCC_UNUSED AvahiTimeEvent *e, void *userdata) {
    AvahiSRecordBrowser *b = userdata;
    int n;

    assert(b);
    assert(!b->dead);

    /* Remove the defer timeout */
    if (b->defer_time_event) {
        avahi_time_event_free(b->defer_time_event);
        b->defer_time_event = NULL;
    }

    /* Create initial query */
    assert(!b->root_lookup);
    b->root_lookup = lookup_add(b, b->interface, b->protocol, b->flags, b->key);
    assert(b->root_lookup);

    n = lookup_go(b->root_lookup);

    if (b->dead)
        return;

    if (n < 0) {
        /* sending of the initial query failed */

        avahi_server_set_errno(b->server, AVAHI_ERR_FAILURE);

        b->callback(
            b, b->interface, b->protocol, AVAHI_BROWSER_FAILURE, NULL,
            b->flags & AVAHI_LOOKUP_USE_WIDE_AREA ? AVAHI_LOOKUP_RESULT_WIDE_AREA : AVAHI_LOOKUP_RESULT_MULTICAST,
            b->userdata);

        browser_cancel(b);
        return;
    }

    /* Tell the client that we're done with the cache */
    b->callback(
        b, b->interface, b->protocol, AVAHI_BROWSER_CACHE_EXHAUSTED, NULL,
        b->flags & AVAHI_LOOKUP_USE_WIDE_AREA ? AVAHI_LOOKUP_RESULT_WIDE_AREA : AVAHI_LOOKUP_RESULT_MULTICAST,
        b->userdata);

    if (!b->dead && b->root_lookup && b->root_lookup->flags & AVAHI_LOOKUP_USE_WIDE_AREA && n > 0) {

        /* If we do wide area lookups and the the cache contained
         * entries, we assume that it is complete, and tell the user
         * so by firing ALL_FOR_NOW. */

        b->callback(b, b->interface, b->protocol, AVAHI_BROWSER_ALL_FOR_NOW, NULL, AVAHI_LOOKUP_RESULT_WIDE_AREA, b->userdata);
    }
}

void avahi_s_record_browser_restart(AvahiSRecordBrowser *b) {
    assert(b);
    assert(!b->dead);

    browser_cancel(b);

    /* Request a new iteration of the cache scanning */
    if (!b->defer_time_event) {
        b->defer_time_event = avahi_time_event_new(b->server->time_event_queue, NULL, defer_callback, b);
        assert(b->defer_time_event);
    }
}

AvahiSRecordBrowser *avahi_s_record_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiKey *key,
    AvahiLookupFlags flags,
    AvahiSRecordBrowserCallback callback,
    void* userdata) {

    AvahiSRecordBrowser *b;

    assert(server);
    assert(key);
    assert(callback);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, !avahi_key_is_pattern(key), AVAHI_ERR_IS_PATTERN);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, avahi_key_is_valid(key), AVAHI_ERR_INVALID_KEY);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_FLAGS_VALID(flags, AVAHI_LOOKUP_USE_WIDE_AREA|AVAHI_LOOKUP_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, !(flags & AVAHI_LOOKUP_USE_WIDE_AREA) || !(flags & AVAHI_LOOKUP_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);

    if (!(b = avahi_new(AvahiSRecordBrowser, 1))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    b->dead = 0;
    b->defer_time_event = NULL;
    b->server = server;
    b->interface = interface;
    b->protocol = protocol;
    b->key = avahi_key_ref(key);
    b->flags = flags;
    b->callback = callback;
    b->userdata = userdata;
    b->n_lookups = 0;
    AVAHI_LLIST_HEAD_INIT(AvahiSRBLookup, b->lookups);
    b->root_lookup = NULL;

    AVAHI_LLIST_PREPEND(AvahiSRecordBrowser, browser, server->record_browsers, b);

    return b;
}


void avahi_s_record_browser_start_query(AvahiSRecordBrowser *b) {
    assert(b);
    assert(!b->dead);

    /* If the number of lookups greater than zero, the object has already been used.
     * To restart querying, call only avahi_s_record_browser_restart */
    if(b->n_lookups > 0)
        return;

    /* The currently cached entries are scanned a bit later, and than we will start querying, too */
    avahi_s_record_browser_restart(b);
}


void avahi_s_record_browser_free(AvahiSRecordBrowser *b) {
    assert(b);
    assert(!b->dead);

    b->dead = 1;
    b->server->need_browser_cleanup = 1;

    browser_cancel(b);
}

void avahi_s_record_browser_destroy(AvahiSRecordBrowser *b) {
    assert(b);

    browser_cancel(b);

    AVAHI_LLIST_REMOVE(AvahiSRecordBrowser, browser, b->server->record_browsers, b);

    avahi_key_unref(b->key);

    avahi_free(b);
}

void avahi_browser_cleanup(AvahiServer *server) {
    AvahiSRecordBrowser *b;
    AvahiSRecordBrowser *n;

    assert(server);

    while (server->need_browser_cleanup) {
        server->need_browser_cleanup = 0;

        for (b = server->record_browsers; b; b = n) {
            n = b->browser_next;

            if (b->dead)
                avahi_s_record_browser_destroy(b);
        }
    }

    if (server->wide_area_lookup_engine)
        avahi_wide_area_cleanup(server->wide_area_lookup_engine);
    avahi_multicast_lookup_engine_cleanup(server->multicast_lookup_engine);
}

AvahiSRecordBrowser *avahi_s_record_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiKey *key,
    AvahiLookupFlags flags,
    AvahiSRecordBrowserCallback callback,
    void* userdata) {
        AvahiSRecordBrowser *b;

        b = avahi_s_record_browser_prepare(server, interface, protocol, key, flags, callback, userdata);
        avahi_s_record_browser_start_query(b);

        return b;
}
