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

#include <avahi-common/domain.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "browse.h"
#include "log.h"

struct AvahiSServiceTypeBrowser {
    AvahiServer *server;
    char *domain_name;

    AvahiSRecordBrowser *record_browser;

    AvahiSServiceTypeBrowserCallback callback;
    void* userdata;

    AVAHI_LLIST_FIELDS(AvahiSServiceTypeBrowser, browser);
};

static void record_browser_callback(
    AvahiSRecordBrowser*rr,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AvahiRecord *record,
    AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiSServiceTypeBrowser *b = userdata;

    assert(rr);
    assert(b);

    /* Filter flags */
    flags &= AVAHI_LOOKUP_RESULT_CACHED | AVAHI_LOOKUP_RESULT_MULTICAST | AVAHI_LOOKUP_RESULT_WIDE_AREA;

    if (record) {
        char type[AVAHI_DOMAIN_NAME_MAX], domain[AVAHI_DOMAIN_NAME_MAX];

        assert(record->key->type == AVAHI_DNS_TYPE_PTR);

        if (avahi_service_name_split(record->data.ptr.name, NULL, 0, type, sizeof(type), domain, sizeof(domain)) < 0) {
            avahi_log_warn("Invalid service type '%s'", record->key->name);
            return;
        }

        b->callback(b, interface, protocol, event, type, domain, flags, b->userdata);
    } else
        b->callback(b, interface, protocol, event, NULL, b->domain_name, flags, b->userdata);
}

AvahiSServiceTypeBrowser *avahi_s_service_type_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiSServiceTypeBrowserCallback callback,
    void* userdata) {

    AvahiSServiceTypeBrowser *b;
    AvahiKey *k = NULL;
    char n[AVAHI_DOMAIN_NAME_MAX];
    int r;

    assert(server);
    assert(callback);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_FLAGS_VALID(flags, AVAHI_LOOKUP_USE_WIDE_AREA|AVAHI_LOOKUP_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);

    if (!domain)
        domain = server->domain_name;

    if ((r = avahi_service_name_join(n, sizeof(n), NULL, "_services._dns-sd._udp", domain)) < 0) {
        avahi_server_set_errno(server, r);
        return NULL;
    }

    if (!(b = avahi_new(AvahiSServiceTypeBrowser, 1))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    b->server = server;
    b->callback = callback;
    b->userdata = userdata;
    b->record_browser = NULL;

    AVAHI_LLIST_PREPEND(AvahiSServiceTypeBrowser, browser, server->service_type_browsers, b);

    if (!(b->domain_name = avahi_normalize_name_strdup(domain))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(k = avahi_key_new(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_PTR))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(b->record_browser = avahi_s_record_browser_prepare(server, interface, protocol, k, flags, record_browser_callback, b)))
        goto fail;

    avahi_key_unref(k);

    return b;

fail:
    if (k)
        avahi_key_unref(k);

    avahi_s_service_type_browser_free(b);

    return NULL;
}

void avahi_s_service_type_browser_free(AvahiSServiceTypeBrowser *b) {
    assert(b);

    AVAHI_LLIST_REMOVE(AvahiSServiceTypeBrowser, browser, b->server->service_type_browsers, b);

    if (b->record_browser)
        avahi_s_record_browser_free(b->record_browser);

    avahi_free(b->domain_name);
    avahi_free(b);
}

void avahi_s_service_type_browser_start(AvahiSServiceTypeBrowser *b) {
    assert(b);

    avahi_s_record_browser_start_query(b->record_browser);
}

AvahiSServiceTypeBrowser *avahi_s_service_type_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiSServiceTypeBrowserCallback callback,
    void* userdata) {
        AvahiSServiceTypeBrowser *b;

        b = avahi_s_service_type_browser_prepare(server, interface, protocol, domain, flags, callback, userdata);
        avahi_s_service_type_browser_start(b);

        return b;
}

