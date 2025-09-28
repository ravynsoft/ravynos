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

#include "browse.h"

#define TIMEOUT_MSEC 5000

struct AvahiSAddressResolver {
    AvahiServer *server;
    AvahiAddress address;

    AvahiSRecordBrowser *record_browser;

    AvahiSAddressResolverCallback callback;
    void* userdata;

    AvahiRecord *ptr_record;
    AvahiIfIndex interface;
    AvahiProtocol protocol;
    AvahiLookupResultFlags flags;

    int retry_with_multicast;
    AvahiKey *key;

    AvahiTimeEvent *time_event;

    AVAHI_LLIST_FIELDS(AvahiSAddressResolver, resolver);
};

static void finish(AvahiSAddressResolver *r, AvahiResolverEvent event) {
    assert(r);

    if (r->time_event) {
        avahi_time_event_free(r->time_event);
        r->time_event = NULL;
    }

    switch (event) {
        case AVAHI_RESOLVER_FAILURE:
            r->callback(r, r->interface, r->protocol, event, &r->address, NULL, r->flags, r->userdata);
            break;

        case AVAHI_RESOLVER_FOUND:
            assert(r->ptr_record);
            r->callback(r, r->interface, r->protocol, event, &r->address, r->ptr_record->data.ptr.name, r->flags, r->userdata);
            break;
    }
}

static void time_event_callback(AvahiTimeEvent *e, void *userdata) {
    AvahiSAddressResolver *r = userdata;

    assert(e);
    assert(r);

    avahi_server_set_errno(r->server, AVAHI_ERR_TIMEOUT);
    finish(r, AVAHI_RESOLVER_FAILURE);
}

static void start_timeout(AvahiSAddressResolver *r) {
    struct timeval tv;
    assert(r);

    if (r->time_event)
        return;

    avahi_elapse_time(&tv, TIMEOUT_MSEC, 0);
    r->time_event = avahi_time_event_new(r->server->time_event_queue, &tv, time_event_callback, r);
}

static void record_browser_callback(
    AvahiSRecordBrowser*rr,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AvahiRecord *record,
    AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiSAddressResolver *r = userdata;

    assert(rr);
    assert(r);

    switch (event) {
        case AVAHI_BROWSER_NEW:
            assert(record);
            assert(record->key->type == AVAHI_DNS_TYPE_PTR);

            if (r->interface > 0 && interface != r->interface)
                return;

            if (r->protocol != AVAHI_PROTO_UNSPEC && protocol != r->protocol)
                return;

            if (r->interface <= 0)
                r->interface = interface;

            if (r->protocol == AVAHI_PROTO_UNSPEC)
                r->protocol = protocol;

            if (!r->ptr_record) {
                r->ptr_record = avahi_record_ref(record);
                r->flags = flags;

                finish(r, AVAHI_RESOLVER_FOUND);
            }
            break;

        case AVAHI_BROWSER_REMOVE:
            assert(record);
            assert(record->key->type == AVAHI_DNS_TYPE_PTR);

            if (r->ptr_record && avahi_record_equal_no_ttl(record, r->ptr_record)) {
                avahi_record_unref(r->ptr_record);
                r->ptr_record = NULL;
                r->flags = flags;

                /** Look for a replacement */
                avahi_s_record_browser_restart(r->record_browser);
                start_timeout(r);
            }

            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;

        case AVAHI_BROWSER_FAILURE:

            if (r->retry_with_multicast) {
                r->retry_with_multicast = 0;

                avahi_s_record_browser_free(r->record_browser);
                r->record_browser = avahi_s_record_browser_prepare(r->server, r->interface, r->protocol, r->key, AVAHI_LOOKUP_USE_MULTICAST, record_browser_callback, r);

                if (r->record_browser) {
                    avahi_s_record_browser_start_query(r->record_browser);
                    start_timeout(r);
                    break;
                }
            }

            r->flags = flags;
            finish(r, AVAHI_RESOLVER_FAILURE);
            break;
    }
}

AvahiSAddressResolver *avahi_s_address_resolver_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const AvahiAddress *address,
    AvahiLookupFlags flags,
    AvahiSAddressResolverCallback callback,
    void* userdata) {

    AvahiSAddressResolver *r;
    AvahiKey *k;
    char n[AVAHI_DOMAIN_NAME_MAX];

    assert(server);
    assert(address);
    assert(callback);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, address->proto == AVAHI_PROTO_INET || address->proto == AVAHI_PROTO_INET6, AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_FLAGS_VALID(flags, AVAHI_LOOKUP_USE_WIDE_AREA|AVAHI_LOOKUP_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);

    avahi_reverse_lookup_name(address, n, sizeof(n));

    if (!(k = avahi_key_new(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_PTR))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    if (!(r = avahi_new(AvahiSAddressResolver, 1))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        avahi_key_unref(k);
        return NULL;
    }

    r->server = server;
    r->address = *address;
    r->callback = callback;
    r->userdata = userdata;
    r->ptr_record = NULL;
    r->interface = interface;
    r->protocol = protocol;
    r->flags = 0;
    r->retry_with_multicast = 0;
    r->key = k;

    r->record_browser = NULL;
    AVAHI_LLIST_PREPEND(AvahiSAddressResolver, resolver, server->address_resolvers, r);

    r->time_event = NULL;

    if (!(flags & (AVAHI_LOOKUP_USE_MULTICAST|AVAHI_LOOKUP_USE_WIDE_AREA))) {

        if (!server->wide_area_lookup_engine || !avahi_wide_area_has_servers(server->wide_area_lookup_engine))
            flags |= AVAHI_LOOKUP_USE_MULTICAST;
        else {
            flags |= AVAHI_LOOKUP_USE_WIDE_AREA;
            r->retry_with_multicast = 1;
        }
    }

    r->record_browser = avahi_s_record_browser_prepare(server, interface, protocol, k, flags, record_browser_callback, r);

    if (!r->record_browser) {
        avahi_s_address_resolver_free(r);
        return NULL;
    }

    start_timeout(r);

    return r;
}

void avahi_s_address_resolver_start(AvahiSAddressResolver *r) {
    assert(r);

    if(r->record_browser)
        avahi_s_record_browser_start_query(r->record_browser);
}

void avahi_s_address_resolver_free(AvahiSAddressResolver *r) {
    assert(r);

    AVAHI_LLIST_REMOVE(AvahiSAddressResolver, resolver, r->server->address_resolvers, r);

    if (r->record_browser)
        avahi_s_record_browser_free(r->record_browser);

    if (r->time_event)
        avahi_time_event_free(r->time_event);

    if (r->ptr_record)
        avahi_record_unref(r->ptr_record);

    if (r->key)
        avahi_key_unref(r->key);

    avahi_free(r);
}

AvahiSAddressResolver *avahi_s_address_resolver_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const AvahiAddress *address,
    AvahiLookupFlags flags,
    AvahiSAddressResolverCallback callback,
    void* userdata) {
        AvahiSAddressResolver *b;

        b = avahi_s_address_resolver_prepare(server, interface, protocol, address, flags, callback, userdata);
        avahi_s_address_resolver_start(b);

        return b;
}