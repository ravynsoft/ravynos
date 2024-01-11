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
#include <stdlib.h>

#include <avahi-common/domain.h>
#include <avahi-common/timeval.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "browse.h"
#include "log.h"

#define TIMEOUT_MSEC 5000

struct AvahiSServiceResolver {
    AvahiServer *server;
    char *service_name;
    char *service_type;
    char *domain_name;
    AvahiProtocol address_protocol;

    AvahiIfIndex interface;
    AvahiProtocol protocol;

    AvahiSRecordBrowser *record_browser_srv;
    AvahiSRecordBrowser *record_browser_txt;
    AvahiSRecordBrowser *record_browser_a;
    AvahiSRecordBrowser *record_browser_aaaa;

    AvahiRecord *srv_record, *txt_record, *address_record;
    AvahiLookupResultFlags srv_flags, txt_flags, address_flags;

    AvahiSServiceResolverCallback callback;
    void* userdata;
    AvahiLookupFlags user_flags;

    AvahiTimeEvent *time_event;

    AVAHI_LLIST_FIELDS(AvahiSServiceResolver, resolver);
};

static void finish(AvahiSServiceResolver *r, AvahiResolverEvent event) {
    AvahiLookupResultFlags flags;

    assert(r);

    if (r->time_event) {
        avahi_time_event_free(r->time_event);
        r->time_event = NULL;
    }

    flags =
        r->txt_flags |
        r->srv_flags |
        r->address_flags;

    switch (event) {
        case AVAHI_RESOLVER_FAILURE:

            r->callback(
                r,
                r->interface,
                r->protocol,
                event,
                r->service_name,
                r->service_type,
                r->domain_name,
                NULL,
                NULL,
                0,
                NULL,
                flags,
                r->userdata);

            break;

        case AVAHI_RESOLVER_FOUND: {
            AvahiAddress a;

            assert(event == AVAHI_RESOLVER_FOUND);

            assert(r->srv_record);

            if (r->address_record) {
                switch (r->address_record->key->type) {
                    case AVAHI_DNS_TYPE_A:
                        a.proto = AVAHI_PROTO_INET;
                        a.data.ipv4 = r->address_record->data.a.address;
                        break;

                    case AVAHI_DNS_TYPE_AAAA:
                        a.proto = AVAHI_PROTO_INET6;
                        a.data.ipv6 = r->address_record->data.aaaa.address;
                        break;

                    default:
                        assert(0);
                }
            }

            r->callback(
                r,
                r->interface,
                r->protocol,
                event,
                r->service_name,
                r->service_type,
                r->domain_name,
                r->srv_record->data.srv.name,
                r->address_record ? &a : NULL,
                r->srv_record->data.srv.port,
                r->txt_record ? r->txt_record->data.txt.string_list : NULL,
                flags,
                r->userdata);

            break;
        }
    }
}

static void time_event_callback(AvahiTimeEvent *e, void *userdata) {
    AvahiSServiceResolver *r = userdata;

    assert(e);
    assert(r);

    avahi_server_set_errno(r->server, AVAHI_ERR_TIMEOUT);
    finish(r, AVAHI_RESOLVER_FAILURE);
}

