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

void avahi_dbus_sync_address_resolver_free(SyncAddressResolverInfo *i) {
    assert(i);

    if (i->address_resolver)
        avahi_s_address_resolver_free(i->address_resolver);
    dbus_message_unref(i->message);
    AVAHI_LLIST_REMOVE(SyncAddressResolverInfo, sync_address_resolvers, i->client->sync_address_resolvers, i);

    assert(i->client->n_objects >= 1);
    i->client->n_objects--;

    avahi_free(i);
}

void avahi_dbus_sync_address_resolver_callback(AvahiSAddressResolver *r, AvahiIfIndex interface, AvahiProtocol protocol, AvahiResolverEvent event, const AvahiAddress *address, const char *host_name, AvahiLookupResultFlags flags, void* userdata) {
    SyncAddressResolverInfo *i = userdata;

    assert(r);
    assert(address);
    assert(i);

    if (event == AVAHI_RESOLVER_FOUND) {
        char t[AVAHI_ADDRESS_STR_MAX], *pt = t;
        int32_t i_interface, i_protocol, i_aprotocol;
        uint32_t u_flags;
        DBusMessage *reply;

        assert(host_name);
        avahi_address_snprint(t, sizeof(t), address);

        i_interface = (int32_t) interface;
        i_protocol = (int32_t) protocol;
        i_aprotocol = (int32_t) address->proto;
        u_flags = (uint32_t) flags;

        reply = dbus_message_new_method_return(i->message);

        if (!reply) {
            avahi_log_error("Failed allocate message");
            goto finish;
        }

        dbus_message_append_args(
            reply,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_INT32, &i_aprotocol,
            DBUS_TYPE_STRING, &pt,
            DBUS_TYPE_STRING, &host_name,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_INVALID);

        dbus_connection_send(server->bus, reply, NULL);
        dbus_message_unref(reply);
    } else {
        assert(event == AVAHI_RESOLVER_FAILURE);
        avahi_dbus_respond_error(server->bus, i->message, avahi_server_errno(avahi_server), NULL);
    }

finish:
    avahi_dbus_sync_address_resolver_free(i);
}
