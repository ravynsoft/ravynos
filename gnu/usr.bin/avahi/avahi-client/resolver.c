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

#include "client.h"
#include "internal.h"

/* AvahiServiceResolver implementation */

DBusHandlerResult avahi_service_resolver_event (AvahiClient *client, AvahiResolverEvent event, DBusMessage *message) {
    AvahiServiceResolver *r = NULL;
    DBusError error;
    const char *path;
    AvahiStringList *strlst = NULL;

    assert(client);
    assert(message);

    dbus_error_init (&error);

    if (!(path = dbus_message_get_path(message)))
        goto fail;

    for (r = client->service_resolvers; r; r = r->service_resolvers_next)
        if (strcmp (r->path, path) == 0)
            break;

    if (!r)
        goto fail;

    switch (event) {
        case AVAHI_RESOLVER_FOUND: {
            int j;
            int32_t interface, protocol, aprotocol;
            uint32_t flags;
            char *name, *type, *domain, *host, *address;
            uint16_t port;
            DBusMessageIter iter, sub;
            AvahiAddress a;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_INT32, &interface,
                    DBUS_TYPE_INT32, &protocol,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_STRING, &type,
                    DBUS_TYPE_STRING, &domain,
                    DBUS_TYPE_STRING, &host,
                    DBUS_TYPE_INT32, &aprotocol,
                    DBUS_TYPE_STRING, &address,
                    DBUS_TYPE_UINT16, &port,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {

                fprintf(stderr, "Failed to parse resolver event.\n");
                goto fail;
            }

            dbus_message_iter_init(message, &iter);

            for (j = 0; j < 9; j++)
                dbus_message_iter_next(&iter);

            if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
                dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_ARRAY) {
                fprintf(stderr, "Error parsing service resolving message\n");
                goto fail;
            }

            strlst = NULL;
            dbus_message_iter_recurse(&iter, &sub);

            for (;;) {
                DBusMessageIter sub2;
                int at;
                const uint8_t *k;
                int n;

                if ((at = dbus_message_iter_get_arg_type(&sub)) == DBUS_TYPE_INVALID)
                    break;

                assert(at == DBUS_TYPE_ARRAY);

                if (dbus_message_iter_get_element_type(&sub) != DBUS_TYPE_BYTE) {
                    fprintf(stderr, "Error parsing service resolving message\n");
                    goto fail;
                }

                dbus_message_iter_recurse(&sub, &sub2);

                k = NULL; n = 0;
                dbus_message_iter_get_fixed_array(&sub2, &k, &n);
                if (k && n > 0)
                    strlst = avahi_string_list_add_arbitrary(strlst, k, n);

                dbus_message_iter_next(&sub);
            }

            dbus_message_iter_next(&iter);

            if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT32) {
                fprintf(stderr, "Failed to parse resolver event.\n");
                goto fail;
            }

            dbus_message_iter_get_basic(&iter, &flags);

            assert(address);

            if (address[0] == 0)
                address = NULL;
	    else
            	avahi_address_parse(address, (AvahiProtocol) aprotocol, &a);

            r->callback(r, (AvahiIfIndex) interface, (AvahiProtocol) protocol, AVAHI_RESOLVER_FOUND, name, type, domain, host, address ? &a : NULL, port, strlst, (AvahiLookupResultFlags) flags, r->userdata);

            avahi_string_list_free(strlst);
            break;
        }

        case AVAHI_RESOLVER_FAILURE: {
            char *etxt;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_STRING, &etxt,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse resolver event.\n");
                goto fail;
            }

            avahi_client_set_errno(r->client, avahi_error_dbus_to_number(etxt));
            r->callback(r, r->interface, r->protocol, event, r->name, r->type, r->domain, NULL, NULL, 0, NULL, 0, r->userdata);
            break;
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;


fail:
    dbus_error_free (&error);
    avahi_string_list_free(strlst);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