static void start_timeout(AvahiSServiceResolver *r) {
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

    AvahiSServiceResolver *r = userdata;

    assert(rr);
    assert(r);

    if (rr == r->record_browser_aaaa || rr == r->record_browser_a)
        r->address_flags = flags;
    else if (rr == r->record_browser_srv)
        r->srv_flags = flags;
    else if (rr == r->record_browser_txt)
        r->txt_flags = flags;

    switch (event) {

        case AVAHI_BROWSER_NEW: {
            int changed = 0;
            assert(record);

            if (r->interface > 0 && interface > 0 &&  interface != r->interface)
                return;

            if (r->protocol != AVAHI_PROTO_UNSPEC && protocol != AVAHI_PROTO_UNSPEC && protocol != r->protocol)
                return;

            if (r->interface <= 0)
                r->interface = interface;

            if (r->protocol == AVAHI_PROTO_UNSPEC)
                r->protocol = protocol;

            switch (record->key->type) {
                case AVAHI_DNS_TYPE_SRV:
                    if (!r->srv_record) {
                        r->srv_record = avahi_record_ref(record);
                        changed = 1;

                        if (r->record_browser_a) {
                            avahi_s_record_browser_free(r->record_browser_a);
                            r->record_browser_a = NULL;
                        }

                        if (r->record_browser_aaaa) {
                            avahi_s_record_browser_free(r->record_browser_aaaa);
                            r->record_browser_aaaa = NULL;
                        }

                        if (!(r->user_flags & AVAHI_LOOKUP_NO_ADDRESS)) {

                            if (r->address_protocol == AVAHI_PROTO_INET || r->address_protocol == AVAHI_PROTO_UNSPEC) {
                                AvahiKey *k = avahi_key_new(r->srv_record->data.srv.name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_A);
                                r->record_browser_a = avahi_s_record_browser_prepare(r->server, r->interface, r->protocol, k, r->user_flags & ~(AVAHI_LOOKUP_NO_TXT|AVAHI_LOOKUP_NO_ADDRESS), record_browser_callback, r);
                                if(r->record_browser_a)
                                    avahi_s_record_browser_start_query(r->record_browser_a);
                                avahi_key_unref(k);
                            }

                            if (r->address_protocol == AVAHI_PROTO_INET6 || r->address_protocol == AVAHI_PROTO_UNSPEC) {
                                AvahiKey *k = avahi_key_new(r->srv_record->data.srv.name, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_AAAA);
                                r->record_browser_aaaa = avahi_s_record_browser_prepare(r->server, r->interface, r->protocol, k, r->user_flags & ~(AVAHI_LOOKUP_NO_TXT|AVAHI_LOOKUP_NO_ADDRESS), record_browser_callback, r);
                                if(r->record_browser_aaaa)
                                    avahi_s_record_browser_start_query(r->record_browser_aaaa);
                                avahi_key_unref(k);
                            }
                        }
                    }
                    break;

                case AVAHI_DNS_TYPE_TXT:

                    assert(!(r->user_flags & AVAHI_LOOKUP_NO_TXT));

                    if (!r->txt_record) {
                        r->txt_record = avahi_record_ref(record);
                        changed = 1;
                    }
                    break;

                case AVAHI_DNS_TYPE_A:
                case AVAHI_DNS_TYPE_AAAA:

                    assert(!(r->user_flags & AVAHI_LOOKUP_NO_ADDRESS));

                    if (!r->address_record) {
                        r->address_record = avahi_record_ref(record);
                        changed = 1;
                    }
                    break;

                default:
                    abort();
            }


            if (changed &&
                r->srv_record &&
                (r->txt_record || (r->user_flags & AVAHI_LOOKUP_NO_TXT)) &&
                (r->address_record || (r->user_flags & AVAHI_LOOKUP_NO_ADDRESS)))
                finish(r, AVAHI_RESOLVER_FOUND);

            break;

        }

        case AVAHI_BROWSER_REMOVE:

            assert(record);

            switch (record->key->type) {
                case AVAHI_DNS_TYPE_SRV:

                    if (r->srv_record && avahi_record_equal_no_ttl(record, r->srv_record)) {
                        avahi_record_unref(r->srv_record);
                        r->srv_record = NULL;

                        if (r->record_browser_a) {
                            avahi_s_record_browser_free(r->record_browser_a);
                            r->record_browser_a = NULL;
                        }

                        if (r->record_browser_aaaa) {
                            avahi_s_record_browser_free(r->record_browser_aaaa);
                            r->record_browser_aaaa = NULL;
                        }

                        /** Look for a replacement */
                        avahi_s_record_browser_restart(r->record_browser_srv);
                        start_timeout(r);
                    }

                    break;

                case AVAHI_DNS_TYPE_TXT:

                    assert(!(r->user_flags & AVAHI_LOOKUP_NO_TXT));

                    if (r->txt_record && avahi_record_equal_no_ttl(record, r->txt_record)) {
                        avahi_record_unref(r->txt_record);
                        r->txt_record = NULL;

                        /** Look for a replacement */
                        avahi_s_record_browser_restart(r->record_browser_txt);
                        start_timeout(r);
                    }
                    break;

                case AVAHI_DNS_TYPE_A:
                case AVAHI_DNS_TYPE_AAAA:

                    assert(!(r->user_flags & AVAHI_LOOKUP_NO_ADDRESS));

                    if (r->address_record && avahi_record_equal_no_ttl(record, r->address_record)) {
                        avahi_record_unref(r->address_record);
                        r->address_record = NULL;

                        /** Look for a replacement */
                        if (r->record_browser_aaaa)
                            avahi_s_record_browser_restart(r->record_browser_aaaa);
                        if (r->record_browser_a)
                            avahi_s_record_browser_restart(r->record_browser_a);
                        start_timeout(r);
                    }
                    break;

                default:
                    abort();
            }

            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;

        case AVAHI_BROWSER_FAILURE:

            if (rr == r->record_browser_a && r->record_browser_aaaa) {
                /* We were looking for both AAAA and A, and the other query is still living, so we'll not die */
                avahi_s_record_browser_free(r->record_browser_a);
                r->record_browser_a = NULL;
                break;
            }

            if (rr == r->record_browser_aaaa && r->record_browser_a) {
                /* We were looking for both AAAA and A, and the other query is still living, so we'll not die */
                avahi_s_record_browser_free(r->record_browser_aaaa);
                r->record_browser_aaaa = NULL;
                break;
            }

            /* Hmm, everything's lost, tell the user */

            if (r->record_browser_srv)
                avahi_s_record_browser_free(r->record_browser_srv);
            if (r->record_browser_txt)
                avahi_s_record_browser_free(r->record_browser_txt);
            if (r->record_browser_a)
                avahi_s_record_browser_free(r->record_browser_a);
            if (r->record_browser_aaaa)
                avahi_s_record_browser_free(r->record_browser_aaaa);

            r->record_browser_srv = r->record_browser_txt = r->record_browser_a = r->record_browser_aaaa = NULL;

            finish(r, AVAHI_RESOLVER_FAILURE);
            break;
    }
}

