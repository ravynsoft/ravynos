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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include <dbus/dbus.h>

#include <avahi-common/dbus.h>
#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>
#include <avahi-common/dbus-watch-glue.h>
#include <avahi-common/alternative.h>
#include <avahi-common/error.h>
#include <avahi-common/domain.h>
#include <avahi-common/timeval.h>

#include <avahi-core/log.h>
#include <avahi-core/core.h>
#include <avahi-core/lookup.h>
#include <avahi-core/publish.h>

#include "dbus-protocol.h"
#include "dbus-util.h"
#include "dbus-internal.h"
#include "main.h"

/* #define VALGRIND_WORKAROUND 1 */

#define RECONNECT_MSEC 3000

Server *server = NULL;

static int dbus_connect(void);
static void dbus_disconnect(void);

static void client_free(Client *c) {

    assert(server);
    assert(c);

    while (c->entry_groups)
        avahi_dbus_entry_group_free(c->entry_groups);

    while (c->sync_host_name_resolvers)
        avahi_dbus_sync_host_name_resolver_free(c->sync_host_name_resolvers);

    while (c->async_host_name_resolvers)
        avahi_dbus_async_host_name_resolver_free(c->async_host_name_resolvers);

    while (c->sync_address_resolvers)
        avahi_dbus_sync_address_resolver_free(c->sync_address_resolvers);

    while (c->async_address_resolvers)
        avahi_dbus_async_address_resolver_free(c->async_address_resolvers);

    while (c->domain_browsers)
        avahi_dbus_domain_browser_free(c->domain_browsers);

    while (c->service_type_browsers)
        avahi_dbus_service_type_browser_free(c->service_type_browsers);

    while (c->service_browsers)
        avahi_dbus_service_browser_free(c->service_browsers);

    while (c->sync_service_resolvers)
        avahi_dbus_sync_service_resolver_free(c->sync_service_resolvers);

    while (c->async_service_resolvers)
        avahi_dbus_async_service_resolver_free(c->async_service_resolvers);

    while (c->record_browsers)
        avahi_dbus_record_browser_free(c->record_browsers);

    assert(c->n_objects == 0);

    avahi_free(c->name);
    AVAHI_LLIST_REMOVE(Client, clients, server->clients, c);
    avahi_free(c);

    assert(server->n_clients >= 1);
    server->n_clients --;
}

static Client *client_get(const char *name, int create) {
    Client *client;

    assert(server);
    assert(name);

    for (client = server->clients; client; client = client->clients_next)
        if (!strcmp(name, client->name))
            return client;

    if (!create)
        return NULL;

    if (server->n_clients >= server->n_clients_max)
        return NULL;

    /* If not existent yet, create a new entry */
    client = avahi_new(Client, 1);
    client->id = server->current_id++;
    client->name = avahi_strdup(name);
    client->current_id = 0;
    client->n_objects = 0;

    AVAHI_LLIST_HEAD_INIT(EntryGroupInfo, client->entry_groups);
    AVAHI_LLIST_HEAD_INIT(SyncHostNameResolverInfo, client->sync_host_name_resolvers);
    AVAHI_LLIST_HEAD_INIT(AsyncHostNameResolverInfo, client->async_host_name_resolvers);
    AVAHI_LLIST_HEAD_INIT(SyncAddressResolverInfo, client->sync_address_resolvers);
    AVAHI_LLIST_HEAD_INIT(AsyncAddressResolverInfo, client->async_address_resolvers);
    AVAHI_LLIST_HEAD_INIT(DomainBrowserInfo, client->domain_browsers);
    AVAHI_LLIST_HEAD_INIT(ServiceTypeBrowserInfo, client->service_type_browsers);
    AVAHI_LLIST_HEAD_INIT(ServiceBrowserInfo, client->service_browsers);
    AVAHI_LLIST_HEAD_INIT(SyncServiceResolverInfo, client->sync_service_resolvers);
    AVAHI_LLIST_HEAD_INIT(AsyncServiceResolverInfo, client->async_service_resolvers);
    AVAHI_LLIST_HEAD_INIT(RecordBrowserInfo, client->record_browsers);

    AVAHI_LLIST_PREPEND(Client, clients, server->clients, client);

    server->n_clients++;
    assert(server->n_clients > 0);

    return client;
}

static void reconnect_callback(AvahiTimeout *t, AVAHI_GCC_UNUSED void *userdata) {
    assert(!server->bus);

    if (dbus_connect() < 0) {
        struct timeval tv;
        avahi_log_debug(__FILE__": Connection failed, retrying in %ims...", RECONNECT_MSEC);
        avahi_elapse_time(&tv, RECONNECT_MSEC, 0);
        server->poll_api->timeout_update(t, &tv);
    } else {
        avahi_log_debug(__FILE__": Successfully reconnected.");
        server->poll_api->timeout_update(t, NULL);
    }
}

