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
#include <avahi-common/domain.h>
#include <avahi-core/log.h>

#include "dbus-util.h"
#include "dbus-internal.h"
#include "main.h"

void avahi_dbus_entry_group_free(EntryGroupInfo *i) {
    assert(i);

    if (i->entry_group)
        avahi_s_entry_group_free(i->entry_group);

    if (i->path) {
        dbus_connection_unregister_object_path(server->bus, i->path);
        avahi_free(i->path);
    }
    AVAHI_LLIST_REMOVE(EntryGroupInfo, entry_groups, i->client->entry_groups, i);

    assert(i->client->n_objects >= 1);
    i->client->n_objects--;

    avahi_free(i);
}

void avahi_dbus_entry_group_callback(AvahiServer *s, AvahiSEntryGroup *g, AvahiEntryGroupState state, void* userdata) {
    EntryGroupInfo *i = userdata;
    DBusMessage *m;
    int32_t t;
    const char *e;

    assert(s);
    assert(g);
    assert(i);

    m = dbus_message_new_signal(i->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "StateChanged");

    if (!m) {
        avahi_log_error("Failed allocate message");
        return;
    }

    t = (int32_t) state;
    if (state == AVAHI_ENTRY_GROUP_FAILURE)
        e = avahi_error_number_to_dbus(avahi_server_errno(s));
    else if (state == AVAHI_ENTRY_GROUP_COLLISION)
        e = AVAHI_DBUS_ERR_COLLISION;
    else
        e = AVAHI_DBUS_ERR_OK;

    dbus_message_append_args(
        m,
        DBUS_TYPE_INT32, &t,
        DBUS_TYPE_STRING, &e,
        DBUS_TYPE_INVALID);
    dbus_message_set_destination(m, i->client->name);
    dbus_connection_send(server->bus, m, NULL);
    dbus_message_unref(m);
}

