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

#include <avahi-common/domain.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "browse.h"
#include "log.h"

struct AvahiSDomainBrowser {
    int ref;

    AvahiServer *server;

    AvahiSRecordBrowser *record_browser;

    AvahiDomainBrowserType type;
    AvahiSDomainBrowserCallback callback;
    void* userdata;

    AvahiTimeEvent *defer_event;

    int all_for_now_scheduled;

    AVAHI_LLIST_FIELDS(AvahiSDomainBrowser, browser);
};

static void inc_ref(AvahiSDomainBrowser *b) {
    assert(b);
    assert(b->ref >= 1);

    b->ref++;
}

static void record_browser_callback(
    AvahiSRecordBrowser*rr,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AvahiRecord *record,
    AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiSDomainBrowser *b = userdata;
    char *n = NULL;

    assert(rr);
    assert(b);

    if (event == AVAHI_BROWSER_ALL_FOR_NOW &&
        b->defer_event) {

        b->all_for_now_scheduled = 1;
        return;
    }

    /* Filter flags */
    flags &= AVAHI_LOOKUP_RESULT_CACHED | AVAHI_LOOKUP_RESULT_MULTICAST | AVAHI_LOOKUP_RESULT_WIDE_AREA;

    if (record) {
        assert(record->key->type == AVAHI_DNS_TYPE_PTR);
        n = record->data.ptr.name;

        if (b->type == AVAHI_DOMAIN_BROWSER_BROWSE) {
            AvahiStringList *l;

            /* Filter out entries defined statically */

            for (l = b->server->config.browse_domains; l; l = l->next)
                if (avahi_domain_equal((char*) l->text, n))
                    return;
        }

    }

    b->callback(b, interface, protocol, event, n, flags, b->userdata);
}

static void defer_callback(AvahiTimeEvent *e, void *userdata) {
    AvahiSDomainBrowser *b = userdata;
    AvahiStringList *l;

    assert(e);
    assert(b);

    assert(b->type == AVAHI_DOMAIN_BROWSER_BROWSE);

    avahi_time_event_free(b->defer_event);
    b->defer_event = NULL;

    /* Increase ref counter */
    inc_ref(b);

    for (l = b->server->config.browse_domains; l; l = l->next) {

        /* Check whether this object still exists outside our own
         * stack frame */
        if (b->ref <= 1)
            break;

        b->callback(b, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_BROWSER_NEW, (char*) l->text, AVAHI_LOOKUP_RESULT_STATIC, b->userdata);
    }

    if (b->ref > 1) {
        /* If the ALL_FOR_NOW event has already been scheduled, execute it now */

        if (b->all_for_now_scheduled)
            b->callback(b, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_BROWSER_ALL_FOR_NOW, NULL, 0, b->userdata);
    }

    /* Decrease ref counter */
    avahi_s_domain_browser_free(b);
}

AvahiSDomainBrowser *avahi_s_domain_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDomainBrowserType type,
    AvahiLookupFlags flags,
    AvahiSDomainBrowserCallback callback,
    void* userdata) {

    static const char * const type_table[AVAHI_DOMAIN_BROWSER_MAX] = {
        "b",
        "db",
        "r",
        "dr",
        "lb"
    };

    AvahiSDomainBrowser *b;
    AvahiKey *k = NULL;
    char n[AVAHI_DOMAIN_NAME_MAX];
    int r;

    assert(server);
    assert(callback);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, type < AVAHI_DOMAIN_BROWSER_MAX, AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_FLAGS_VALID(flags, AVAHI_LOOKUP_USE_WIDE_AREA|AVAHI_LOOKUP_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);

    if (!domain)
        domain = server->domain_name;

    if ((r = avahi_service_name_join(n, sizeof(n), type_table[type], "_dns-sd._udp", domain)) < 0) {
        avahi_server_set_errno(server, r);
        return NULL;
    }

    if (!(b = avahi_new(AvahiSDomainBrowser, 1))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    b->ref = 1;
    b->server = server;
    b->callback = callback;
    b->userdata = userdata;
    b->record_browser = NULL;
    b->type = type;
    b->all_for_now_scheduled = 0;
    b->defer_event = NULL;

    AVAHI_LLIST_PREPEND(AvahiSDomainBrowser, browser, server->domain_browsers, b);

    if (!(k = avahi_key_new(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_PTR))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(b->record_browser = avahi_s_record_browser_prepare(server, interface, protocol, k, flags, record_browser_callback, b)))
        goto fail;

    avahi_key_unref(k);

    if (type == AVAHI_DOMAIN_BROWSER_BROWSE && b->server->config.browse_domains)
        b->defer_event = avahi_time_event_new(server->time_event_queue, NULL, defer_callback, b);

    return b;

fail:

    if (k)
        avahi_key_unref(k);

    avahi_s_domain_browser_free(b);

    return NULL;
}

void avahi_s_domain_browser_start(AvahiSDomainBrowser *b) {
    assert(b);

    if(b->record_browser)
        avahi_s_record_browser_start_query(b->record_browser);
}

void avahi_s_domain_browser_free(AvahiSDomainBrowser *b) {
    assert(b);

    assert(b->ref >= 1);
    if (--b->ref > 0)
        return;

    AVAHI_LLIST_REMOVE(AvahiSDomainBrowser, browser, b->server->domain_browsers, b);

    if (b->record_browser)
        avahi_s_record_browser_free(b->record_browser);

    if (b->defer_event)
        avahi_time_event_free(b->defer_event);

    avahi_free(b);
}

AvahiSDomainBrowser *avahi_s_domain_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDomainBrowserType type,
    AvahiLookupFlags flags,
    AvahiSDomainBrowserCallback callback,
    void* userdata) {
        AvahiSDomainBrowser *b;

        b = avahi_s_domain_browser_prepare(server, interface, protocol, domain, type, flags, callback, userdata);
        avahi_s_domain_browser_start(b);

        return b;
}