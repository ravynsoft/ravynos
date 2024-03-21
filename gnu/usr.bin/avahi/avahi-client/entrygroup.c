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

void avahi_entry_group_set_state(AvahiEntryGroup *group, AvahiEntryGroupState state) {
    assert(group);

    if (group->state_valid && group->state == state)
        return;

    group->state = state;
    group->state_valid = 1;

    if (group->callback)
        group->callback(group, state, group->userdata);
}

static int retrieve_state(AvahiEntryGroup *group) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    int r = AVAHI_OK;
    int32_t state;
    AvahiClient *client;

    dbus_error_init(&error);

    assert(group);
    client = group->client;

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "GetState"))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block(client->bus, message, -1, &error)) ||
        dbus_error_is_set (&error)) {
        r = avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_INT32, &state, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error)) {
        r = avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return state;

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

AvahiEntryGroup* avahi_entry_group_new (AvahiClient *client, AvahiEntryGroupCallback callback, void *userdata) {
    AvahiEntryGroup *group = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    char *path;
    int state;

    assert(client);

    dbus_error_init (&error);

    if (!avahi_client_is_connected(client)) {
        avahi_client_set_errno(client, AVAHI_ERR_BAD_STATE);
        goto fail;
    }

    if (!(group = avahi_new(AvahiEntryGroup, 1))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    group->client = client;
    group->callback = callback;
    group->userdata = userdata;
    group->state_valid = 0;
    group->path = NULL;
    AVAHI_LLIST_PREPEND(AvahiEntryGroup, groups, client->groups, group);

    if (!(message = dbus_message_new_method_call(
              AVAHI_DBUS_NAME,
              AVAHI_DBUS_PATH_SERVER,
              AVAHI_DBUS_INTERFACE_SERVER,
              "EntryGroupNew"))) {
        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block (client->bus, message, -1, &error)) ||
        dbus_error_is_set (&error)) {
        avahi_client_set_errno (client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error)) {
        avahi_client_set_errno (client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!(group->path = avahi_strdup (path))) {

        /* FIXME: We don't remove the object on the server side */

        avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if ((state = retrieve_state(group)) < 0) {
        avahi_client_set_errno(client, state);
        goto fail;
    }

    avahi_entry_group_set_state(group, (AvahiEntryGroupState) state);

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return group;

fail:
    if (dbus_error_is_set(&error)) {
        avahi_client_set_dbus_error(client, &error);
        dbus_error_free(&error);
    }

    if (group)
        avahi_entry_group_free(group);

    if (message)
        dbus_message_unref(message);

    if (reply)
        dbus_message_unref(reply);

    return NULL;
}

static int entry_group_simple_method_call(AvahiEntryGroup *group, const char *method) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    int r = AVAHI_OK;
    AvahiClient *client;

    dbus_error_init(&error);

    assert(group);
    client = group->client;

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, method))) {
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

int avahi_entry_group_free(AvahiEntryGroup *group) {
    AvahiClient *client = group->client;
    int r = AVAHI_OK;

    assert(group);

    if (group->path && avahi_client_is_connected(client))
        r = entry_group_simple_method_call(group, "Free");

    AVAHI_LLIST_REMOVE(AvahiEntryGroup, groups, client->groups, group);

    avahi_free(group->path);
    avahi_free(group);

    return r;
}

int avahi_entry_group_commit(AvahiEntryGroup *group) {
    int ret;
    assert(group);

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    if ((ret = entry_group_simple_method_call(group, "Commit")) < 0)
        return ret;

    group->state_valid = 0;
    return ret;
}

int avahi_entry_group_reset(AvahiEntryGroup *group) {
    int ret;
    assert(group);

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    if ((ret = entry_group_simple_method_call(group, "Reset")) < 0)
        return ret;

    group->state_valid = 0;
    return ret;
}

int avahi_entry_group_get_state (AvahiEntryGroup *group) {
    assert (group);

    if (group->state_valid)
        return group->state;

    return retrieve_state(group);
}

AvahiClient* avahi_entry_group_get_client (AvahiEntryGroup *group) {
    assert(group);

    return group->client;
}

int avahi_entry_group_is_empty (AvahiEntryGroup *group) {
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    int r = AVAHI_OK;
    int b;
    AvahiClient *client;

    assert(group);
    client = group->client;

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call(AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "IsEmpty"))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    if (!(reply = dbus_connection_send_with_reply_and_block(client->bus, message, -1, &error)) ||
        dbus_error_is_set (&error)) {
        r = avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_BOOLEAN, &b, DBUS_TYPE_INVALID) ||
        dbus_error_is_set (&error)) {
        r = avahi_client_set_errno(client, AVAHI_ERR_DBUS_ERROR);
        goto fail;
    }

    dbus_message_unref(message);
    dbus_message_unref(reply);

    return !!b;

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