DBusHandlerResult avahi_dbus_msg_entry_group_impl(DBusConnection *c, DBusMessage *m, void *userdata) {
    DBusError error;
    EntryGroupInfo *i = userdata;

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
        return avahi_dbus_handle_introspect(c, m, "org.freedesktop.Avahi.EntryGroup.xml");

    /* Access control */
    if (strcmp(dbus_message_get_sender(m), i->client->name))
        return avahi_dbus_respond_error(c, m, AVAHI_ERR_ACCESS_DENIED, NULL);

    if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "Free")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing EntryGroup::Free message");
            goto fail;
        }

        avahi_dbus_entry_group_free(i);
        return avahi_dbus_respond_ok(c, m);

    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "Commit")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing EntryGroup::Commit message");
            goto fail;
        }

        if (avahi_s_entry_group_commit(i->entry_group) < 0)
            return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);

        return avahi_dbus_respond_ok(c, m);


    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "Reset")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing EntryGroup::Reset message");
            goto fail;
        }

        avahi_s_entry_group_reset(i->entry_group);
        i->n_entries = 0;
        return avahi_dbus_respond_ok(c, m);

    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "IsEmpty")) {

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing EntryGroup::IsEmpty message");
            goto fail;
        }

        return avahi_dbus_respond_boolean(c, m, !!avahi_s_entry_group_is_empty(i->entry_group));

    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "GetState")) {
        AvahiEntryGroupState state;

        if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
            avahi_log_warn("Error parsing EntryGroup::GetState message");
            goto fail;
        }

        state = avahi_s_entry_group_get_state(i->entry_group);
        return avahi_dbus_respond_int32(c, m, (int32_t) state);

    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddService")) {
        int32_t interface, protocol;
        uint32_t flags;
        char *type, *name, *domain, *host;
        uint16_t port;
        AvahiStringList *strlst = NULL;

        if (!dbus_message_get_args(
                m, &error,
                DBUS_TYPE_INT32, &interface,
                DBUS_TYPE_INT32, &protocol,
                DBUS_TYPE_UINT32, &flags,
                DBUS_TYPE_STRING, &name,
                DBUS_TYPE_STRING, &type,
                DBUS_TYPE_STRING, &domain,
                DBUS_TYPE_STRING, &host,
                DBUS_TYPE_UINT16, &port,
                DBUS_TYPE_INVALID) ||
            !type || !name ||
            avahi_dbus_read_strlst(m, 8, &strlst) < 0) {
            avahi_log_warn("Error parsing EntryGroup::AddService message");
            goto fail;
        }

        if (!(flags & AVAHI_PUBLISH_UPDATE) && i->n_entries >= server->n_entries_per_entry_group_max) {
            avahi_string_list_free(strlst);
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_ENTRIES, NULL);
        }

        if (domain && !*domain)
            domain = NULL;

        if (host && !*host)
            host = NULL;

        if (avahi_server_add_service_strlst(avahi_server, i->entry_group, (AvahiIfIndex) interface, (AvahiProtocol) protocol, (AvahiPublishFlags) flags, name, type, domain, host, port, strlst) < 0) {
            avahi_string_list_free(strlst);
            return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
        }

        if (!(flags & AVAHI_PUBLISH_UPDATE))
            i->n_entries ++;

        avahi_string_list_free(strlst);

        return avahi_dbus_respond_ok(c, m);

    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddServiceSubtype")) {

        int32_t interface, protocol;
        uint32_t flags;
        char *type, *name, *domain, *subtype;

        if (!dbus_message_get_args(
                m, &error,
                DBUS_TYPE_INT32, &interface,
                DBUS_TYPE_INT32, &protocol,
                DBUS_TYPE_UINT32, &flags,
                DBUS_TYPE_STRING, &name,
                DBUS_TYPE_STRING, &type,
                DBUS_TYPE_STRING, &domain,
                DBUS_TYPE_STRING, &subtype,
                DBUS_TYPE_INVALID) || !type || !name || !subtype) {
            avahi_log_warn("Error parsing EntryGroup::AddServiceSubtype message");
            goto fail;
        }

        if (!(flags & AVAHI_PUBLISH_UPDATE) && i->n_entries >= server->n_entries_per_entry_group_max)
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_ENTRIES, NULL);

        if (domain && !*domain)
            domain = NULL;

        if (avahi_server_add_service_subtype(avahi_server, i->entry_group, (AvahiIfIndex) interface, (AvahiProtocol) protocol, (AvahiPublishFlags) flags, name, type, domain, subtype) < 0)
            return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);

        if (!(flags & AVAHI_PUBLISH_UPDATE))
            i->n_entries ++;

        return avahi_dbus_respond_ok(c, m);

    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "UpdateServiceTxt")) {
        int32_t interface, protocol;
        uint32_t flags;
        char *type, *name, *domain;
        AvahiStringList *strlst;

        if (!dbus_message_get_args(
                m, &error,
                DBUS_TYPE_INT32, &interface,
                DBUS_TYPE_INT32, &protocol,
                DBUS_TYPE_UINT32, &flags,
                DBUS_TYPE_STRING, &name,
                DBUS_TYPE_STRING, &type,
                DBUS_TYPE_STRING, &domain,
                DBUS_TYPE_INVALID) ||
            !type || !name ||
            avahi_dbus_read_strlst(m, 6, &strlst)) {
            avahi_log_warn("Error parsing EntryGroup::UpdateServiceTxt message");
            goto fail;
        }

        if (domain && !*domain)
            domain = NULL;

        if (avahi_server_update_service_txt_strlst(avahi_server, i->entry_group, (AvahiIfIndex) interface, (AvahiProtocol) protocol, (AvahiPublishFlags) flags, name, type, domain, strlst) < 0) {
            avahi_string_list_free(strlst);
            return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
        }

        avahi_string_list_free(strlst);

        return avahi_dbus_respond_ok(c, m);

    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddAddress")) {
        int32_t interface, protocol;
        uint32_t flags;
        char *name, *address;
        AvahiAddress a;

        if (!dbus_message_get_args(
                m, &error,
                DBUS_TYPE_INT32, &interface,
                DBUS_TYPE_INT32, &protocol,
                DBUS_TYPE_UINT32, &flags,
                DBUS_TYPE_STRING, &name,
                DBUS_TYPE_STRING, &address,
                DBUS_TYPE_INVALID) || !name || !address) {
            avahi_log_warn("Error parsing EntryGroup::AddAddress message");
            goto fail;
        }

        if (!(flags & AVAHI_PUBLISH_UPDATE) && i->n_entries >= server->n_entries_per_entry_group_max)
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_ENTRIES, NULL);

        if (!(avahi_address_parse(address, AVAHI_PROTO_UNSPEC, &a)))
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_INVALID_ADDRESS, NULL);

        if (avahi_server_add_address(avahi_server, i->entry_group, (AvahiIfIndex) interface, (AvahiProtocol) protocol, (AvahiPublishFlags) flags, name, &a) < 0)
            return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);

        if (!(flags & AVAHI_PUBLISH_UPDATE))
            i->n_entries ++;

        return avahi_dbus_respond_ok(c, m);
    } else if (dbus_message_is_method_call(m, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddRecord")) {
        int32_t interface, protocol;
        uint32_t flags, ttl, size;
        uint16_t clazz, type;
        char *name;
        void *rdata;
        AvahiRecord *r;

        if (!dbus_message_get_args(
                m, &error,
                DBUS_TYPE_INT32, &interface,
                DBUS_TYPE_INT32, &protocol,
                DBUS_TYPE_UINT32, &flags,
                DBUS_TYPE_STRING, &name,
                DBUS_TYPE_UINT16, &clazz,
                DBUS_TYPE_UINT16, &type,
                DBUS_TYPE_UINT32, &ttl,
                DBUS_TYPE_INVALID) || !name ||
            avahi_dbus_read_rdata (m, 7, &rdata, &size)) {
            avahi_log_warn("Error parsing EntryGroup::AddRecord message");
            goto fail;
        }

        if (!(flags & AVAHI_PUBLISH_UPDATE) && i->n_entries >= server->n_entries_per_entry_group_max)
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_TOO_MANY_ENTRIES, NULL);

        if (!avahi_is_valid_domain_name (name))
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_INVALID_DOMAIN_NAME, NULL);

        if (!(r = avahi_record_new_full (name, clazz, type, ttl)))
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_NO_MEMORY, NULL);

        if (avahi_rdata_parse (r, rdata, size) < 0) {
            avahi_record_unref (r);
            return avahi_dbus_respond_error(c, m, AVAHI_ERR_INVALID_RDATA, NULL);
        }

        if (avahi_server_add(avahi_server, i->entry_group, (AvahiIfIndex) interface, (AvahiProtocol) protocol, (AvahiPublishFlags) flags, r) < 0) {
            avahi_record_unref (r);
            return avahi_dbus_respond_error(c, m, avahi_server_errno(avahi_server), NULL);
        }

        if (!(flags & AVAHI_PUBLISH_UPDATE))
            i->n_entries ++;

        avahi_record_unref (r);

        return avahi_dbus_respond_ok(c, m);
    }


    avahi_log_warn("Missed message %s::%s()", dbus_message_get_interface(m), dbus_message_get_member(m));

fail:
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