AvahiSServiceResolver *avahi_s_service_resolver_prepare(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    const char *type,
    const char *domain,
    AvahiProtocol aprotocol,
    AvahiLookupFlags flags,
    AvahiSServiceResolverCallback callback,
    void* userdata) {

    AvahiSServiceResolver *r;
    AvahiKey *k;
    char n[AVAHI_DOMAIN_NAME_MAX];
    int ret;

    assert(server);
    assert(type);
    assert(callback);

    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_IF_VALID(interface), AVAHI_ERR_INVALID_INTERFACE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_PROTO_VALID(protocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_PROTO_VALID(aprotocol), AVAHI_ERR_INVALID_PROTOCOL);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, !domain || avahi_is_valid_domain_name(domain), AVAHI_ERR_INVALID_DOMAIN_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, !name || avahi_is_valid_service_name(name), AVAHI_ERR_INVALID_SERVICE_NAME);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, avahi_is_valid_service_type_strict(type), AVAHI_ERR_INVALID_SERVICE_TYPE);
    AVAHI_CHECK_VALIDITY_RETURN_NULL(server, AVAHI_FLAGS_VALID(flags, AVAHI_LOOKUP_USE_WIDE_AREA|AVAHI_LOOKUP_USE_MULTICAST|AVAHI_LOOKUP_NO_TXT|AVAHI_LOOKUP_NO_ADDRESS), AVAHI_ERR_INVALID_FLAGS);

    if (!domain)
        domain = server->domain_name;

    if ((ret = avahi_service_name_join(n, sizeof(n), name, type, domain)) < 0) {
        avahi_server_set_errno(server, ret);
        return NULL;
    }

    if (!(r = avahi_new(AvahiSServiceResolver, 1))) {
        avahi_server_set_errno(server, AVAHI_ERR_NO_MEMORY);
        return NULL;
    }

    r->server = server;
    r->service_name = avahi_strdup(name);
    r->service_type = avahi_normalize_name_strdup(type);
    r->domain_name = avahi_normalize_name_strdup(domain);
    r->callback = callback;
    r->userdata = userdata;
    r->address_protocol = aprotocol;
    r->srv_record = r->txt_record = r->address_record = NULL;
    r->srv_flags = r->txt_flags = r->address_flags = 0;
    r->interface = interface;
    r->protocol = protocol;
    r->user_flags = flags;
    r->record_browser_a = r->record_browser_aaaa = r->record_browser_srv = r->record_browser_txt = NULL;
    r->time_event = NULL;
    AVAHI_LLIST_PREPEND(AvahiSServiceResolver, resolver, server->service_resolvers, r);

    k = avahi_key_new(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_SRV);
    r->record_browser_srv = avahi_s_record_browser_new(server, interface, protocol, k, flags & ~(AVAHI_LOOKUP_NO_TXT|AVAHI_LOOKUP_NO_ADDRESS), record_browser_callback, r);
    avahi_key_unref(k);

    if (!r->record_browser_srv) {
        avahi_s_service_resolver_free(r);
        return NULL;
    }

    if (!(flags & AVAHI_LOOKUP_NO_TXT)) {
        k = avahi_key_new(n, AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_TXT);
        r->record_browser_txt = avahi_s_record_browser_new(server, interface, protocol, k, flags & ~(AVAHI_LOOKUP_NO_TXT|AVAHI_LOOKUP_NO_ADDRESS),  record_browser_callback, r);
        avahi_key_unref(k);

        if (!r->record_browser_txt) {
            avahi_s_service_resolver_free(r);
            return NULL;
        }
    }

    start_timeout(r);

    return r;
}