static int append_rdata(DBusMessage *message, const void *rdata, size_t size) {
    DBusMessageIter iter, sub;

    assert(message);

    dbus_message_iter_init_append(message, &iter);

    if (!(dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &sub)) ||
        !(dbus_message_iter_append_fixed_array(&sub, DBUS_TYPE_BYTE, &rdata, size)) ||
        !(dbus_message_iter_close_container(&iter, &sub)))
        return -1;

    return 0;
}

static int append_string_list(DBusMessage *message, AvahiStringList *txt) {
    DBusMessageIter iter, sub;
    int r = -1;
    AvahiStringList *p;

    assert(message);

    dbus_message_iter_init_append(message, &iter);

    /* Reverse the string list, so that we can pass it in-order to the server */
    txt = avahi_string_list_reverse(txt);

    if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "ay", &sub))
        goto fail;

    /* Assemble the AvahiStringList into an Array of Array of Bytes to send over dbus */
    for (p = txt; p != NULL; p = p->next) {
        DBusMessageIter sub2;
        const uint8_t *data = p->text;

        if (!(dbus_message_iter_open_container(&sub, DBUS_TYPE_ARRAY, "y", &sub2)) ||
            !(dbus_message_iter_append_fixed_array(&sub2, DBUS_TYPE_BYTE, &data, p->size)) ||
            !(dbus_message_iter_close_container(&sub, &sub2)))
            goto fail;
    }

    if (!dbus_message_iter_close_container(&iter, &sub))
        goto fail;

    r = 0;

fail:

    /* Reverse the string list to the original state */
    txt = avahi_string_list_reverse(txt);

    return r;
}

