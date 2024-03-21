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

#include <avahi-common/dbus.h>
#include <avahi-common/llist.h>
#include <avahi-common/error.h>
#include <avahi-common/dbus.h>
#include <avahi-common/malloc.h>
#include <avahi-common/dbus-watch-glue.h>
#include <avahi-common/i18n.h>

#include "client.h"
#include "internal.h"

#define AVAHI_CLIENT_DBUS_API_SUPPORTED ((uint32_t) 0x0201)

static int init_server(AvahiClient *client, int *ret_error);

int avahi_client_set_errno (AvahiClient *client, int error) {
    assert(client);

    return client->error = error;
}

int avahi_client_set_dbus_error(AvahiClient *client, DBusError *error) {
    assert(client);
    assert(error);

    return avahi_client_set_errno(client, avahi_error_dbus_to_number(error->name));
}

static void client_set_state(AvahiClient *client, AvahiClientState state) {
    assert(client);

    if (client->state == state)
        return;

    client->state = state;

    switch (client->state) {
        case AVAHI_CLIENT_FAILURE:
            if (client->bus) {
#ifdef HAVE_DBUS_CONNECTION_CLOSE
                dbus_connection_close(client->bus);
#else
                dbus_connection_disconnect(client->bus);
#endif
                dbus_connection_unref(client->bus);
                client->bus = NULL;
            }

            /* Fall through */

        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_S_REGISTERING:

            /* Clear cached strings */
            avahi_free(client->host_name);
            avahi_free(client->host_name_fqdn);
            avahi_free(client->domain_name);

            client->host_name =  NULL;
            client->host_name_fqdn = NULL;
            client->domain_name = NULL;
            break;

        case AVAHI_CLIENT_S_RUNNING:
        case AVAHI_CLIENT_CONNECTING:
            break;

    }

    if (client->callback)
        client->callback (client, state, client->userdata);
}

