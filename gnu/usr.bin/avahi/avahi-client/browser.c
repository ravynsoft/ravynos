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
#include <stdio.h>
#include <string.h>

#include <dbus/dbus.h>

#include <avahi-client/client.h>
#include <avahi-common/dbus.h>
#include <avahi-common/llist.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>

#include "client.h"
#include "internal.h"
#include "xdg-config.h"

static void parse_environment(AvahiDomainBrowser *b) {
    char buf[AVAHI_DOMAIN_NAME_MAX*3], *e, *t, *p;

    assert(b);

    if (!(e = getenv("AVAHI_BROWSE_DOMAINS")))
        return;

    snprintf(buf, sizeof(buf), "%s", e);

    for (t = strtok_r(buf, ":", &p); t; t = strtok_r(NULL, ":", &p)) {
        char domain[AVAHI_DOMAIN_NAME_MAX];
        if (avahi_normalize_name(t, domain, sizeof(domain)))
            b->static_browse_domains = avahi_string_list_add(b->static_browse_domains, domain);
    }
}

static void parse_domain_file(AvahiDomainBrowser *b) {
    FILE *f;
    char buf[AVAHI_DOMAIN_NAME_MAX];

    assert(b);

    if (!(f = avahi_xdg_config_open("avahi/browse-domains")))
        return;


    while (fgets(buf, sizeof(buf)-1, f)) {
        char domain[AVAHI_DOMAIN_NAME_MAX];
        buf[strcspn(buf, "\n\r")] = 0;

        if (avahi_normalize_name(buf, domain, sizeof(domain)))
            b->static_browse_domains = avahi_string_list_add(b->static_browse_domains, domain);
    }
}

static void domain_browser_ref(AvahiDomainBrowser *db) {
    assert(db);
    assert(db->ref >= 1);
    db->ref++;
}

static void defer_timeout_callback(AvahiTimeout *t, void *userdata) {
    AvahiDomainBrowser *db = userdata;
    AvahiStringList *l;
    assert(t);

    db->client->poll_api->timeout_free(db->defer_timeout);
    db->defer_timeout = NULL;

    domain_browser_ref(db);

    for (l = db->static_browse_domains; l; l = l->next) {

        if (db->ref <= 1)
            break;

        db->callback(db, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, AVAHI_BROWSER_NEW, (char*) l->text, AVAHI_LOOKUP_RESULT_STATIC, db->userdata);
    }

    avahi_domain_browser_free(db);
}