int avahi_entry_group_add_service_strlst(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *host,
    uint16_t port,
    AvahiStringList *txt) {

    DBusMessage *message = NULL, *reply = NULL;
    int r = AVAHI_OK;
    DBusError error;
    AvahiClient *client;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;

    assert(group);
    assert(name);
    assert(type);

    client = group->client;

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    if (!domain)
        domain = "";

    if (!host)
        host = "";

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddService"))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_STRING, &host,
            DBUS_TYPE_UINT16, &port,
            DBUS_TYPE_INVALID) ||
        append_string_list(message, txt) < 0) {
        r = avahi_client_set_errno(group->client, AVAHI_ERR_NO_MEMORY);
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

int avahi_entry_group_add_service(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *host,
    uint16_t port,
    ...) {

    va_list va;
    int r;
    AvahiStringList *txt;

    assert(group);

    va_start(va, port);
    txt = avahi_string_list_new_va(va);
    r = avahi_entry_group_add_service_strlst(group, interface, protocol, flags, name, type, domain, host, port, txt);
    avahi_string_list_free(txt);
    va_end(va);
    return r;
}

int avahi_entry_group_add_service_subtype(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    const char *subtype) {

    DBusMessage *message = NULL, *reply = NULL;
    int r = AVAHI_OK;
    DBusError error;
    AvahiClient *client;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;

    assert(group);
    assert(name);
    assert(type);
    assert(subtype);

    client = group->client;

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    if (!domain)
        domain = "";

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddServiceSubtype"))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_STRING, &subtype,
            DBUS_TYPE_INVALID)) {
        r = avahi_client_set_errno(group->client, AVAHI_ERR_NO_MEMORY);
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

int avahi_entry_group_update_service_txt(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    ...) {

    va_list va;
    int r;
    AvahiStringList *txt;

    va_start(va, domain);
    txt = avahi_string_list_new_va(va);
    r = avahi_entry_group_update_service_txt_strlst(group, interface, protocol, flags, name, type, domain, txt);
    avahi_string_list_free(txt);
    va_end(va);
    return r;
}

int avahi_entry_group_update_service_txt_strlst(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const char *type,
    const char *domain,
    AvahiStringList *txt) {

    DBusMessage *message = NULL, *reply = NULL;
    int r = AVAHI_OK;
    DBusError error;
    AvahiClient *client;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;

    assert(group);
    assert(name);
    assert(type);

    client = group->client;

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    if (!domain)
        domain = "";

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "UpdateServiceTxt"))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &type,
            DBUS_TYPE_STRING, &domain,
            DBUS_TYPE_INVALID) ||
        append_string_list(message, txt) < 0) {
        r = avahi_client_set_errno(group->client, AVAHI_ERR_NO_MEMORY);
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

/** Add a host/address pair */
int avahi_entry_group_add_address(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    const AvahiAddress *a) {

    DBusMessage *message = NULL, *reply = NULL;
    int r = AVAHI_OK;
    DBusError error;
    AvahiClient *client;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;
    char s_address[AVAHI_ADDRESS_STR_MAX];
    char *p_address = s_address;

    assert(name);

    client = group->client;

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddAddress"))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!avahi_address_snprint (s_address, sizeof (s_address), a))
    {
        r = avahi_client_set_errno(client, AVAHI_ERR_INVALID_ADDRESS);
        goto fail;
    }

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_STRING, &p_address,
            DBUS_TYPE_INVALID)) {
        r = avahi_client_set_errno(group->client, AVAHI_ERR_NO_MEMORY);
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

/** Add an arbitrary record */
int avahi_entry_group_add_record(
    AvahiEntryGroup *group,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiPublishFlags flags,
    const char *name,
    uint16_t clazz,
    uint16_t type,
    uint32_t ttl,
    const void *rdata,
    size_t size) {

    DBusMessage *message = NULL, *reply = NULL;
    int r = AVAHI_OK;
    DBusError error;
    AvahiClient *client;
    int32_t i_interface, i_protocol;
    uint32_t u_flags;

    assert(name);

    client = group->client;

    if (!group->path || !avahi_client_is_connected(group->client))
        return avahi_client_set_errno(group->client, AVAHI_ERR_BAD_STATE);

    dbus_error_init(&error);

    if (!(message = dbus_message_new_method_call (AVAHI_DBUS_NAME, group->path, AVAHI_DBUS_INTERFACE_ENTRY_GROUP, "AddRecord"))) {
        r = avahi_client_set_errno(client, AVAHI_ERR_NO_MEMORY);
        goto fail;
    }

    i_interface = (int32_t) interface;
    i_protocol = (int32_t) protocol;
    u_flags = (uint32_t) flags;

    if (!dbus_message_append_args(
            message,
            DBUS_TYPE_INT32, &i_interface,
            DBUS_TYPE_INT32, &i_protocol,
            DBUS_TYPE_UINT32, &u_flags,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_UINT16, &clazz,
            DBUS_TYPE_UINT16, &type,
            DBUS_TYPE_UINT32, &ttl,
            DBUS_TYPE_INVALID) || append_rdata(message, rdata, size) < 0) {
        r = avahi_client_set_errno(group->client, AVAHI_ERR_NO_MEMORY);
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