static DBusHandlerResult filter_func(DBusConnection *bus, DBusMessage *message, void *userdata) {
    AvahiClient *client = userdata;
    DBusError error;

    assert(bus);
    assert(message);

    dbus_error_init(&error);

/*     fprintf(stderr, "dbus: interface=%s, path=%s, member=%s\n", */
/*             dbus_message_get_interface (message), */
/*             dbus_message_get_path (message), */
/*             dbus_message_get_member (message)); */

    if (dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL, "Disconnected")) {

        /* The DBUS server died or kicked us */
        avahi_client_set_errno(client, AVAHI_ERR_DISCONNECTED);
        goto fail;

    } else if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "NameAcquired")) {

        /* Ignore this message */

    } else if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "NameOwnerChanged")) {
        char *name, *old, *new;

        if (!dbus_message_get_args(
                  message, &error,
                  DBUS_TYPE_STRING, &name,
                  DBUS_TYPE_STRING, &old,
                  DBUS_TYPE_STRING, &new,
                  DBUS_TYPE_INVALID) || dbus_error_is_set(&error)) {

            fprintf(stderr, "WARNING: Failed to parse NameOwnerChanged signal: %s\n", error.message);
            avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
            goto fail;
        }

        if (strcmp(name, AVAHI_DBUS_NAME) == 0) {

            if (old[0] &&
                avahi_client_is_connected(client)) {

                /* Regardless if the server lost its name or
                 * if the name was transfered: our services are no longer
                 * available, so we disconnect ourselves */
                avahi_client_set_errno(client, AVAHI_ERR_DISCONNECTED);
                goto fail;

            } else if (client->state == AVAHI_CLIENT_CONNECTING && (!old || *old == 0)) {
                int ret;

                /* Server appeared */

                if ((ret = init_server(client, NULL)) < 0) {
                    avahi_client_set_errno(client, ret);
                    goto fail;
                }
            }
        }

    } else if (!avahi_client_is_connected(client)) {

        /* Ignore messages we get in unconnected state */

    } else if (dbus_message_is_signal (message, AVAHI_DBUS_INTERFACE_SERVER, "StateChanged")) {
        int32_t state;
        char *e = NULL;
        int c;

        if (!dbus_message_get_args(
                  message, &error,
                  DBUS_TYPE_INT32, &state,
                  DBUS_TYPE_STRING, &e,
                  DBUS_TYPE_INVALID) || dbus_error_is_set (&error)) {

            fprintf(stderr, "WARNING: Failed to parse Server.StateChanged signal: %s\n", error.message);
            avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
            goto fail;
        }

        if ((c = avahi_error_dbus_to_number(e)) != AVAHI_OK)
            avahi_client_set_errno(client, c);

        client_set_state(client, (AvahiClientState) state);

    } else if (dbus_message_is_signal (message, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "StateChanged")) {
        const char *path;
        AvahiEntryGroup *g;
        path = dbus_message_get_path(message);

        for (g = client->groups; g; g = g->groups_next)
            if (strcmp(g->path, path) == 0)
                break;

        if (g) {
            int32_t state;
            char *e;
            int c;

            if (!dbus_message_get_args(
                      message, &error,
                      DBUS_TYPE_INT32, &state,
                      DBUS_TYPE_STRING, &e,
                      DBUS_TYPE_INVALID) ||
                dbus_error_is_set(&error)) {

                fprintf(stderr, "WARNING: Failed to parse EntryGroup.StateChanged signal: %s\n", error.message);
                avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
                goto fail;
            }

            if ((c = avahi_error_dbus_to_number(e)) != AVAHI_OK)
                avahi_client_set_errno(client, c);

            avahi_entry_group_set_state(g, state);
        }

    } else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_DOMAIN_BROWSER, "ItemNew"))
        return avahi_domain_browser_event(client, AVAHI_BROWSER_NEW, message);
    else if (dbus_message_is_signal (message, AVAHI_DBUS_INTERFACE_DOMAIN_BROWSER, "ItemRemove"))
        return avahi_domain_browser_event(client, AVAHI_BROWSER_REMOVE, message);
    else if (dbus_message_is_signal (message, AVAHI_DBUS_INTERFACE_DOMAIN_BROWSER, "CacheExhausted"))
        return avahi_domain_browser_event(client, AVAHI_BROWSER_CACHE_EXHAUSTED, message);
    else if (dbus_message_is_signal (message, AVAHI_DBUS_INTERFACE_DOMAIN_BROWSER, "AllForNow"))
        return avahi_domain_browser_event(client, AVAHI_BROWSER_ALL_FOR_NOW, message);
    else if (dbus_message_is_signal (message, AVAHI_DBUS_INTERFACE_DOMAIN_BROWSER, "Failure"))
        return avahi_domain_browser_event(client, AVAHI_BROWSER_FAILURE, message);

    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "ItemNew"))
        return avahi_service_type_browser_event (client, AVAHI_BROWSER_NEW, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "ItemRemove"))
        return avahi_service_type_browser_event (client, AVAHI_BROWSER_REMOVE, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "CacheExhausted"))
        return avahi_service_type_browser_event (client, AVAHI_BROWSER_CACHE_EXHAUSTED, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "AllForNow"))
        return avahi_service_type_browser_event (client, AVAHI_BROWSER_ALL_FOR_NOW, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "Failure"))
        return avahi_service_type_browser_event (client, AVAHI_BROWSER_FAILURE, message);

    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_BROWSER, "ItemNew"))
        return avahi_service_browser_event (client, AVAHI_BROWSER_NEW, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_BROWSER, "ItemRemove"))
        return avahi_service_browser_event (client, AVAHI_BROWSER_REMOVE, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_BROWSER, "CacheExhausted"))
        return avahi_service_browser_event (client, AVAHI_BROWSER_CACHE_EXHAUSTED, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_BROWSER, "AllForNow"))
        return avahi_service_browser_event (client, AVAHI_BROWSER_ALL_FOR_NOW, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_BROWSER, "Failure"))
        return avahi_service_browser_event (client, AVAHI_BROWSER_FAILURE, message);

    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_RESOLVER, "Found"))
        return avahi_service_resolver_event (client, AVAHI_RESOLVER_FOUND, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_SERVICE_RESOLVER, "Failure"))
        return avahi_service_resolver_event (client, AVAHI_RESOLVER_FAILURE, message);

    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_HOST_NAME_RESOLVER, "Found"))
        return avahi_host_name_resolver_event (client, AVAHI_RESOLVER_FOUND, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_HOST_NAME_RESOLVER, "Failure"))
        return avahi_host_name_resolver_event (client, AVAHI_RESOLVER_FAILURE, message);

    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_ADDRESS_RESOLVER, "Found"))
        return avahi_address_resolver_event (client, AVAHI_RESOLVER_FOUND, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_ADDRESS_RESOLVER, "Failure"))
        return avahi_address_resolver_event (client, AVAHI_RESOLVER_FAILURE, message);

    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_RECORD_BROWSER, "ItemNew"))
        return avahi_record_browser_event (client, AVAHI_BROWSER_NEW, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_RECORD_BROWSER, "ItemRemove"))
        return avahi_record_browser_event (client, AVAHI_BROWSER_REMOVE, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_RECORD_BROWSER, "CacheExhausted"))
        return avahi_record_browser_event (client, AVAHI_BROWSER_CACHE_EXHAUSTED, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_RECORD_BROWSER, "AllForNow"))
        return avahi_record_browser_event (client, AVAHI_BROWSER_ALL_FOR_NOW, message);
    else if (dbus_message_is_signal(message, AVAHI_DBUS_INTERFACE_RECORD_BROWSER, "Failure"))
        return avahi_record_browser_event (client, AVAHI_BROWSER_FAILURE, message);

    else {

        fprintf(stderr, "WARNING: Unhandled message: interface=%s, path=%s, member=%s\n",
               dbus_message_get_interface(message),
               dbus_message_get_path(message),
               dbus_message_get_member(message));

        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    return DBUS_HANDLER_RESULT_HANDLED;

fail:

    if (dbus_error_is_set(&error)) {
        avahi_client_set_errno(client, avahi_error_dbus_to_number(error.name));
        dbus_error_free(&error);
    }

    client_set_state(client, AVAHI_CLIENT_FAILURE);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static int get_server_state(AvahiClient *client, int *ret_error) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    int32_t state;
    int e = AVAHI_ERR_NO_MEMORY;

    assert(client);

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "GetState")))
        goto fail;

    reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error);

    if (!reply || dbus_error_is_set (&error))
        goto fail;

    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_INT32, &state, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error))
        goto fail;

    client_set_state(client, (AvahiClientState) state);

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return AVAHI_OK;