static DBusHandlerResult dbus_parsing_error(const char *txt, DBusError *error) {
    avahi_log_warn("%s",txt);
    if (dbus_error_is_set(error))
        dbus_error_free(error);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult msg_signal_filter_impl(AVAHI_GCC_UNUSED DBusConnection *c, DBusMessage *m, AVAHI_GCC_UNUSED void *userdata) {
    DBusError error;

    dbus_error_init(&error);

/*     avahi_log_debug(__FILE__": interface=%s, path=%s, member=%s", */
/*                     dbus_message_get_interface(m), */
/*                     dbus_message_get_path(m), */
/*                     dbus_message_get_member(m)); */

    if (dbus_message_is_signal(m, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        struct timeval tv;

        if (server->reconnect) {
            avahi_log_warn("Disconnected from D-Bus, trying to reconnect in %ims...", RECONNECT_MSEC);

            dbus_disconnect();

            avahi_elapse_time(&tv, RECONNECT_MSEC, 0);

            if (server->reconnect_timeout)
                server->poll_api->timeout_update(server->reconnect_timeout, &tv);
            else
                server->reconnect_timeout = server->poll_api->timeout_new(server->poll_api, &tv, reconnect_callback, NULL);
        } else {
            avahi_log_warn("Disconnected from D-Bus, exiting.");
            raise(SIGTERM);
        }

        return DBUS_HANDLER_RESULT_HANDLED;

    } else if (dbus_message_is_signal(m, DBUS_INTERFACE_DBUS, "NameAcquired")) {
        char *name;

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID)) {
            return dbus_parsing_error("Error parsing NameAcquired message", &error);
        }

/*         avahi_log_info(__FILE__": name acquired (%s)", name); */
        return DBUS_HANDLER_RESULT_HANDLED;

    } else if (dbus_message_is_signal(m, DBUS_INTERFACE_DBUS, "NameOwnerChanged")) {
        char *name, *old, *new;

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &old, DBUS_TYPE_STRING, &new, DBUS_TYPE_INVALID)) {
            return dbus_parsing_error("Error parsing NameOwnerChanged message", &error);
        }

        if (!*new) {
            Client *client;

            if ((client = client_get(name, FALSE))) {
                avahi_log_debug(__FILE__": client %s vanished.", name);
                client_free(client);
            }
        }
    }

    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult dbus_get_host_name(DBusConnection *c, DBusMessage *m, DBusError *error) {
    if (!dbus_message_get_args(m, error, DBUS_TYPE_INVALID)) {
        return dbus_parsing_error("Error parsing Server::GetHostName message", error);
    }

    return avahi_dbus_respond_string(c, m, avahi_server_get_host_name(avahi_server));
}

static DBusHandlerResult dbus_set_host_name(DBusConnection *c, DBusMessage *m, DBusError *error) {
    char *name;

    if (!dbus_message_get_args(m, error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID)) {
        return dbus_parsing_error("Error parsing Server::SetHostName message", error);
    }

    if (avahi_server_set_host_name(avahi_server, name) < 0)
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);

    avahi_log_info("Changing host name to '%s'.", name);

    return avahi_dbus_respond_ok(c, m);
}

static DBusHandlerResult dbus_get_domain_name(DBusConnection *c, DBusMessage *m, DBusError *error) {
    if (!dbus_message_get_args(m, error, DBUS_TYPE_INVALID)) {
        return dbus_parsing_error("Error parsing Server::GetDomainName message", error);
    }

    return avahi_dbus_respond_string(c, m, avahi_server_get_domain_name(avahi_server));
}

static DBusHandlerResult dbus_get_host_name_fqdn(DBusConnection *c, DBusMessage *m, DBusError *error) {
    if (!(dbus_message_get_args(m, error, DBUS_TYPE_INVALID))) {
        return dbus_parsing_error("Error parsing Server::GetHostNameFqdn message", error);
    }

    return avahi_dbus_respond_string(c, m, avahi_server_get_host_name_fqdn(avahi_server));
}

static DBusHandlerResult dbus_is_ns_support_available(DBusConnection *c, DBusMessage *m, DBusError *error) {
    if (!(dbus_message_get_args(m, error, DBUS_TYPE_INVALID))) {
        return dbus_parsing_error("Error parsing Server::IsNSSSupportAvailable message", error);
    }

    return avahi_dbus_respond_boolean(c, m, nss_support);
}

static DBusHandlerResult dbus_get_version_string(DBusConnection *c, DBusMessage *m, DBusError *error) {
    if (!(dbus_message_get_args(m, error, DBUS_TYPE_INVALID))) {
        return dbus_parsing_error("Error parsing Server::GetVersionString message", error);
    }

    return avahi_dbus_respond_string(c, m, PACKAGE_STRING);
}

static DBusHandlerResult dbus_get_api_version(DBusConnection *c, DBusMessage *m, DBusError *error) {
    if (!(dbus_message_get_args(m, error, DBUS_TYPE_INVALID))) {
        return dbus_parsing_error("Error parsing Server::GetAPIVersion message", error);
    }

    return avahi_dbus_respond_uint32(c, m, AVAHI_DBUS_API_VERSION);
}

static DBusHandlerResult dbus_get_state(DBusConnection *c, DBusMessage *m, DBusError *error) {
    AvahiServerState state;

    if (!(dbus_message_get_args(m, error, DBUS_TYPE_INVALID))) {
        return dbus_parsing_error("Error parsing Server::GetState message", error);
    }

    state = avahi_server_get_state(avahi_server);
    return avahi_dbus_respond_int32(c, m, (int32_t) state);
}