void avahi_s_service_resolver_start(AvahiSServiceResolver *r) {
    assert(r);

    if (r->record_browser_srv)
        avahi_s_record_browser_start_query(r->record_browser_srv);
    if (r->record_browser_txt)
        avahi_s_record_browser_start_query(r->record_browser_txt);
    if (r->record_browser_a)
        avahi_s_record_browser_start_query(r->record_browser_a);
    if (r->record_browser_aaaa)
        avahi_s_record_browser_start_query(r->record_browser_aaaa);
}

void avahi_s_service_resolver_free(AvahiSServiceResolver *r) {
    assert(r);

    AVAHI_LLIST_REMOVE(AvahiSServiceResolver, resolver, r->server->service_resolvers, r);

    if (r->time_event)
        avahi_time_event_free(r->time_event);

    if (r->record_browser_srv)
        avahi_s_record_browser_free(r->record_browser_srv);
    if (r->record_browser_txt)
        avahi_s_record_browser_free(r->record_browser_txt);
    if (r->record_browser_a)
        avahi_s_record_browser_free(r->record_browser_a);
    if (r->record_browser_aaaa)
        avahi_s_record_browser_free(r->record_browser_aaaa);

    if (r->srv_record)
        avahi_record_unref(r->srv_record);
    if (r->txt_record)
        avahi_record_unref(r->txt_record);
    if (r->address_record)
        avahi_record_unref(r->address_record);

    avahi_free(r->service_name);
    avahi_free(r->service_type);
    avahi_free(r->domain_name);
    avahi_free(r);
}

AvahiSServiceResolver *avahi_s_service_resolver_new(
    AvahiServer *server,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    const char *type,
    const char *domain,
    AvahiProtocol aprotocol,
    AvahiLookupFlags flags,
    AvahiSServiceResolverCallback callback,
    void* userdata) {
        AvahiSServiceResolver *b;

        b = avahi_s_service_resolver_prepare(server, interface, protocol, name, type, domain, aprotocol, flags, callback, userdata);
        avahi_s_service_resolver_start(b);

        return b;
}