fail:
    if (dbus_error_is_set(&error)) {
        e = avahi_error_dbus_to_number (error.name);
        dbus_error_free(&error);
    }

    if (ret_error)
        *ret_error = e;

    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);

    return e;
}

static int check_version(AvahiClient *client, int *ret_error) {
    DBusMessage *message = NULL, *reply  = NULL;
    DBusError error;
    uint32_t version;
    int e = AVAHI_ERR_NO_MEMORY;

    assert(client);

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "GetAPIVersion")))
        goto fail;

    reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error);

    if (!reply || dbus_error_is_set (&error)) {
        char *version_str;

        if (!dbus_error_is_set(&error) || strcmp(error.name, DBUS_ERROR_UNKNOWN_METHOD))
            goto fail;

        /* If the method GetAPIVersion is not known, we look if
         * GetVersionString matches "avahi 0.6" which is the only
         * version we support which doesn't have GetAPIVersion() .*/

        dbus_message_unref(message);
        if (reply) dbus_message_unref(reply);
        dbus_error_free(&error);

        if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "GetVersionString")))
            goto fail;

        reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error);

        if (!reply || dbus_error_is_set (&error))
            goto fail;

        if (!dbus_message_get_args (reply, &error, DBUS_TYPE_STRING, &version_str, DBUS_TYPE_INVALID) ||
            dbus_error_is_set (&error))
            goto fail;

        version = strcmp(version_str, "avahi 0.6") == 0 ? 0x0201 : 0x0000;

    } else {

        if (!dbus_message_get_args (reply, &error, DBUS_TYPE_UINT32, &version, DBUS_TYPE_INVALID) ||
            dbus_error_is_set(&error))
            goto fail;
    }

    /*fprintf(stderr, "API Version 0x%04x\n", version);*/

    if ((version & 0xFF00) != (AVAHI_CLIENT_DBUS_API_SUPPORTED & 0xFF00) ||
        (version & 0x00FF) < (AVAHI_CLIENT_DBUS_API_SUPPORTED & 0x00FF)) {
        e = AVAHI_ERR_VERSION_MISMATCH;
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return AVAHI_OK;

fail:
    if (dbus_error_is_set(&error)) {
        e = avahi_error_dbus_to_number (error.name);
        dbus_error_free(&error);
    }

    if (ret_error)
        *ret_error = e;

    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);

    return e;
}