static DBusHandlerResult dbus_get_local_service_cookie(DBusConnection *c, DBusMessage *m, DBusError *error) {
    if (!(dbus_message_get_args(m, error, DBUS_TYPE_INVALID))) {
        return dbus_parsing_error("Error parsing Server::GetLocalServiceCookie message", error);
    }

    return avahi_dbus_respond_uint32(c, m, avahi_server_get_local_service_cookie(avahi_server));
}

static DBusHandlerResult dbus_get_net_if_by_index(DBusConnection *c, DBusMessage *m, DBusError *error) {
    int32_t idx;
    char name[IF_NAMESIZE];

    if (!(dbus_message_get_args(m, error, DBUS_TYPE_INT32, &idx, DBUS_TYPE_INVALID))) {
        return dbus_parsing_error("Error parsing Server::GetNetworkInterfaceNameByIndex message", error);
    }

#ifdef VALGRIND_WORKAROUND
    return respond_string(c, m, "blah");
#else
    if ((!if_indextoname(idx, name))) {
        char txt[256];
        snprintf(txt, sizeof(txt), "OS Error: %s", strerror(errno));
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_OS, txt);
    }

    return avahi_dbus_respond_string(c, m, name);
#endif
}

static DBusHandlerResult dbus_get_net_if_by_name(DBusConnection *c, DBusMessage *m, DBusError *error) {
    char *n;
    int32_t idx;

    if (!(dbus_message_get_args(m, error, DBUS_TYPE_STRING, &n, DBUS_TYPE_INVALID)) || !n) {
        return dbus_parsing_error("Error parsing Server::GetNetworkInterfaceIndexByName message", error);
    }

#ifdef VALGRIND_WORKAROUND
    return respond_int32(c, m, 1);
#else
    if (!(idx = if_nametoindex(n))) {
        char txt[256];
        snprintf(txt, sizeof(txt), "OS Error: %s", strerror(errno));
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_OS, txt);
    }

    return avahi_dbus_respond_int32(c, m, idx);
#endif
}

