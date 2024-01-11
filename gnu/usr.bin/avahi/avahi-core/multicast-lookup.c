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

#include <avahi-common/malloc.h>
#include <avahi-common/timeval.h>

#include "internal.h"
#include "browse.h"
#include "socket.h"
#include "log.h"
#include "hashmap.h"
#include "multicast-lookup.h"
#include "rr-util.h"

struct AvahiMulticastLookup {
    AvahiMulticastLookupEngine *engine;
    int dead;

    AvahiKey *key, *cname_key;

    AvahiMulticastLookupCallback callback;
    void *userdata;

    AvahiIfIndex interface;
    AvahiProtocol protocol;

    int queriers_added;

    AvahiTimeEvent *all_for_now_event;

    AVAHI_LLIST_FIELDS(AvahiMulticastLookup, lookups);
    AVAHI_LLIST_FIELDS(AvahiMulticastLookup, by_key);
};

struct AvahiMulticastLookupEngine {
    AvahiServer *server;

    /* Lookups */
    AVAHI_LLIST_HEAD(AvahiMulticastLookup, lookups);
    AvahiHashmap *lookups_by_key;

    int cleanup_dead;
};

static void all_for_now_callback(AvahiTimeEvent *e, void* userdata) {
    AvahiMulticastLookup *l = userdata;

    assert(e);
    assert(l);

    avahi_time_event_free(l->all_for_now_event);
    l->all_for_now_event = NULL;

    l->callback(l->engine, l->interface, l->protocol, AVAHI_BROWSER_ALL_FOR_NOW, AVAHI_LOOKUP_RESULT_MULTICAST, NULL, l->userdata);
}

AvahiMulticastLookup *avahi_multicast_lookup_new(
    AvahiMulticastLookupEngine *e,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiKey *key,
    AvahiMulticastLookupCallback callback,
    void *userdata) {

    AvahiMulticastLookup *l, *t;
    struct timeval tv;

    assert(e);
    assert(AVAHI_IF_VALID(interface));
    assert(AVAHI_PROTO_VALID(protocol));
    assert(key);
    assert(callback);

    l = avahi_new(AvahiMulticastLookup, 1);
    l->engine = e;
    l->dead = 0;
    l->key = avahi_key_ref(key);
    l->cname_key = avahi_key_new_cname(l->key);
    l->callback = callback;
    l->userdata = userdata;
    l->interface = interface;
    l->protocol = protocol;
    l->all_for_now_event = NULL;
    l->queriers_added = 0;

    t = avahi_hashmap_lookup(e->lookups_by_key, l->key);
    AVAHI_LLIST_PREPEND(AvahiMulticastLookup, by_key, t, l);
    avahi_hashmap_replace(e->lookups_by_key, avahi_key_ref(l->key), t);

    AVAHI_LLIST_PREPEND(AvahiMulticastLookup, lookups, e->lookups, l);

    avahi_querier_add_for_all(e->server, interface, protocol, l->key, &tv);
    l->queriers_added = 1;

    /* Add a second */
    avahi_timeval_add(&tv, 1000000);

    /* Issue the ALL_FOR_NOW event one second after the querier was initially created */
    l->all_for_now_event = avahi_time_event_new(e->server->time_event_queue, &tv, all_for_now_callback, l);

    return l;
}

static void lookup_stop(AvahiMulticastLookup *l) {
    assert(l);

    l->callback = NULL;

    if (l->queriers_added) {
        avahi_querier_remove_for_all(l->engine->server, l->interface, l->protocol, l->key);
        l->queriers_added = 0;
    }

    if (l->all_for_now_event) {
        avahi_time_event_free(l->all_for_now_event);
        l->all_for_now_event = NULL;
    }
}

static void lookup_destroy(AvahiMulticastLookup *l) {
    AvahiMulticastLookup *t;
    assert(l);

    lookup_stop(l);

    t = avahi_hashmap_lookup(l->engine->lookups_by_key, l->key);
    AVAHI_LLIST_REMOVE(AvahiMulticastLookup, by_key, t, l);
    if (t)
        avahi_hashmap_replace(l->engine->lookups_by_key, avahi_key_ref(l->key), t);
    else
        avahi_hashmap_remove(l->engine->lookups_by_key, l->key);

    AVAHI_LLIST_REMOVE(AvahiMulticastLookup, lookups, l->engine->lookups, l);

    if (l->key)
        avahi_key_unref(l->key);

    if (l->cname_key)
        avahi_key_unref(l->cname_key);

    avahi_free(l);
}

void avahi_multicast_lookup_free(AvahiMulticastLookup *l) {
    assert(l);

    if (l->dead)
        return;

    l->dead = 1;
    l->engine->cleanup_dead = 1;
    lookup_stop(l);
}

void avahi_multicast_lookup_engine_cleanup(AvahiMulticastLookupEngine *e) {
    AvahiMulticastLookup *l, *n;
    assert(e);

    while (e->cleanup_dead) {
        e->cleanup_dead = 0;

        for (l = e->lookups; l; l = n) {
            n = l->lookups_next;

            if (l->dead)
                lookup_destroy(l);
        }
    }
}