static int init_server(AvahiClient *client, int *ret_error) {
    int r;

    if ((r = check_version(client, ret_error)) < 0)
        return r;

    if ((r = get_server_state(client, ret_error)) < 0)
        return r;

    return AVAHI_OK;
}

/* This function acts like dbus_bus_get but creates a private
 * connection instead.  */
static DBusConnection* avahi_dbus_bus_get(DBusError *error) {
    DBusConnection *c;

#ifdef HAVE_DBUS_BUS_GET_PRIVATE
    if (!(c = dbus_bus_get_private(DBUS_BUS_SYSTEM, error)))
        return NULL;

    dbus_connection_set_exit_on_disconnect(c, FALSE);
#else
    const char *a;

    if (!(a = getenv("DBUS_SYSTEM_BUS_ADDRESS")) || !*a)
        a = DBUS_SYSTEM_BUS_DEFAULT_ADDRESS;

    if (!(c = dbus_connection_open_private(a, error)))
        return NULL;

    dbus_connection_set_exit_on_disconnect(c, FALSE);

    if (!dbus_bus_register(c, error)) {
#ifdef HAVE_DBUS_CONNECTION_CLOSE
        dbus_connection_close(c);
#else
        dbus_connection_disconnect(c);
#endif
        dbus_connection_unref(c);
        return NULL;
    }
#endif

    return c;
}

AvahiClient *avahi_client_new(const AvahiPoll *poll_api, AvahiClientFlags flags, AvahiClientCallback callback, void *userdata, int *ret_error) {
    AvahiClient *client = NULL;
    DBusError error;
    DBusMessage *message = NULL, *reply = NULL;

    avahi_init_i18n();

    dbus_error_init(&error);

    if (!(client = avahi_new(AvahiClient, 1))) {
        if (ret_error)
            *ret_error = AVAHI_ERR_NO_MEMORY;
        goto fail;
    }

    client->poll_api = poll_api;
    client->error = AVAHI_OK;
    client->callback = callback;
    client->userdata = userdata;
    client->state = (AvahiClientState) -1;
    client->flags = flags;

    client->host_name = NULL;
    client->host_name_fqdn = NULL;
    client->domain_name = NULL;
    client->version_string = NULL;
    client->local_service_cookie_valid = 0;

    AVAHI_LLIST_HEAD_INIT(AvahiEntryGroup, client->groups);
    AVAHI_LLIST_HEAD_INIT(AvahiDomainBrowser, client->domain_browsers);
    AVAHI_LLIST_HEAD_INIT(AvahiServiceBrowser, client->service_browsers);
    AVAHI_LLIST_HEAD_INIT(AvahiServiceTypeBrowser, client->service_type_browsers);
    AVAHI_LLIST_HEAD_INIT(AvahiServiceResolver, client->service_resolvers);
    AVAHI_LLIST_HEAD_INIT(AvahiHostNameResolver, client->host_name_resolvers);
    AVAHI_LLIST_HEAD_INIT(AvahiAddressResolver, client->address_resolvers);
    AVAHI_LLIST_HEAD_INIT(AvahiRecordBrowser, client->record_browsers);

    if (!(client->bus = avahi_dbus_bus_get(&error)) || dbus_error_is_set(&error)) {
        if (ret_error)
            *ret_error = AVAHI_ERR_DBUS_ERROR;
        goto fail;
    }

    if (avahi_dbus_connection_glue(client->bus, poll_api) < 0) {
        if (ret_error)
            *ret_error = AVAHI_ERR_NO_MEMORY; /* Not optimal */
        goto fail;
    }

    if (!dbus_connection_add_filter(client->bus, filter_func, client, NULL)) {
        if (ret_error)
            *ret_error = AVAHI_ERR_NO_MEMORY;
        goto fail;
    }

    dbus_bus_add_match(
        client->bus,
        "type='signal', "
        "interface='" AVAHI_DBUS_INTERFACE_SERVER "', "
        "sender='" AVAHI_DBUS_NAME "', "
        "path='" AVAHI_DBUS_PATH_SERVER "'",
        &error);

    if (dbus_error_is_set(&error))
        goto fail;

    dbus_bus_add_match (
        client->bus,
        "type='signal', "
        "interface='" DBUS_INTERFACE_DBUS "', "
        "sender='" DBUS_SERVICE_DBUS "', "
        "path='" DBUS_PATH_DBUS "'",
        &error);

    if (dbus_error_is_set(&error))
        goto fail;

    dbus_bus_add_match(
        client->bus,
        "type='signal', "
        "interface='" DBUS_INTERFACE_LOCAL "'",
        &error);

    if (dbus_error_is_set(&error))
        goto fail;

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, "org.freedesktop.DBus.Peer", "Ping")))
        goto fail;

    reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error);

    if (!reply || dbus_error_is_set (&error)) {
        /* We free the error so its not set, that way the fail target
         * will return the NO_DAEMON error rather than a DBUS error */
        dbus_error_free(&error);

        if (!(flags & AVAHI_CLIENT_NO_FAIL)) {

            if (ret_error)
                *ret_error = AVAHI_ERR_NO_DAEMON;

            goto fail;
        }

        /* The user doesn't want this call to fail if the daemon is not
         * available, so let's return succesfully */
        client_set_state(client, AVAHI_CLIENT_CONNECTING);

    } else {

        if (init_server(client, ret_error) < 0)
            goto fail;
    }

    dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return client;

