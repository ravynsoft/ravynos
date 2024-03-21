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

void avahi_dbus_service_type_browser_free(ServiceTypeBrowserInfo *i) {
    const AvahiPoll *poll_api = NULL;

    assert(i);

    poll_api = avahi_simple_poll_get(simple_poll_api);

    if (i->delay_timeout)
        poll_api->timeout_free(i->delay_timeout);

    if (i->service_type_browser)
        avahi_s_service_type_browser_free(i->service_type_browser);

    if (i->path) {
        dbus_connection_unregister_object_path(server->bus, i->path);
        avahi_free(i->path);
    }

    AVAHI_LLIST_REMOVE(ServiceTypeBrowserInfo, service_type_browsers, i->client->service_type_browsers, i);

    assert(i->client->n_objects >= 1);
    i->client->n_objects--;

    avahi_free(i);
}

void avahi_dbus_service_type_browser_start(ServiceTypeBrowserInfo *i) {
    assert(i);

    if(i->service_type_browser)
        avahi_s_service_type_browser_start(i->service_type_browser);
}

DBusHandlerResult avahi_dbus_msg_service_type_browser_impl(DBusConnection *c, DBusMessage *m, void *userdata) {
    DBusError error;
    ServiceTypeBrowserInfo *i = userdata;

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
        return avahi_dbus_handle_introspect(c, m, "org.freedesktop.Avahi.ServiceTypeBrowser.xml");

    /* Access control */
    if (strcmp(dbus_message_get_sender(m), i->client->name))
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_ACCESS_DENIED, NULL);

    if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "Free")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing ServiceTypeBrowser::Free message");
            goto fail;
        }

        avahi_dbus_service_type_browser_free(i);
        return avahi_dbus_respond_ok(c, m);

    }

    if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, "Start")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing ServiceTypeBrowser::Start message");
            goto fail;
        }

        avahi_dbus_service_type_browser_start(i);
        return avahi_dbus_respond_ok(c, m);

    }

    avahi_log_warn("Missed message %s::%s()", dbus_message_get_interface(m), dbus_message_get_member(m));

fail:
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void avahi_dbus_service_type_browser_callback(AvahiSServiceTypeBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol, AvahiBrowserEvent event, const char *type, const char *domain, AvahiLookupResultFlags flags, void* userdata) {
    ServiceTypeBrowserInfo *i = userdata;
    DBusMessage *m;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;

    assert(b);
    assert(i);

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    m = dbus_message_new_signal(i->path, AVAHI_DBUS_INTERFACE_SERVICE_TYPE_BROWSER, avahi_dbus_map_browse_signal_name(event));

    if (!m) {
        avahi_log_error("Failed allocate message");
        return;
    }

    if (event == AVAHI_BROWSER_NEW || event == AVAHI_BROWSER_REMOVE) {
        assert(type);
        assert(domain);
        dbus_message_append_args(
            m,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_INVALID);
    } else if (event == AVAHI_BROWSER_FAILURE)
        avahi_dbus_append_server_error(m);

    dbus_message_set_destination(m, i->client->name);
    dbus_connection_send(server->bus, m, NULL);
    dbus_message_unref(m);
}