struct cbdata {
    AvahiMulticastLookupEngine *engine;
    AvahiMulticastLookupCallback callback;
    void *userdata;
    AvahiKey *key, *cname_key;
    AvahiInterface *interface;
    unsigned n_found;
};

static void* scan_cache_callback(AvahiCache *c, AvahiKey *pattern, AvahiCacheEntry *e, void* userdata) {
    struct cbdata *cbdata = userdata;

    assert(c);
    assert(pattern);
    assert(e);
    assert(cbdata);

    cbdata->callback(
        cbdata->engine,
        cbdata->interface->hardware->index,
        cbdata->interface->protocol,
        AVAHI_BROWSER_NEW,
        AVAHI_LOOKUP_RESULT_CACHED|AVAHI_LOOKUP_RESULT_MULTICAST,
        e->record,
        cbdata->userdata);

    cbdata->n_found ++;

    return NULL;
}

static void scan_interface_callback(AvahiInterfaceMonitor *m, AvahiInterface *i, void* userdata) {
    struct cbdata *cbdata = userdata;

    assert(m);
    assert(i);
    assert(cbdata);

    cbdata->interface = i;

    avahi_cache_walk(i->cache, cbdata->key, scan_cache_callback, cbdata);

    if (cbdata->cname_key)
        avahi_cache_walk(i->cache, cbdata->cname_key, scan_cache_callback, cbdata);

    cbdata->interface = NULL;
}

unsigned avahi_multicast_lookup_engine_scan_cache(
    AvahiMulticastLookupEngine *e,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiKey *key,
    AvahiMulticastLookupCallback callback,
    void *userdata) {

    struct cbdata cbdata;

    assert(e);
    assert(key);
    assert(callback);

    assert(AVAHI_IF_VALID(interface));
    assert(AVAHI_PROTO_VALID(protocol));

    cbdata.engine = e;
    cbdata.key = key;
    cbdata.cname_key = avahi_key_new_cname(key);
    cbdata.callback = callback;
    cbdata.userdata = userdata;
    cbdata.interface = NULL;
    cbdata.n_found = 0;

    avahi_interface_monitor_walk(e->server->monitor, interface, protocol, scan_interface_callback, &cbdata);

    if (cbdata.cname_key)
        avahi_key_unref(cbdata.cname_key);

    return cbdata.n_found;
}

void avahi_multicast_lookup_engine_new_interface(AvahiMulticastLookupEngine *e, AvahiInterface *i) {
    AvahiMulticastLookup *l;

    assert(e);
    assert(i);

    for (l = e->lookups; l; l = l->lookups_next) {

        if (l->dead || !l->callback)
            continue;

        if (l->queriers_added && avahi_interface_match(i, l->interface, l->protocol))
            avahi_querier_add(i, l->key, NULL);
    }
}

void avahi_multicast_lookup_engine_notify(AvahiMulticastLookupEngine *e, AvahiInterface *i, AvahiRecord *record, AvahiBrowserEvent event) {
    AvahiMulticastLookup *l;

    assert(e);
    assert(record);
    assert(i);

    for (l = avahi_hashmap_lookup(e->lookups_by_key, record->key); l; l = l->by_key_next) {
        if (l->dead || !l->callback)
            continue;

        if (avahi_interface_match(i, l->interface, l->protocol))
            l->callback(e, i->hardware->index, i->protocol, event, AVAHI_LOOKUP_RESULT_MULTICAST, record, l->userdata);
    }


    if (record->key->clazz == AVAHI_DNS_CLASS_IN && record->key->type == AVAHI_DNS_TYPE_CNAME) {
        /* It's a CNAME record, so we have to scan the all lookups to see if one matches */

        for (l = e->lookups; l; l = l->lookups_next) {
            AvahiKey *key;

            if (l->dead || !l->callback)
                continue;

            if ((key = avahi_key_new_cname(l->key))) {
                if (avahi_key_equal(record->key, key))
                    l->callback(e, i->hardware->index, i->protocol, event, AVAHI_LOOKUP_RESULT_MULTICAST, record, l->userdata);

                avahi_key_unref(key);
            }
        }
    }
}

AvahiMulticastLookupEngine *avahi_multicast_lookup_engine_new(AvahiServer *s) {
    AvahiMulticastLookupEngine *e;

    assert(s);

    e = avahi_new(AvahiMulticastLookupEngine, 1);
    e->server = s;
    e->cleanup_dead = 0;

    /* Initialize lookup list */
    e->lookups_by_key = avahi_hashmap_new((AvahiHashFunc) avahi_key_hash, (AvahiEqualFunc) avahi_key_equal, (AvahiFreeFunc) avahi_key_unref, NULL);
    AVAHI_LLIST_HEAD_INIT(AvahiWideAreaLookup, e->lookups);

    return e;
}

void avahi_multicast_lookup_engine_free(AvahiMulticastLookupEngine *e) {
    assert(e);

    while (e->lookups)
        lookup_destroy(e->lookups);

    avahi_hashmap_free(e->lookups_by_key);
    avahi_free(e);
}