fail:

    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);

    if (client)
        avahi_client_free(client);

    if (dbus_error_is_set(&error)) {

        if (ret_error) {
            if (strcmp(error.name, DBUS_ERROR_FILE_NOT_FOUND) == 0)
                /* DBUS returns this error when the DBUS daemon is not running */
                *ret_error = AVAHI_ERR_NO_DAEMON;
            else
                *ret_error = avahi_error_dbus_to_number(error.name);
        }

        dbus_error_free(&error);
    }

    return NULL;
}

void avahi_client_free(AvahiClient *client) {
    assert(client);

    if (client->bus)
        /* Disconnect in advance, so that the free() functions won't
         * issue needless server calls */
#ifdef HAVE_DBUS_CONNECTION_CLOSE
        dbus_connection_close(client->bus);
#else
        dbus_connection_disconnect(client->bus);
#endif

    while (client->groups)
        avahi_entry_group_free(client->groups);

    while (client->domain_browsers)
        avahi_domain_browser_free(client->domain_browsers);

    while (client->service_browsers)
        avahi_service_browser_free(client->service_browsers);

    while (client->service_type_browsers)
        avahi_service_type_browser_free(client->service_type_browsers);

    while (client->service_resolvers)
        avahi_service_resolver_free(client->service_resolvers);

    while (client->host_name_resolvers)
        avahi_host_name_resolver_free(client->host_name_resolvers);

    while (client->address_resolvers)
        avahi_address_resolver_free(client->address_resolvers);

    while (client->record_browsers)
        avahi_record_browser_free(client->record_browsers);

    if (client->bus)
        dbus_connection_unref(client->bus);

    avahi_free(client->version_string);
    avahi_free(client->host_name);
    avahi_free(client->host_name_fqdn);
    avahi_free(client->domain_name);

    avahi_free(client);
}

