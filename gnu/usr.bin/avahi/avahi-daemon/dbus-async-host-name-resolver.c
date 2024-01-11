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

#include <avahi-common/malloc.h>
#include <avahi-common/dbus.h>
#include <avahi-common/error.h>
#include <avahi-core/log.h>

#include "dbus-util.h"
#include "dbus-internal.h"
#include "main.h"

void avahi_dbus_async_host_name_resolver_free(AsyncHostNameResolverInfo *i) {
    const AvahiPoll *poll_api = NULL;

    assert(i);

    poll_api = avahi_simple_poll_get(simple_poll_api);

    if (i->delay_timeout)
        poll_api->timeout_free(i->delay_timeout);

    if (i->host_name_resolver)
        avahi_s_host_name_resolver_free(i->host_name_resolver);

    if (i->path) {
        dbus_connection_unregister_object_path(server->bus, i->path);
        avahi_free(i->path);
    }
    AVAHI_LLIST_REMOVE(AsyncHostNameResolverInfo, async_host_name_resolvers, i->client->async_host_name_resolvers, i);

    assert(i->client->n_objects >= 1);
    i->client->n_objects--;

    avahi_free(i);
}

void avahi_dbus_async_host_name_resolver_start(AsyncHostNameResolverInfo *i) {
    assert(i);

    if(i->host_name_resolver)
        avahi_s_host_name_resolver_start(i->host_name_resolver);
}

void avahi_dbus_async_host_name_resolver_callback(AvahiSHostNameResolver *r, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event, const char *host_name, const AvahiAddress *a, AvahiLookupResultFlags flags, void* userdata) {
    AsyncHostNameResolverInfo *i = userdata;
    DBusMessage *reply;

    assert(r);
    assert(i);

    reply = dbus_message_new_signal(i->path, AVAHI_DBUS_INTERFACE_HOST_NAME_RESOLVER, avahi_dbus_map_resolve_signal_name(event));

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return;
    }

    if (event == AVAHI_RESOLVER_FOUND) {
        char t[AVAHI_ADDRESS_STR_MAX], *pt = t;
        int32_t i_interface, i_protocol, i_aprotocol;
        uint32_t u_flags;

        assert(a);
        assert(host_name);
        avahi_address_snprint(t, sizeof(t), a);

        i_interface = (int32_t) interface;
        i_protocol = (int32_t) protocol;
        i_aprotocol = (int32_t) a->proto;
        u_flags = (uint32_t) flags;

        dbus_message_append_args(
            reply,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_STRING, &host_name,
            DBUS_TYPE_INT32, &i_aprotocol,
            DBUS_TYPE_STRING, &pt,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_INVALID);
    }  else {
        assert(event == AVAHI_RESOLVER_FAILURE);
        avahi_dbus_append_server_error(reply);
    }

    dbus_message_set_destination(reply, i->client->name);
    dbus_connection_send(server->bus, reply, NULL);
    dbus_message_unref(reply);
}

DBusHandlerResult avahi_dbus_msg_async_host_name_resolver_impl(DBusConnection *c, DBusMessage *m, void *userdata) {
    DBusError error;
    AsyncHostNameResolverInfo *i = userdata;

    assert(c);
    assert(m);
    assert(i);

    dbus_error_init(&error);

    avahi_log_debug(__FILE__": interface=%s, path=%s, member=%s",
                    dbus_message_get_interface(m),
                    dbus_message_get_path(m),
                    dbus_message_get_member(m));

    /* Introspection */
    if (dbus_message_is_method_call(m, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
        return avahi_dbus_handle_introspect(c, m, "org.freedesktop.Avahi.HostNameResolver.xml");

    /* Access control */
    if (strcmp(dbus_message_get_sender(m), i->client->name))
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_ACCESS_DENIED, NULL);

    if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_HOST_NAME_RESOLVER, "Free")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing HostNameResolver::Free message");
            goto fail;
        }

        avahi_dbus_async_host_name_resolver_free(i);
        return avahi_dbus_respond_ok(c, m);
    }

    if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_HOST_NAME_RESOLVER, "Start")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing HostNameResolver::Start message");
            goto fail;
        }

        avahi_dbus_async_host_name_resolver_start(i);
        return avahi_dbus_respond_ok(c, m);
    }


    avahi_log_warn("Missed message %s::%s()", dbus_message_get_interface(m), dbus_message_get_member(m));

fail:
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