AvahiDomainBrowser* avahi_domain_browser_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiDomainBrowserType btype,
    AvahiLookupFlags flags,
    AvahiDomainBrowserCallback callback,
    void *userdata) {

    AvahiDomainBrowser *db = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    char *path;
    int32_t i_interface, i_protocol, bt;
    uint32_t u_flags;

    assert(client);
    assert(callback);

    dbus_error_init (&error);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!domain)
        domain = "";

    if (!(db = avahi_new (AvahiDomainBrowser, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    db->ref = 1;
    db->client = client;
    db->callback = callback;
    db->userdata = userdata;
    db->path = NULL;
    db->interface = interface;
    db->protocol = protocol;
    db->static_browse_domains = NULL;
    db->defer_timeout = NULL;

    AVAHI_LLIST_PREPEND(AvahiDomainBrowser, domain_browsers, client->domain_browsers, db);

    if (!(client->flags & AVAHI_CLIENT_IGNORE_USER_CONFIG)) {
        parse_environment(db);
        parse_domain_file(db);
    }

    db->static_browse_domains = avahi_string_list_reverse(db->static_browse_domains);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "DomainBrowserNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;
    bt = btype;

    if (!(dbus_message_append_args(
              message,
              DBUS_TYPE_INT32, &i_interface,
              DBUS_TYPE_INT32, &i_protocol,
              DBUS_TYPE_STRING, &domain,
              DBUS_TYPE_INT32, &bt,
              DBUS_TYPE_UINT32, &u_flags,
              DBUS_TYPE_INVALID))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error)) ||
        dbus_error_is_set(&error)) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args (reply, &error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID) ||
        dbus_error_is_set(&error) ||
        !path) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!(db->path = avahi_strdup(path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (db->static_browse_domains && btype == AVAHI_DOMAIN_BROWSER_BROWSE) {
        struct timeval tv = { 0, 0 };

        if (!(db->defer_timeout = client->poll_api->timeout_new(client->poll_api, &tv, defer_timeout_callback, db))) {
            avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
            goto fail;
        }
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return db;

fail:

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (db)
        avahi_domain_browser_free(db);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;
}

AvahiClient* avahi_domain_browser_get_client (AvahiDomainBrowser *b) {
    assert(b);
    return b->client;
}

int avahi_domain_browser_free (AvahiDomainBrowser *b) {
    AvahiClient *client;
    int r = AVAHI_OK;

    assert(b);
    assert(b->ref >= 1);

    if (--(b->ref) >= 1)
        return AVAHI_OK;

    client = b->client;

    if (b->path && avahi_client_is_connected(b->client))
        r = avahi_client_simple_method_call(client, b->path, AVAHI_DBUS_INTERFACE_DOMAIN_BROWSER, "Free");

    AVAHI_LLIST_REMOVE(AvahiDomainBrowser, domain_browsers, client->domain_browsers, b);

    if (b->defer_timeout)
        b->client->poll_api->timeout_free(b->defer_timeout);

    avahi_string_list_free(b->static_browse_domains);
    avahi_free(b->path);
    avahi_free(b);

    return r;
}

DBusHandlerResult avahi_domain_browser_event (AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message) {
    AvahiDomainBrowser *db = NULL;
    DBusError error;
    const char *path;
    char *domain = NULL;
    int32_t interface, protocol;
    uint32_t flags = 0;
    AvahiStringList *l;

    assert(client);
    assert(message);

    dbus_error_init (&error);

    if (!(path = dbus_message_get_path(message)))
        goto fail;

    for (db = client->domain_browsers; db; db = db->domain_browsers_next)
        if (strcmp (db->path, path) == 0)
            break;

    if (!db)
        goto fail;

    interface = db->interface;
    protocol = db->protocol;

    switch (event) {
        case AVAHI_BROWSER_NEW:
        case AVAHI_BROWSER_REMOVE:

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_INT32, &interface,
                    DBUS_TYPE_INT32, &protocol,
                    DBUS_TYPE_STRING, &domain,
                    DBUS_TYPE_UINT32, &flags,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }

            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;

        case AVAHI_BROWSER_FAILURE: {
            char *etxt;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_STRING, &etxt,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }

            avahi_client_set_errno(db->client, avahi_error_dbus_to_number(etxt));
            break;
        }
    }

    if (domain)
        for (l = db->static_browse_domains; l; l = l->next)
            if (avahi_domain_equal((char*) l->text, domain)) {
                /* We had this entry already in the static entries */
                return DBUS_HANDLER_RESULT_HANDLED;
            }

    db->callback(db, (AvahiIfIndex) interface, (AvahiProtocol) protocol, event, domain, (AvahiLookupResultFlags) flags, db->userdata);

    return DBUS_HANDLER_RESULT_HANDLED;

fail:
    dbus_error_free (&error);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/* AvahiServiceTypeBrowser */

AvahiServiceTypeBrowser* avahi_service_type_browser_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiServiceTypeBrowserCallback callback,
    void *userdata) {

    AvahiServiceTypeBrowser *b = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    char *path;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;

    assert(client);
    assert(callback);

    dbus_error_init(&error);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!domain)
        domain = "";

    if (!(b = avahi_new(AvahiServiceTypeBrowser, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    b->client = client;
    b->callback = callback;
    b->userdata = userdata;
    b->path = NULL;
    b->domain = NULL;
    b->interface = interface;
    b->protocol = protocol;

    AVAHI_LLIST_PREPEND(AvahiServiceTypeBrowser, service_type_browsers, client->service_type_browsers, b);

    if (domain[0])
        if (!(b->domain = avahi_strdup(domain))) {
            avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
            goto fail;
        }

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "ServiceTypeBrowserNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_INVALID)) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error)) ||
        dbus_error_is_set(&error)) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args (reply, &error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID) ||
        dbus_error_is_set(&error) ||
        !path) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!(b->path = avahi_strdup(path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return b;

fail:

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (b)
        avahi_service_type_browser_free(b);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;
}

AvahiClient* avahi_service_type_browser_get_client (AvahiServiceTypeBrowser *b) {
    assert(b);
    return b->client;
}

int avahi_service_type_browser_free (AvahiServiceTypeBrowser *b) {
    AvahiClient *client;
    int r = AVAHI_OK;

    assert(b);
    client = b->client;

    if (b->path && avahi_client_is_connected(b->client))
        r = avahi_client_simple_method_call(client, b->path, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "Free");

    AVAHI_LLIST_REMOVE(AvahiServiceTypeBrowser, service_type_browsers, b->client->service_type_browsers, b);

    avahi_free(b->path);
    avahi_free(b->domain);
    avahi_free(b);
    return r;
}

DBusHandlerResult avahi_service_type_browser_event (AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message) {
    AvahiServiceTypeBrowser *b = NULL;
    DBusError error;
    const char *path;
    char *domain, *type = NULL;
    int32_t interface, protocol;
    uint32_t flags = 0;

    assert(client);
    assert(message);

    dbus_error_init (&error);

    if (!(path = dbus_message_get_path(message)))
        goto fail;

    for (b = client->service_type_browsers; b; b = b->service_type_browsers_next)
        if (strcmp (b->path, path) == 0)
            break;

    if (!b)
        goto fail;

    domain = b->domain;
    interface = b->interface;
    protocol = b->protocol;

    switch (event) {
        case AVAHI_BROWSER_NEW:
        case AVAHI_BROWSER_REMOVE:
            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_INT32, &interface,
                    DBUS_TYPE_INT32, &protocol,
                    DBUS_TYPE_STRING, &type,
                    DBUS_TYPE_STRING, &domain,
                    DBUS_TYPE_UINT32, &flags,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set(&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;

        case AVAHI_BROWSER_FAILURE: {
            char *etxt;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_STRING, &etxt,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }

            avahi_client_set_errno(b->client, avahi_error_dbus_to_number(etxt));
            break;
        }
    }

    b->callback(b, (AvahiIfIndex) interface, (AvahiProtocol) protocol, event, type, domain, (AvahiLookupResultFlags) flags, b->userdata);

    return DBUS_HANDLER_RESULT_HANDLED;

fail:
    dbus_error_free (&error);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/* AvahiServiceBrowser */

AvahiServiceBrowser* avahi_service_browser_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *type,
    const char *domain,
    AvahiLookupFlags flags,
    AvahiServiceBrowserCallback callback,
    void *userdata) {

    AvahiServiceBrowser *b = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    char *path;
    int32_t i_protocol, i_interface;
    uint32_t u_flags;

    assert(client);
    assert(type);
    assert(callback);

    dbus_error_init(&error);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!domain)
        domain = "";

    if (!(b = avahi_new(AvahiServiceBrowser, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    b->client = client;
    b->callback = callback;
    b->userdata = userdata;
    b->path = NULL;
    b->type = b->domain = NULL;
    b->interface = interface;
    b->protocol = protocol;

    AVAHI_LLIST_PREPEND(AvahiServiceBrowser, service_browsers, client->service_browsers, b);

    if (!(b->type = avahi_strdup(type))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (domain && domain[0])
        if (!(b->domain = avahi_strdup(domain))) {
            avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
            goto fail;
        }

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "ServiceBrowserNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_INVALID)) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error)) ||
        dbus_error_is_set(&error)) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args (reply, &error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID) ||
        dbus_error_is_set(&error) ||
        !path) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!(b->path = avahi_strdup(path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return b;

fail:
    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (b)
        avahi_service_browser_free(b);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;
}

AvahiClient* avahi_service_browser_get_client (AvahiServiceBrowser *b) {
    assert(b);
    return b->client;
}

int avahi_service_browser_free (AvahiServiceBrowser *b) {
    AvahiClient *client;
    int r = AVAHI_OK;

    assert(b);
    client = b->client;

    if (b->path && avahi_client_is_connected(b->client))
        r = avahi_client_simple_method_call(client, b->path, AVAHI_DBUS_INTERFACE_SERVICE_BROWSER, "Free");

    AVAHI_LLIST_REMOVE(AvahiServiceBrowser, service_browsers, b->client->service_browsers, b);

    avahi_free(b->path);
    avahi_free(b->type);
    avahi_free(b->domain);
    avahi_free(b);
    return r;
}

DBusHandlerResult avahi_service_browser_event(AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message) {
    AvahiServiceBrowser *b = NULL;
    DBusError error;
    const char *path;
    char *name = NULL, *type, *domain;
    int32_t interface, protocol;
    uint32_t flags = 0;

    dbus_error_init (&error);

    if (!(path = dbus_message_get_path(message)))
        goto fail;

    for (b = client->service_browsers; b; b = b->service_browsers_next)
        if (strcmp (b->path, path) == 0)
            break;

    if (!b)
        goto fail;

    type = b->type;
    domain = b->domain;
    interface = b->interface;
    protocol = b->protocol;

    switch (event) {
        case AVAHI_BROWSER_NEW:
        case AVAHI_BROWSER_REMOVE:

            if (!dbus_message_get_args (
                    message, &error,
                    DBUS_TYPE_INT32, &interface,
                    DBUS_TYPE_INT32, &protocol,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_STRING, &type,
                    DBUS_TYPE_STRING, &domain,
                    DBUS_TYPE_UINT32, &flags,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set(&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;

        case AVAHI_BROWSER_FAILURE: {
            char *etxt;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_STRING, &etxt,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }

            avahi_client_set_errno(b->client, avahi_error_dbus_to_number(etxt));
            break;
        }
    }

    b->callback(b, (AvahiIfIndex) interface, (AvahiProtocol) protocol, event, name, type, domain, (AvahiLookupResultFlags) flags, b->userdata);

    return DBUS_HANDLER_RESULT_HANDLED;

fail:
    dbus_error_free (&error);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/* AvahiRecordBrowser */

AvahiRecordBrowser* avahi_record_browser_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    uint16_t clazz,
    uint16_t type,
    AvahiLookupFlags flags,
    AvahiRecordBrowserCallback callback,
    void *userdata) {

    AvahiRecordBrowser *b = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    char *path;
    int32_t i_protocol, i_interface;
    uint32_t u_flags;

    assert(client);
    assert(name);
    assert(callback);

    dbus_error_init(&error);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!(b = avahi_new(AvahiRecordBrowser, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    b->client = client;
    b->callback = callback;
    b->userdata = userdata;
    b->path = NULL;
    b->name = NULL;
    b->clazz = clazz;
    b->type = type;
    b->interface = interface;
    b->protocol = protocol;

    AVAHI_LLIST_PREPEND(AvahiRecordBrowser, record_browsers, client->record_browsers, b);

    if (!(b->name = avahi_strdup(name))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "RecordBrowserNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_UINT16, &clazz,
            DBUS_TYPE_UINT16, &type,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_INVALID)) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error)) ||
        dbus_error_is_set(&error)) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args (reply, &error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID) ||
        dbus_error_is_set(&error) ||
        !path) {
        avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!(b->path = avahi_strdup(path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return b;

fail:
    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (b)
        avahi_record_browser_free(b);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;
}

AvahiClient* avahi_record_browser_get_client (AvahiRecordBrowser *b) {
    assert(b);
    return b->client;
}

int avahi_record_browser_free (AvahiRecordBrowser *b) {
    AvahiClient *client;
    int r = AVAHI_OK;

    assert(b);
    client = b->client;

    if (b->path && avahi_client_is_connected(b->client))
        r = avahi_client_simple_method_call(client, b->path, AVAHI_DBUS_INTERFACE_RECORD_BROWSER, "Free");

    AVAHI_LLIST_REMOVE(AvahiRecordBrowser, record_browsers, b->client->record_browsers, b);

    avahi_free(b->path);
    avahi_free(b->name);
    avahi_free(b);
    return r;
}

DBusHandlerResult avahi_record_browser_event(AvahiClient *client, AvahiBrowserEvent event, DBusMessage *message) {
    AvahiRecordBrowser *b = NULL;
    DBusError error;
    const char *path;
    char *name;
    int32_t interface, protocol;
    uint32_t flags = 0;
    uint16_t clazz, type;
    void *rdata = NULL;
    int rdata_size = 0;

    dbus_error_init (&error);

    if (!(path = dbus_message_get_path(message)))
        goto fail;

    for (b = client->record_browsers; b; b = b->record_browsers_next)
        if (strcmp (b->path, path) == 0)
            break;

    if (!b)
        goto fail;

    interface = b->interface;
    protocol = b->protocol;
    clazz = b->clazz;
    type = b->type;
    name = b->name;

    switch (event) {
        case AVAHI_BROWSER_NEW:
        case AVAHI_BROWSER_REMOVE: {
            DBusMessageIter iter, sub;
            int j;

            if (!dbus_message_get_args (
                    message, &error,
                    DBUS_TYPE_INT32, &interface,
                    DBUS_TYPE_INT32, &protocol,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_UINT16, &clazz,
                    DBUS_TYPE_UINT16, &type,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set(&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }


            dbus_message_iter_init(message, &iter);

            for (j = 0; j < 5; j++)
                dbus_message_iter_next(&iter);

            if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
                dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_BYTE)
                goto fail;

            dbus_message_iter_recurse(&iter, &sub);
            dbus_message_iter_get_fixed_array(&sub, &rdata, &rdata_size);

            dbus_message_iter_next(&iter);

            if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT32)
                goto fail;

            dbus_message_iter_get_basic(&iter, &flags);

            break;
        }

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;

        case AVAHI_BROWSER_FAILURE: {
            char *etxt;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_STRING, &etxt,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse browser event.\n");
                goto fail;
            }

            avahi_client_set_errno(b->client, avahi_error_dbus_to_number(etxt));
            break;
        }
    }

    b->callback(b, (AvahiIfIndex) interface, (AvahiProtocol) protocol, event, name, clazz, type, rdata, (size_t) rdata_size, (AvahiLookupResultFlags) flags, b->userdata);

    return DBUS_HANDLER_RESULT_HANDLED;

fail:
    dbus_error_free (&error);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