static char* avahi_client_get_string_reply_and_block (AvahiClient *client, const char *method, const char *param) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    char *ret, *n;

    assert(client);
    assert(method);

    dbus_error_init (&error);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, method))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (param) {
        if (!dbus_message_append_args (message, DBUS_TYPE_STRING, &param, DBUS_TYPE_INVALID)) {
            avahi_client_set_errno (client, AVAHI_ERR_NO_MEMORY);
            goto fail;
        }
    }

    reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error);

    if (!reply || dbus_error_is_set (&error))
        goto fail;

    if (!dbus_message_get_args (reply, &error, DBUS_TYPE_STRING, &ret, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error))
        goto fail;

    if (!(n = avahi_strdup(ret))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return n;

fail:

    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    return NULL;
}

const char* avahi_client_get_version_string(AvahiClient *client) {
    assert(client);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        return NULL;
    }

    if (!client->version_string)
        client->version_string = avahi_client_get_string_reply_and_block(client, "GetVersionString", NULL);

    return client->version_string;
}

const char* avahi_client_get_domain_name(AvahiClient *client) {
    assert(client);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        return NULL;
    }

    if (!client->domain_name)
        client->domain_name = avahi_client_get_string_reply_and_block(client, "GetDomainName", NULL);

    return client->domain_name;
}

const char* avahi_client_get_host_name(AvahiClient *client) {
    assert(client);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        return NULL;
    }

    if (!client->host_name)
        client->host_name = avahi_client_get_string_reply_and_block(client, "GetHostName", NULL);

    return client->host_name;
}

const char* avahi_client_get_host_name_fqdn (AvahiClient *client) {
    assert(client);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        return NULL;
    }

    if (!client->host_name_fqdn)
        client->host_name_fqdn = avahi_client_get_string_reply_and_block(client, "GetHostNameFqdn", NULL);

    return client->host_name_fqdn;
}

AvahiClientState avahi_client_get_state(AvahiClient *client) {
    assert(client);

    return client->state;
}

int avahi_client_errno(AvahiClient *client) {
    assert(client);

    return client->error;
}

/* Just for internal use */
int avahi_client_simple_method_call(AvahiClient *client, const char *path, const char *interface, const char *method) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    int r = AVAHI_OK;

    dbus_error_init(&error);

    assert(client);
    assert(path);
    assert(interface);
    assert(method);

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, path, interface, method))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block(client->bus, message, -1, &error)) ||
        dbus_error_is_set (&error)) {
        r = avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error)) {
        r = avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return AVAHI_OK;

fail:
    if (dbus_error_is_set(&error)) {
        r = avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return r;
}

uint32_t avahi_client_get_local_service_cookie(AvahiClient *client) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    assert(client);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        return AVAHI_SERVICE_COOKIE_INVALID;
    }

    if (client->local_service_cookie_valid)
        return client->local_service_cookie;

    dbus_error_init (&error);

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "GetLocalServiceCookie"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error);

    if (!reply || dbus_error_is_set (&error))
        goto fail;

    if (!dbus_message_get_args (reply, &error, DBUS_TYPE_UINT32, &client->local_service_cookie, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error))
        goto fail;

    dbus_message_unref(message);
    dbus_message_unref(reply);

    client->local_service_cookie_valid = 1;
    return client->local_service_cookie;

fail:

    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    return AVAHI_SERVICE_COOKIE_INVALID;
}

int avahi_client_is_connected(AvahiClient *client) {
    assert(client);

    return
        client->bus &&
        dbus_connection_get_is_connected(client->bus) &&
        (client->state == AVAHI_CLIENT_S_RUNNING || client->state == AVAHI_CLIENT_S_REGISTERING || client->state == AVAHI_CLIENT_S_COLLISION);
}

int avahi_client_set_host_name(AvahiClient* client, const char *name) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;

    assert(client);

    if (!avahi_client_is_connected(client))
        return avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);

    dbus_error_init (&error);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "SetHostName"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!dbus_message_append_args (message, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID)) {
        avahi_client_set_errno (client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    reply = dbus_connection_send_with_reply_and_block(client->bus, message, -1, &error);

    if (!reply || dbus_error_is_set (&error))
        goto fail;

    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error))
        goto fail;

    dbus_message_unref(message);
    dbus_message_unref(reply);

    avahi_free(client->host_name);
    client->host_name = NULL;
    avahi_free(client->host_name_fqdn);
    client->host_name_fqdn = NULL;

    return 0;

fail:

    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    return client->error;
}