AvahiServiceResolver * avahi_service_resolver_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    const char *type,
    const char *domain,
    AvahiProtocol aprotocol,
    AvahiLookupFlags flags,
    AvahiServiceResolverCallback callback,
    void *userdata) {

    DBusError error;
    AvahiServiceResolver *r = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    int32_t i_interface, i_protocol, i_aprotocol;
    uint32_t u_flags;
    char *path;

    assert(client);
    assert(type);

    if (!domain)
        domain = "";

    if (!name)
        name = "";

    dbus_error_init (&error);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!(r = avahi_new(AvahiServiceResolver, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    r->client = client;
    r->callback = callback;
    r->userdata = userdata;
    r->path = NULL;
    r->name = r->type = r->domain = NULL;
    r->interface = interface;
    r->protocol = protocol;

    AVAHI_LLIST_PREPEND(AvahiServiceResolver, service_resolvers, client->service_resolvers, r);

    if (name && name[0])
        if (!(r->name = avahi_strdup(name))) {
            avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
            goto fail;
        }

    if (!(r->type = avahi_strdup(type))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (domain && domain[0])
        if (!(r->domain = avahi_strdup(domain))) {
            avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
            goto fail;
        }


    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "ServiceResolverNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    i_aprotocol = (int32_t) aprotocol;
    u_flags = (uint32_t) flags;

    if (!(dbus_message_append_args(
              message,
              DBUS_TYPE_INT32, &i_interface,
              DBUS_TYPE_INT32, &i_protocol,
              DBUS_TYPE_STRING, &name,
              DBUS_TYPE_STRING, &type,
              DBUS_TYPE_STRING, &domain,
              DBUS_TYPE_INT32, &i_aprotocol,
              DBUS_TYPE_UINT32, &u_flags,
              DBUS_TYPE_INVALID))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block(client->bus, message, -1, &error)) ||
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

    if (!(r->path = avahi_strdup(path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }


    dbus_message_unref(message);
    dbus_message_unref(reply);

    return r;

fail:

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (r)
        avahi_service_resolver_free(r);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;

}

AvahiClient* avahi_service_resolver_get_client (AvahiServiceResolver *r) {
    assert (r);

    return r->client;
}

int avahi_service_resolver_free(AvahiServiceResolver *r) {
    AvahiClient *client;
    int ret = AVAHI_OK;

    assert(r);
    client = r->client;

    if (r->path && avahi_client_is_connected(client))
        ret = avahi_client_simple_method_call(client, r->path, AVAHI_DBUS_INTERFACE_SERVICE_RESOLVER, "Free");

    AVAHI_LLIST_REMOVE(AvahiServiceResolver, service_resolvers, client->service_resolvers, r);

    avahi_free(r->path);
    avahi_free(r->name);
    avahi_free(r->type);
    avahi_free(r->domain);
    avahi_free(r);

    return ret;
}

/* AvahiHostNameResolver implementation */

DBusHandlerResult avahi_host_name_resolver_event (AvahiClient *client, AvahiResolverEvent event, DBusMessage *message) {
    AvahiHostNameResolver *r = NULL;
    DBusError error;
    const char *path;

    assert(client);
    assert(message);

    dbus_error_init (&error);

    if (!(path = dbus_message_get_path(message)))
        goto fail;

    for (r = client->host_name_resolvers; r; r = r->host_name_resolvers_next)
        if (strcmp (r->path, path) == 0)
            break;

    if (!r)
        goto fail;

    switch (event) {
        case AVAHI_RESOLVER_FOUND: {
            int32_t interface, protocol, aprotocol;
            uint32_t flags;
            char *name, *address;
            AvahiAddress a;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_INT32, &interface,
                    DBUS_TYPE_INT32, &protocol,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INT32, &aprotocol,
                    DBUS_TYPE_STRING, &address,
                    DBUS_TYPE_UINT32, &flags,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse resolver event.\n");
                goto fail;
            }

            assert(address);
            if (!avahi_address_parse(address, (AvahiProtocol) aprotocol, &a)) {
                fprintf(stderr, "Failed to parse address\n");
                goto fail;
            }

            r->callback(r, (AvahiIfIndex) interface, (AvahiProtocol) protocol, AVAHI_RESOLVER_FOUND, name, &a, (AvahiLookupResultFlags) flags, r->userdata);
            break;
        }

        case AVAHI_RESOLVER_FAILURE: {
            char *etxt;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_STRING, &etxt,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse resolver event.\n");
                goto fail;
            }

            avahi_client_set_errno(r->client, avahi_error_dbus_to_number(etxt));
            r->callback(r, r->interface, r->protocol, event, r->host_name, NULL, 0, r->userdata);
            break;
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;

fail:
    dbus_error_free (&error);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


AvahiHostNameResolver * avahi_host_name_resolver_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const char *name,
    AvahiProtocol aprotocol,
    AvahiLookupFlags flags,
    AvahiHostNameResolverCallback callback,
    void *userdata) {

    DBusError error;
    AvahiHostNameResolver *r = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    int32_t i_interface, i_protocol, i_aprotocol;
    uint32_t u_flags;
    char *path;

    assert(client);
    assert(name);

    dbus_error_init (&error);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!(r = avahi_new(AvahiHostNameResolver, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    r->client = client;
    r->callback = callback;
    r->userdata = userdata;
    r->path = NULL;
    r->interface = interface;
    r->protocol = protocol;
    r->host_name = NULL;

    AVAHI_LLIST_PREPEND(AvahiHostNameResolver, host_name_resolvers, client->host_name_resolvers, r);

    if (!(r->host_name = avahi_strdup(name))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "HostNameResolverNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    i_aprotocol = (int32_t) aprotocol;
    u_flags = (uint32_t) flags;

    if (!(dbus_message_append_args(
              message,
              DBUS_TYPE_INT32, &i_interface,
              DBUS_TYPE_INT32, &i_protocol,
              DBUS_TYPE_STRING, &name,
              DBUS_TYPE_INT32, &i_aprotocol,
              DBUS_TYPE_UINT32, &u_flags,
              DBUS_TYPE_INVALID))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block(client->bus, message, -1, &error)) ||
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

    if (!(r->path = avahi_strdup(path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return r;

fail:

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (r)
        avahi_host_name_resolver_free(r);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;

}

int avahi_host_name_resolver_free(AvahiHostNameResolver *r) {
    int ret = AVAHI_OK;
    AvahiClient *client;

    assert(r);
    client = r->client;

    if (r->path && avahi_client_is_connected(client))
        ret = avahi_client_simple_method_call(client, r->path, AVAHI_DBUS_INTERFACE_HOST_NAME_RESOLVER, "Free");

    AVAHI_LLIST_REMOVE(AvahiHostNameResolver, host_name_resolvers, client->host_name_resolvers, r);

    avahi_free(r->path);
    avahi_free(r->host_name);
    avahi_free(r);

    return ret;
}

AvahiClient* avahi_host_name_resolver_get_client (AvahiHostNameResolver *r) {
    assert (r);

    return r->client;
}

/* AvahiAddressResolver implementation */

DBusHandlerResult avahi_address_resolver_event (AvahiClient *client, AvahiResolverEvent event, DBusMessage *message) {
    AvahiAddressResolver *r = NULL;
    DBusError error;
    const char *path;

    assert(client);
    assert(message);

    dbus_error_init (&error);

    if (!(path = dbus_message_get_path(message)))
        goto fail;

    for (r = client->address_resolvers; r; r = r->address_resolvers_next)
        if (strcmp (r->path, path) == 0)
            break;

    if (!r)
        goto fail;

    switch (event) {
        case AVAHI_RESOLVER_FOUND: {
            int32_t interface, protocol, aprotocol;
            uint32_t flags;
            char *name, *address;
            AvahiAddress a;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_INT32, &interface,
                    DBUS_TYPE_INT32, &protocol,
                    DBUS_TYPE_INT32, &aprotocol,
                    DBUS_TYPE_STRING, &address,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_UINT32, &flags,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse resolver event.\n");
                goto fail;
            }

            assert(address);
            if (!avahi_address_parse(address, (AvahiProtocol) aprotocol, &a)) {
                fprintf(stderr, "Failed to parse address\n");
                goto fail;
            }

            r->callback(r, (AvahiIfIndex) interface, (AvahiProtocol) protocol, AVAHI_RESOLVER_FOUND, &a, name, (AvahiLookupResultFlags) flags, r->userdata);
            break;
        }

        case AVAHI_RESOLVER_FAILURE: {
            char *etxt;

            if (!dbus_message_get_args(
                    message, &error,
                    DBUS_TYPE_STRING, &etxt,
                    DBUS_TYPE_INVALID) ||
                dbus_error_is_set (&error)) {
                fprintf(stderr, "Failed to parse resolver event.\n");
                goto fail;
            }

            avahi_client_set_errno(r->client, avahi_error_dbus_to_number(etxt));
            r->callback(r, r->interface, r->protocol, event, &r->address, NULL, 0, r->userdata);
            break;
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;

fail:
    dbus_error_free (&error);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

AvahiAddressResolver * avahi_address_resolver_new(
    AvahiClient *client,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    const AvahiAddress *a,
    AvahiLookupFlags flags,
    AvahiAddressResolverCallback callback,
    void *userdata) {

    DBusError error;
    AvahiAddressResolver *r = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;
    char *path;
    char addr[AVAHI_ADDRESS_STR_MAX], *address = addr;

    assert(client);
    assert(a);

    dbus_error_init (&error);

    if (!avahi_address_snprint (addr, sizeof(addr), a)) {
        avahi_client_set_errno(client, AVAHI_ERR_INVALID_ADDRESS);
        return NULL;
    }

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!(r = avahi_new(AvahiAddressResolver, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    r->client = client;
    r->callback = callback;
    r->userdata = userdata;
    r->path = NULL;
    r->interface = interface;
    r->protocol = protocol;
    r->address = *a;

    AVAHI_LLIST_PREPEND(AvahiAddressResolver, address_resolvers, client->address_resolvers, r);

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "AddressResolverNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!(dbus_message_append_args(
              message,
              DBUS_TYPE_INT32, &i_interface,
              DBUS_TYPE_INT32, &i_protocol,
              DBUS_TYPE_STRING, &address,
              DBUS_TYPE_UINT32, &u_flags,
              DBUS_TYPE_INVALID))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block(client->bus, message, -1, &error)) ||
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

    if (!(r->path = avahi_strdup(path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return r;

fail:

    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (r)
        avahi_address_resolver_free(r);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;

}

AvahiClient* avahi_address_resolver_get_client (AvahiAddressResolver *r) {
    assert (r);

    return r->client;
}

int avahi_address_resolver_free(AvahiAddressResolver *r) {
    AvahiClient *client;
    int ret = AVAHI_OK;

    assert(r);
    client = r->client;

    if (r->path && avahi_client_is_connected(client))
        ret = avahi_client_simple_method_call(client, r->path, AVAHI_DBUS_INTERFACE_ADDRESS_RESOLVER, "Free");

    AVAHI_LLIST_REMOVE(AvahiAddressResolver, address_resolvers, client->address_resolvers, r);

    avahi_free(r->path);
    avahi_free(r);

    return ret;
}

