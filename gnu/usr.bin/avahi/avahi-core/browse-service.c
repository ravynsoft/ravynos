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

struct AvahiSServiceBrowser {
    AvahiServer *server;
    char *domain_name;
    char *service_type;

    AvahiSRecordBrowser *record_browser;

    AvahiSServiceBrowserCallback callback;
    void* userdata;

    AVAHI_LLIST_FIELDS(AvahiSServiceBrowser, browser);
};

static void record_browser_callback(
    AvahiSRecordBrowser*rr,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AvahiRecord *record,
    AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiSServiceBrowser *b = userdata;

    assert(rr);
    assert(b);

    /* Filter flags */
    flags &= AVAHI_LOOKUP_RESULT_CACHED | AVAHI_LOOKUP_RESULT_MULTICAST | AVAHI_LOOKUP_RESULT_WIDE_AREA;

    if (record) {
        char service[AVAHI_LABEL_MAX], type[AVAHI_DOMAIN_NAME_MAX], domain[AVAHI_DOMAIN_NAME_MAX];

        assert(record->key->type == AVAHI_DNS_TYPE_PTR);

        if (event == AVAHI_BROWSER_NEW && avahi_server_is_service_local(b->server, interface, protocol, record->data.ptr.name))
            flags |= AVAHI_LOOKUP_RESULT_LOCAL;

        if (avahi_service_name_split(record->data.ptr.name, service, sizeof(service), type, sizeof(type), domain, sizeof(domain)) < 0) {
            avahi_log_warn("Failed to split '%s'", record->key->name);
            return;
        }

        b->callback(b, interface, protocol, event, service, type, domain, flags, b->userdata);

    } else
        b->callback(b, interface, protocol, event, NULL, b->service_type, b->domain_name, flags, b->userdata);

}

AvahiSServiceBrowser *avahi_s_service_browser_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *service_type,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiSServiceBrowserCallback callback,
    void* userdata) {

    AvahiSServiceBrowser *b;
    AvahiKey *k = NULL;
    char n[AVAHI_DOMAIN_NAME_MAX];
    int r;

    assert(server);
    assert(callback);
    assert(service_type);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_FLAGS_VALID(flags, AVAHI_LOOKUP_USE_WIDE_AREA|AVAHI_LOOKUP_USE_MULTICAST), AVAHI_ERR_INVALID_FLAGS);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, avahi_is_valid_service_type_generic(service_type), AVAHI_ERR_INVALID_SERVICE_TYPE);

    if (!domain)
        domain = server->domain_name;

    if ((r = avahi_service_name_join(n, sizeof(n), NULL, service_type, domain)) < 0) {
        avahi_server_set_errno(server, r);
        return NULL;
    }

    if (!(b = avahi_new(AvahiSServiceBrowser, 1))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    b->server = server;
    b->domain_name = b->service_type = NULL;
    b->callback = callback;
    b->userdata = userdata;
    b->record_browser = NULL;

    AVAHI_LLIST_PREPEND(AvahiSServiceBrowser, browser, server->service_browsers, b);

    if (!(b->domain_name = avahi_normalize_name_strdup(domain)) ||
        !(b->service_type = avahi_normalize_name_strdup(service_type))) {
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

    avahi_s_service_browser_free(b);
    return NULL;
}

void avahi_s_service_browser_free(AvahiSServiceBrowser *b) {
    assert(b);

    AVAHI_LLIST_REMOVE(AvahiSServiceBrowser, browser, b->server->service_browsers, b);

    if (b->record_browser)
        avahi_s_record_browser_free(b->record_browser);

    avahi_free(b->domain_name);
    avahi_free(b->service_type);
    avahi_free(b);
}

void avahi_s_service_browser_start(AvahiSServiceBrowser *b) {
    assert(b);

    avahi_s_record_browser_start_query(b->record_browser);
}

AvahiSServiceBrowser *avahi_s_service_browser_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *service_type,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiSServiceBrowserCallback callback,
    void* userdata) {
        AvahiSServiceBrowser *b;

        b = avahi_s_service_browser_prepare(server, interface, protocol, service_type, domain, flags, callback, userdata);
        avahi_s_service_browser_start(b);

        return b;
}