static DBusHandlerResult dbus_get_alternative_host_name(DBusConnection *c, DBusMessage *m, DBusError *error) {
    char *n, * t;

    if (!(dbus_message_get_args(m, error, DBUS_TYPE_STRING, &n, DBUS_TYPE_INVALID)) || !n) {
        return dbus_parsing_error("Error parsing Server::GetAlternativeHostName message", error);
    }

    t = avahi_alternative_host_name(n);
    avahi_dbus_respond_string(c, m, t);
    avahi_free(t);

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult dbus_get_alternative_service_name(DBusConnection *c, DBusMessage *m, DBusError *error) {
    char *n, *t;

    if (!(dbus_message_get_args(m, error, DBUS_TYPE_STRING, &n, DBUS_TYPE_INVALID)) || !n) {
        return dbus_parsing_error("Error parsing Server::GetAlternativeServiceName message", error);
    }

    t = avahi_alternative_service_name(n);
    avahi_dbus_respond_string(c, m, t);
    avahi_free(t);

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult dbus_create_new_entry_group(DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    EntryGroupInfo *i;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_entry_group_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };

    if (!dbus_message_get_args(m, error, DBUS_TYPE_INVALID)) {
        return dbus_parsing_error("Error parsing Server::EntryGroupNew message", error);
    }

    if (server->disable_user_service_publishing)
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_NOT_PERMITTED, NULL);

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    i = avahi_new(EntryGroupInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->n_entries = 0;
    AVAHI_LLIST_PREPEND(EntryGroupInfo, entry_groups, client->entry_groups, i);
    client->n_objects++;

    if (!(i->entry_group = avahi_s_entry_group_new(avahi_server, avahi_dbus_entry_group_callback, i))) {
        avahi_dbus_entry_group_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    i->path = avahi_strdup_printf("/Client%u/EntryGroup%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    return avahi_dbus_respond_path(c, m, i->path);
}

static DBusHandlerResult dbus_prepare_domain_browser_object(DomainBrowserInfo **dbi, DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    DomainBrowserInfo *i;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_domain_browser_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };
    int32_t interface, protocol, type;
    uint32_t flags;
    char *domain;

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_INT32, &type,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || type < 0 || type >= AVAHI_DOMAIN_BROWSER_MAX) {
        return dbus_parsing_error("Error parsing Server::DomainBrowserNew message", error);
    }

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    if (!*domain)
        domain = NULL;

    i = avahi_new(DomainBrowserInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->delay_timeout = NULL;
    AVAHI_LLIST_PREPEND(DomainBrowserInfo, domain_browsers, client->domain_browsers, i);
    client->n_objects++;

    if (!(i->domain_browser = avahi_s_domain_browser_prepare(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, domain, (AvahiDomainBrowserType) type, (AvahiLookupFlags) flags, avahi_dbus_domain_browser_callback, i))) {
        avahi_dbus_domain_browser_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    i->path = avahi_strdup_printf("/Client%u/DomainBrowser%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    *dbi = i;
    return avahi_dbus_respond_path(c, m, i->path);
}


static DBusHandlerResult dbus_prepare_service_type_browser_object(ServiceTypeBrowserInfo **stbi, DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    ServiceTypeBrowserInfo *i;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_service_type_browser_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };
    int32_t interface, protocol;
    uint32_t flags;
    char *domain;

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID)) {
        return dbus_parsing_error("Error parsing Server::ServiceTypeBrowserNew message", error);
    }

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    if (!*domain)
        domain = NULL;

    i = avahi_new(ServiceTypeBrowserInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->delay_timeout = NULL;
    AVAHI_LLIST_PREPEND(ServiceTypeBrowserInfo, service_type_browsers, client->service_type_browsers, i);
    client->n_objects++;

    if (!(i->service_type_browser = avahi_s_service_type_browser_prepare(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, domain, (AvahiLookupFlags) flags, avahi_dbus_service_type_browser_callback, i))) {
        avahi_dbus_service_type_browser_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    i->path = avahi_strdup_printf("/Client%u/ServiceTypeBrowser%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    *stbi = i;
    return avahi_dbus_respond_path(c, m, i->path);
}


static DBusHandlerResult dbus_prepare_service_browser_object(ServiceBrowserInfo **sbi, DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    ServiceBrowserInfo *i;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_service_browser_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };
    int32_t interface, protocol;
    uint32_t flags;
    char *domain, *type;

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !type) {
        return dbus_parsing_error("Error parsing Server::ServiceBrowserNew message", error);
    }

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    if (!*domain)
        domain = NULL;

    i = avahi_new(ServiceBrowserInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->delay_timeout = NULL;
    AVAHI_LLIST_PREPEND(ServiceBrowserInfo, service_browsers, client->service_browsers, i);
    client->n_objects++;

    if (!(i->service_browser = avahi_s_service_browser_prepare(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, type, domain, (AvahiLookupFlags) flags, avahi_dbus_service_browser_callback, i))) {
        avahi_dbus_service_browser_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    i->path = avahi_strdup_printf("/Client%u/ServiceBrowser%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    *sbi = i;
    return avahi_dbus_respond_path(c, m, i->path);
}

static DBusHandlerResult dbus_create_sync_service_resolver_object(DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    int32_t interface, protocol, aprotocol;
    uint32_t flags;
    char *name, *type, *domain;
    SyncServiceResolverInfo *i;

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_INT32, &aprotocol,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !type) {
        return dbus_parsing_error("Error parsing Server::ResolveService message", error);
    }

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    if (!*domain)
        domain = NULL;

    if (!*name)
        name = NULL;

    i = avahi_new(SyncServiceResolverInfo, 1);
    i->client = client;
    i->message = dbus_message_ref(m);
    AVAHI_LLIST_PREPEND(SyncServiceResolverInfo, sync_service_resolvers, client->sync_service_resolvers, i);
    client->n_objects++;

    if (!(i->service_resolver = avahi_s_service_resolver_new(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, name, type, domain, (AvahiProtocol) aprotocol, (AvahiLookupFlags) flags, avahi_dbus_sync_service_resolver_callback, i))) {
        avahi_dbus_sync_service_resolver_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult dbus_prepare_async_service_resolver_object(AsyncServiceResolverInfo **sri, DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    int32_t interface, protocol, aprotocol;
    uint32_t flags;
    char *name, *type, *domain;
    AsyncServiceResolverInfo *i;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_async_service_resolver_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_INT32, &aprotocol,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !type) {
        return dbus_parsing_error("Error parsing Server::ServiceResolverNew message", error);
    }

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn(__FILE__": Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn(__FILE__": Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    if (!*domain)
        domain = NULL;

    if (!*name)
        name = NULL;

    i = avahi_new(AsyncServiceResolverInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->delay_timeout = NULL;
    AVAHI_LLIST_PREPEND(AsyncServiceResolverInfo, async_service_resolvers, client->async_service_resolvers, i);
    client->n_objects++;

    if (!(i->service_resolver = avahi_s_service_resolver_prepare(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, name, type, domain, (AvahiProtocol) aprotocol, (AvahiLookupFlags) flags, avahi_dbus_async_service_resolver_callback, i))) {
        avahi_dbus_async_service_resolver_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

/* avahi_log_debug(__FILE__": [%s], new service resolver for <%s.%s.%s>", i->path, name, type, domain); */

    i->path = avahi_strdup_printf("/Client%u/ServiceResolver%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    *sri = i;
    return avahi_dbus_respond_path(c, m, i->path);
}

static DBusHandlerResult dbus_create_sync_host_name_resolver_object(DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    int32_t interface, protocol, aprotocol;
    uint32_t flags;
    char *name;
    SyncHostNameResolverInfo *i;

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_INT32, &aprotocol,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !name) {
        return dbus_parsing_error("Error parsing Server::ResolveHostName message", error);
    }

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    i = avahi_new(SyncHostNameResolverInfo, 1);
    i->client = client;
    i->message = dbus_message_ref(m);
    AVAHI_LLIST_PREPEND(SyncHostNameResolverInfo, sync_host_name_resolvers, client->sync_host_name_resolvers, i);
    client->n_objects++;

    if (!(i->host_name_resolver = avahi_s_host_name_resolver_new(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, name, (AvahiProtocol) aprotocol, (AvahiLookupFlags) flags, avahi_dbus_sync_host_name_resolver_callback, i))) {
        avahi_dbus_sync_host_name_resolver_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult dbus_prepare_async_host_name_resolver_object(AsyncHostNameResolverInfo **hri, DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    int32_t interface, protocol, aprotocol;
    uint32_t flags;
    char *name;
    AsyncHostNameResolverInfo *i;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_async_host_name_resolver_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_INT32, &aprotocol,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !name) {
        return dbus_parsing_error("Error parsing Server::HostNameResolverNew message", error);
    }

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn(__FILE__": Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn(__FILE__": Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    i = avahi_new(AsyncHostNameResolverInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->delay_timeout = NULL;
    AVAHI_LLIST_PREPEND(AsyncHostNameResolverInfo, async_host_name_resolvers, client->async_host_name_resolvers, i);
    client->n_objects++;

    if (!(i->host_name_resolver = avahi_s_host_name_resolver_prepare(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, name, aprotocol, (AvahiLookupFlags) flags, avahi_dbus_async_host_name_resolver_callback, i))) {
        avahi_dbus_async_host_name_resolver_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    i->path = avahi_strdup_printf("/Client%u/HostNameResolver%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    *hri = i;
    return avahi_dbus_respond_path(c, m, i->path);
}

static DBusHandlerResult dbus_create_sync_address_resolver_object(DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    int32_t interface, protocol;
    uint32_t flags;
    char *address;
    SyncAddressResolverInfo *i;
    AvahiAddress a;

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &address,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !address) {
        return dbus_parsing_error("Error parsing Server::ResolveAddress message", error);
    }

    if (!avahi_address_parse(address, AVAHI_PROTO_UNSPEC, &a))
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_INVALID_ADDRESS, NULL);

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    i = avahi_new(SyncAddressResolverInfo, 1);
    i->client = client;
    i->message = dbus_message_ref(m);
    AVAHI_LLIST_PREPEND(SyncAddressResolverInfo, sync_address_resolvers, client->sync_address_resolvers, i);
    client->n_objects++;

    if (!(i->address_resolver = avahi_s_address_resolver_new(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, &a, (AvahiLookupFlags) flags, avahi_dbus_sync_address_resolver_callback, i))) {
        avahi_dbus_sync_address_resolver_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult dbus_prepare_async_address_resolver_object(AsyncAddressResolverInfo **ari, DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    int32_t interface, protocol;
    uint32_t flags;
    char *address;
    AsyncAddressResolverInfo *i;
    AvahiAddress a;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_async_address_resolver_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &address,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !address) {
        return dbus_parsing_error("Error parsing Server::AddressResolverNew message", error);
    }

    if (!avahi_address_parse(address, AVAHI_PROTO_UNSPEC, &a))
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_INVALID_ADDRESS, NULL);

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn(__FILE__": Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn(__FILE__": Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    i = avahi_new(AsyncAddressResolverInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->delay_timeout = NULL;
    AVAHI_LLIST_PREPEND(AsyncAddressResolverInfo, async_address_resolvers, client->async_address_resolvers, i);
    client->n_objects++;

    if (!(i->address_resolver = avahi_s_address_resolver_prepare(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, &a, (AvahiLookupFlags) flags, avahi_dbus_async_address_resolver_callback, i))) {
        avahi_dbus_async_address_resolver_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    i->path = avahi_strdup_printf("/Client%u/AddressResolver%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    *ari = i;
    return avahi_dbus_respond_path(c, m, i->path);
}

static DBusHandlerResult dbus_prepare_record_browser_object(RecordBrowserInfo **rbi, DBusConnection *c, DBusMessage *m, DBusError *error) {
    Client *client;
    RecordBrowserInfo *i;
    static const DBusObjectPathVTable vtable = {
        NULL,
        avahi_dbus_msg_record_browser_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };
    int32_t interface, protocol;
    uint32_t flags;
    char *name;
    uint16_t type, clazz;
    AvahiKey *key;

    if (!dbus_message_get_args(
            m, error,
            DBUS_TYPE_INT32, &interface,
            DBUS_TYPE_INT32, &protocol,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_UINT16, &clazz,
            DBUS_TYPE_UINT16, &type,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID) || !name) {
        return dbus_parsing_error("Error parsing Server::RecordBrowserNew message", error);
    }

    if (!avahi_is_valid_domain_name(name))
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_INVALID_DOMAIN_NAME, NULL);

    if (!(client = client_get(dbus_message_get_sender(m), TRUE))) {
        avahi_log_warn("Too many clients, client request failed.");
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_CLIENTS, NULL);
    }

    if (client->n_objects >= server->n_objects_per_client_max) {
        avahi_log_warn("Too many objects for client '%s', client request failed.", client->name);
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_OBJECTS, NULL);
    }

    i = avahi_new(RecordBrowserInfo, 1);
    i->id = ++client->current_id;
    i->client = client;
    i->path = NULL;
    i->delay_timeout = NULL;
    AVAHI_LLIST_PREPEND(RecordBrowserInfo, record_browsers, client->record_browsers, i);
    client->n_objects++;

    key = avahi_key_new(name, clazz, type);
    assert(key);

    if (!(i->record_browser = avahi_s_record_browser_prepare(avahi_server, (AvahiIfIndex) interface, (AvahiProtocol) protocol, key, (AvahiLookupFlags) flags, avahi_dbus_record_browser_callback, i))) {
        avahi_key_unref(key);
        avahi_dbus_record_browser_free(i);
        return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
    }

    avahi_key_unref(key);

    i->path = avahi_strdup_printf("/Client%u/RecordBrowser%u", client->id, i->id);
    dbus_connection_register_object_path(c, i->path, &vtable, i);
    *rbi = i;
    return avahi_dbus_respond_path(c, m, i->path);
}

static DBusHandlerResult dbus_select_common_methods(DBusConnection *c, DBusMessage *m, AVAHI_GCC_UNUSED void *userdata, const char *iface, DBusError *error) {
    if (dbus_message_is_method_call(m, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
        return avahi_dbus_handle_introspect(c, m, "org.freedesktop.Avahi.Server.xml");

    else if (dbus_message_is_method_call(m, iface, "GetHostName")) {
        return dbus_get_host_name(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "SetHostName")) {
        return dbus_set_host_name(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetDomainName")) {
        return dbus_get_domain_name(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetHostNameFqdn")) {
        return dbus_get_host_name_fqdn(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "IsNSSSupportAvailable")) {
        return dbus_is_ns_support_available(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetVersionString")) {
        return dbus_get_version_string(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetAPIVersion")) {
        return dbus_get_api_version(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetState")) {
        return dbus_get_state(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetLocalServiceCookie")) {
        return dbus_get_local_service_cookie(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetNetworkInterfaceNameByIndex")) {
        return dbus_get_net_if_by_index(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetNetworkInterfaceIndexByName")) {
        return dbus_get_net_if_by_name(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetAlternativeHostName")) {
        return dbus_get_alternative_host_name(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "GetAlternativeServiceName")) {
        return dbus_get_alternative_service_name(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "EntryGroupNew")) {
        return dbus_create_new_entry_group(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "ResolveHostName")) {
        return dbus_create_sync_host_name_resolver_object(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "ResolveAddress")) {
        return dbus_create_sync_address_resolver_object(c, m, error);

    } else if (dbus_message_is_method_call(m, iface, "ResolveService")) {
        return dbus_create_sync_service_resolver_object(c, m, error);
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

CREATE_DBUS_DELAY_FUNC(DomainBrowserInfo, domain_browser, avahi_s_domain_browser_start)
CREATE_DBUS_DELAY_FUNC(ServiceTypeBrowserInfo, service_type_browser, avahi_s_service_type_browser_start)
CREATE_DBUS_DELAY_FUNC(ServiceBrowserInfo, service_browser, avahi_s_service_browser_start)
CREATE_DBUS_DELAY_FUNC(AsyncServiceResolverInfo, service_resolver, avahi_s_service_resolver_start)
CREATE_DBUS_DELAY_FUNC(AsyncHostNameResolverInfo, host_name_resolver, avahi_s_host_name_resolver_start)
CREATE_DBUS_DELAY_FUNC(AsyncAddressResolverInfo, address_resolver, avahi_s_address_resolver_start)
CREATE_DBUS_DELAY_FUNC(RecordBrowserInfo, record_browser, avahi_s_record_browser_start_query)

static DBusHandlerResult dbus_select_browser(DBusConnection *c, DBusMessage *m, AVAHI_GCC_UNUSED void *userdata, const char *iface, DBusError *error) {
    DBusHandlerResult r;
    const AvahiPoll *poll_api = NULL;
    struct timeval tv;

    poll_api = avahi_simple_poll_get(simple_poll_api);
    avahi_elapse_time(&tv, DEFAULT_START_DELAY_MS, 0);

    if (dbus_message_is_method_call(m, iface, "DomainBrowserNew")) {
        DomainBrowserInfo *db = NULL;
        r = dbus_prepare_domain_browser_object(&db, c, m, error);
        if (db)
            db->delay_timeout = poll_api->timeout_new(poll_api, &tv, GET_DBUS_DELAY_FUNC(DomainBrowserInfo, domain_browser), db);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "ServiceTypeBrowserNew")) {
        ServiceTypeBrowserInfo *stbi = NULL;
        r = dbus_prepare_service_type_browser_object(&stbi, c, m, error);
        if (stbi)
            stbi->delay_timeout = poll_api->timeout_new(poll_api, &tv, GET_DBUS_DELAY_FUNC(ServiceTypeBrowserInfo, service_type_browser), stbi);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "ServiceBrowserNew")) {
        ServiceBrowserInfo *sbi = NULL;
        r = dbus_prepare_service_browser_object(&sbi, c, m, error);
        if (sbi)
            sbi->delay_timeout = poll_api->timeout_new(poll_api, &tv, GET_DBUS_DELAY_FUNC(ServiceBrowserInfo, service_browser), sbi);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "ServiceResolverNew")) {
        AsyncServiceResolverInfo *sri = NULL;
        r = dbus_prepare_async_service_resolver_object(&sri, c, m, error);
        if (sri)
            sri->delay_timeout = poll_api->timeout_new(poll_api, &tv, GET_DBUS_DELAY_FUNC(AsyncServiceResolverInfo, service_resolver), sri);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "HostNameResolverNew")) {
        AsyncHostNameResolverInfo *hri = NULL;
        r = dbus_prepare_async_host_name_resolver_object(&hri, c, m, error);
        if (hri)
            hri->delay_timeout = poll_api->timeout_new(poll_api, &tv, GET_DBUS_DELAY_FUNC(AsyncHostNameResolverInfo, host_name_resolver), hri);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "AddressResolverNew")) {
        AsyncAddressResolverInfo *ari = NULL;
        r = dbus_prepare_async_address_resolver_object(&ari, c, m, error);
        if (ari)
            ari->delay_timeout = poll_api->timeout_new(poll_api, &tv, GET_DBUS_DELAY_FUNC(AsyncAddressResolverInfo, address_resolver), ari);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "RecordBrowserNew")) {
        RecordBrowserInfo *rbi = NULL;
        r = dbus_prepare_record_browser_object(&rbi, c, m, error);
        if (rbi)
            rbi->delay_timeout = poll_api->timeout_new(poll_api, &tv, GET_DBUS_DELAY_FUNC(RecordBrowserInfo, record_browser), rbi);
        return r;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult dbus_select_prepare_browser(DBusConnection *c, DBusMessage *m, AVAHI_GCC_UNUSED void *userdata, const char *iface, DBusError *error) {
    DBusHandlerResult r;

    if (dbus_message_is_method_call(m, iface, "DomainBrowserPrepare")) {
        DomainBrowserInfo *db = NULL;
        r = dbus_prepare_domain_browser_object(&db, c, m, error);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "ServiceTypeBrowserPrepare")) {
        ServiceTypeBrowserInfo *stbi = NULL;
        r = dbus_prepare_service_type_browser_object(&stbi, c, m, error);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "ServiceBrowserPrepare")) {
        ServiceBrowserInfo *sbi = NULL;
        r = dbus_prepare_service_browser_object(&sbi, c, m, error);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "ServiceResolverPrepare")) {
        AsyncServiceResolverInfo *sri = NULL;
        r = dbus_prepare_async_service_resolver_object(&sri, c, m, error);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "HostNameResolverPrepare")) {
        AsyncHostNameResolverInfo *hri = NULL;
        r = dbus_prepare_async_host_name_resolver_object(&hri, c, m, error);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "AddressResolverPrepare")) {
        AsyncAddressResolverInfo *ari = NULL;
        r = dbus_prepare_async_address_resolver_object(&ari, c, m, error);
        return r;

    } else if (dbus_message_is_method_call(m, iface, "RecordBrowserPrepare")) {
        RecordBrowserInfo *rbi = NULL;
        r = dbus_prepare_record_browser_object(&rbi, c, m, error);
        return r;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult msg_server_impl(DBusConnection *c, DBusMessage *m, AVAHI_GCC_UNUSED void *userdata) {
    DBusHandlerResult r;
    DBusError error;

    dbus_error_init(&error);

    avahi_log_debug(__FILE__": interface=%s, path=%s, member=%s",
                    dbus_message_get_interface(m),
                    dbus_message_get_path(m),
                    dbus_message_get_member(m));

    r = dbus_select_common_methods(c,m,userdata, AVAHI_DBUS_INTERFACE_SERVER, &error);
    if( r != DBUS_HANDLER_RESULT_NOT_YET_HANDLED)
        return r;

    r = dbus_select_browser(c,m,userdata, AVAHI_DBUS_INTERFACE_SERVER, &error);
    if( r != DBUS_HANDLER_RESULT_NOT_YET_HANDLED)
        return r;

    r = dbus_select_common_methods(c,m,userdata, AVAHI_DBUS_INTERFACE_SERVER2, &error);
    if( r != DBUS_HANDLER_RESULT_NOT_YET_HANDLED)
        return r;

    r = dbus_select_browser(c,m,userdata, AVAHI_DBUS_INTERFACE_SERVER2, &error);
    if( r != DBUS_HANDLER_RESULT_NOT_YET_HANDLED)
        return r;

    r = dbus_select_prepare_browser(c,m,userdata, AVAHI_DBUS_INTERFACE_SERVER2, &error);
    if( r != DBUS_HANDLER_RESULT_NOT_YET_HANDLED)
        return r;


    avahi_log_warn("Missed message %s::%s()", dbus_message_get_interface(m), dbus_message_get_member(m));
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void dbus_protocol_server_state_changed(AvahiServerState state) {
    DBusMessage *m;
    int32_t t;
    const char *e;

    if (!server || !server->bus)
        return;

    m = dbus_message_new_signal(AVAHI_DBUS_PATH_SERVER, AVAHI_DBUS_INTERFACE_SERVER, "StateChanged");

    if (!m) {
        avahi_log_error("Failed allocate message");
        return;
    }

    t = (int32_t) state;

    if (state == AVAHI_SERVER_COLLISION)
        e = AVAHI_DBUS_ERR_COLLISION;
    else if (state == AVAHI_SERVER_FAILURE)
        e = avahi_error_number_to_dbus(avahi_server_errno(avahi_server));
    else
        e = AVAHI_DBUS_ERR_OK;

    dbus_message_append_args(m, DBUS_TYPE_INT32, &t, DBUS_TYPE_STRING, &e, DBUS_TYPE_INVALID);
    dbus_connection_send(server->bus, m, NULL);
    dbus_message_unref(m);
}

static int dbus_connect(void) {
    DBusError error;

    static const DBusObjectPathVTable server_vtable = {
        NULL,
        msg_server_impl,
        NULL,
        NULL,
        NULL,
        NULL
    };

    assert(server);
    assert(!server->bus);

    dbus_error_init(&error);

#ifdef HAVE_DBUS_BUS_GET_PRIVATE
    if (!(server->bus = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error))) {
        assert(dbus_error_is_set(&error));
        avahi_log_error("dbus_bus_get_private(): %s", error.message);
        goto fail;
    }
#else
    {
        const char *a;

        if (!(a = getenv("DBUS_SYSTEM_BUS_ADDRESS")) || !*a)
            a = DBUS_SYSTEM_BUS_DEFAULT_ADDRESS;

        if (!(server->bus = dbus_connection_open_private(a, &error))) {
            assert(dbus_error_is_set(&error));
            avahi_log_error("dbus_bus_open_private(): %s", error.message);
            goto fail;
        }

        if (!dbus_bus_register(server->bus, &error)) {
            assert(dbus_error_is_set(&error));
            avahi_log_error("dbus_bus_register(): %s", error.message);
            goto fail;
        }
    }
#endif

    if (avahi_dbus_connection_glue(server->bus, server->poll_api) < 0) {
        avahi_log_error("avahi_dbus_connection_glue() failed");
        goto fail;
    }

    dbus_connection_set_exit_on_disconnect(server->bus, FALSE);

    if (dbus_bus_request_name(
            server->bus,
            AVAHI_DBUS_NAME,
#if (DBUS_VERSION_MAJOR == 0) && (DBUS_VERSION_MINOR < 60)
            DBUS_NAME_FLAG_PROHIBIT_REPLACEMENT,
#else
            DBUS_NAME_FLAG_DO_NOT_QUEUE,
#endif
            &error) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        if (dbus_error_is_set(&error)) {
            avahi_log_error("dbus_bus_request_name(): %s", error.message);
            goto fail;
        }

        avahi_log_error("Failed to acquire D-Bus name '"AVAHI_DBUS_NAME"'");
        goto fail;
    }

    if (!(dbus_connection_add_filter(server->bus, msg_signal_filter_impl, (void*) server->poll_api, NULL))) {
        avahi_log_error("dbus_connection_add_filter() failed");
        goto fail;
    }

    dbus_bus_add_match(server->bus, "type='signal',""interface='" DBUS_INTERFACE_DBUS  "'", &error);

    if (dbus_error_is_set(&error)) {
        avahi_log_error("dbus_bus_add_match(): %s", error.message);
        goto fail;
    }

    if (!(dbus_connection_register_object_path(server->bus, AVAHI_DBUS_PATH_SERVER, &server_vtable, NULL))) {
        avahi_log_error("dbus_connection_register_object_path() failed");
        goto fail;
    }

    return 0;
fail:

    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    if (server->bus) {
#ifdef HAVE_DBUS_CONNECTION_CLOSE
        dbus_connection_close(server->bus);
#else
        dbus_connection_disconnect(server->bus);
#endif
        dbus_connection_unref(server->bus);
        server->bus = NULL;
    }

    return -1;
}

static void dbus_disconnect(void) {
    assert(server);

    while (server->clients)
        client_free(server->clients);

    assert(server->n_clients == 0);

    if (server->bus) {
#ifdef HAVE_DBUS_CONNECTION_CLOSE
        dbus_connection_close(server->bus);
#else
        dbus_connection_disconnect(server->bus);
#endif
        dbus_connection_unref(server->bus);
        server->bus = NULL;
    }
}

int dbus_protocol_setup(const AvahiPoll *poll_api,
                        int _disable_user_service_publishing,
                        int _n_clients_max,
                        int _n_objects_per_client_max,
                        int _n_entries_per_entry_group_max,
                        int force) {


    server = avahi_new(Server, 1);
    AVAHI_LLIST_HEAD_INIT(Clients, server->clients);
    server->current_id = 0;
    server->n_clients = 0;
    server->bus = NULL;
    server->poll_api = poll_api;
    server->reconnect_timeout = NULL;
    server->reconnect = force;
    server->disable_user_service_publishing = _disable_user_service_publishing;
    server->n_clients_max = _n_clients_max > 0 ? _n_clients_max : DEFAULT_CLIENTS_MAX;
    server->n_objects_per_client_max = _n_objects_per_client_max > 0 ? _n_objects_per_client_max : DEFAULT_OBJECTS_PER_CLIENT_MAX;
    server->n_entries_per_entry_group_max = _n_entries_per_entry_group_max > 0 ? _n_entries_per_entry_group_max : DEFAULT_ENTRIES_PER_ENTRY_GROUP_MAX;

    if (dbus_connect() < 0) {
        struct timeval tv;

        if (!force)
            goto fail;

        avahi_log_warn("WARNING: Failed to contact D-Bus daemon, retrying in %ims.", RECONNECT_MSEC);

        avahi_elapse_time(&tv, RECONNECT_MSEC, 0);
        server->reconnect_timeout = server->poll_api->timeout_new(server->poll_api, &tv, reconnect_callback, NULL);
    }

    return 0;

fail:
    if (server->bus) {
#ifdef HAVE_DBUS_CONNECTION_CLOSE
        dbus_connection_close(server->bus);
#else
        dbus_connection_disconnect(server->bus);
#endif

        dbus_connection_unref(server->bus);
    }

    avahi_free(server);
    server = NULL;
    return -1;
}

void dbus_protocol_shutdown(void) {

    if (server) {
        dbus_disconnect();

        if (server->reconnect_timeout)
            server->poll_api->timeout_free(server->reconnect_timeout);

        avahi_free(server);
        server = NULL;
    }
}
