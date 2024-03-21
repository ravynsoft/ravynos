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

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <avahi-common/error.h>
#include <avahi-common/dbus.h>
#include <avahi-common/malloc.h>
#include <avahi-core/log.h>
#include <avahi-core/core.h>

#ifdef ENABLE_CHROOT
#include "chroot.h"
#endif

#include "main.h"
#include "dbus-util.h"

DBusHandlerResult avahi_dbus_respond_error(DBusConnection *c, DBusMessage *m, int error, const char *text) {
    DBusMessage *reply;

    assert(-error > -AVAHI_OK);
    assert(-error < -AVAHI_ERR_MAX);

    if (!text)
        text = avahi_strerror(error);

    reply = dbus_message_new_error(m, avahi_error_number_to_dbus(error), text);

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_connection_send(c, reply, NULL);
    dbus_message_unref(reply);

    avahi_log_debug(__FILE__": Responding error '%s' (%i)", text, error);

    return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult avahi_dbus_respond_string(DBusConnection *c, DBusMessage *m, const char *text) {
    DBusMessage *reply;

    reply = dbus_message_new_method_return(m);

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_message_append_args(reply, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
    dbus_connection_send(c, reply, NULL);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult avahi_dbus_respond_int32(DBusConnection *c, DBusMessage *m, int32_t i) {
    DBusMessage *reply;

    reply = dbus_message_new_method_return(m);

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_message_append_args(reply, DBUS_TYPE_INT32, &i, DBUS_TYPE_INVALID);
    dbus_connection_send(c, reply, NULL);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult avahi_dbus_respond_uint32(DBusConnection *c, DBusMessage *m, uint32_t u) {
    DBusMessage *reply;

    reply = dbus_message_new_method_return(m);

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_message_append_args(reply, DBUS_TYPE_UINT32, &u, DBUS_TYPE_INVALID);
    dbus_connection_send(c, reply, NULL);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult avahi_dbus_respond_boolean(DBusConnection *c, DBusMessage *m, int b) {
    DBusMessage *reply;

    reply = dbus_message_new_method_return(m);

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &b, DBUS_TYPE_INVALID);
    dbus_connection_send(c, reply, NULL);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult avahi_dbus_respond_ok(DBusConnection *c, DBusMessage *m) {
    DBusMessage *reply;

    if (dbus_message_get_no_reply(m))
            return DBUS_HANDLER_RESULT_HANDLED;

    reply = dbus_message_new_method_return(m);

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_connection_send(c, reply, NULL);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult avahi_dbus_respond_path(DBusConnection *c, DBusMessage *m, const char *path) {
    DBusMessage *reply;

    reply = dbus_message_new_method_return(m);

    if (!reply) {
        avahi_log_error("Failed allocate message");
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_message_append_args(reply, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID);
    dbus_connection_send(c, reply, NULL);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

void avahi_dbus_append_server_error(DBusMessage *reply) {
    const char *t;

    t = avahi_error_number_to_dbus(avahi_server_errno(avahi_server));

    dbus_message_append_args(
        reply,
        DBUS_TYPE_STRING, &t,
        DBUS_TYPE_INVALID);
}

const char *avahi_dbus_map_browse_signal_name(AvahiBrowserEvent e) {
    switch (e) {
        case AVAHI_BROWSER_NEW : return "ItemNew";
        case AVAHI_BROWSER_REMOVE : return "ItemRemove";
        case AVAHI_BROWSER_FAILURE : return "Failure";
        case AVAHI_BROWSER_CACHE_EXHAUSTED : return "CacheExhausted";
        case AVAHI_BROWSER_ALL_FOR_NOW : return "AllForNow";
    }

    abort();
}

const char *avahi_dbus_map_resolve_signal_name(AvahiResolverEvent e) {
    switch (e) {
        case AVAHI_RESOLVER_FOUND : return "Found";
        case AVAHI_RESOLVER_FAILURE : return "Failure";
    }

    abort();
}

static char *file_get_contents(const char *fname) {
    int fd = -1;
    struct stat st;
    ssize_t size;
    char *buf = NULL;

    assert(fname);

#ifdef ENABLE_CHROOT
    fd = avahi_chroot_helper_get_fd(fname);
#else
    fd = open(fname, O_RDONLY);
#endif

    if (fd < 0) {
        avahi_log_error("Failed to open %s: %s", fname, strerror(errno));
        goto fail;
    }

    if (fstat(fd, &st) < 0) {
        avahi_log_error("stat(%s) failed: %s", fname, strerror(errno));
        goto fail;
    }

    if (!(S_ISREG(st.st_mode))) {
        avahi_log_error("Invalid file %s", fname);
        goto fail;
    }

    if (st.st_size > 1024*1024) { /** 1MB */
        avahi_log_error("File too large %s", fname);
        goto fail;
    }

    buf = avahi_new(char, st.st_size+1);

    if ((size = read(fd, buf, st.st_size)) < 0) {
        avahi_log_error("read() failed: %s\n", strerror(errno));
        goto fail;
    }

    buf[size] = 0;

    close(fd);

    return buf;

fail:
    if (fd >= 0)
        close(fd);

    if (buf)
        avahi_free(buf);

    return NULL;

}

DBusHandlerResult avahi_dbus_handle_introspect(DBusConnection *c, DBusMessage *m, const char *fname) {
    char *contents, *path;
    DBusError error;

    assert(c);
    assert(m);
    assert(fname);

    dbus_error_init(&error);

    if (!dbus_message_get_args(m, &error, DBUS_TYPE_INVALID)) {
        avahi_log_error("Error parsing Introspect message: %s", error.message);
        goto fail;
    }

    path = avahi_strdup_printf("%s/%s", AVAHI_DBUS_INTROSPECTION_DIR, fname);
    contents = file_get_contents(path);
    avahi_free(path);

    if (!contents) {
        avahi_log_error("Failed to load introspection data.");
        goto fail;
    }

    avahi_dbus_respond_string(c, m, contents);
    avahi_free(contents);

    return DBUS_HANDLER_RESULT_HANDLED;

fail:
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

}

void avahi_dbus_append_string_list(DBusMessage *reply, AvahiStringList *txt) {
    AvahiStringList *p;
    DBusMessageIter iter, sub;

    assert(reply);

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "ay", &sub);

    for (p = txt; p; p = p->next) {
        DBusMessageIter sub2;
        const uint8_t *data = p->text;

        dbus_message_iter_open_container(&sub, DBUS_TYPE_ARRAY, "y", &sub2);
        dbus_message_iter_append_fixed_array(&sub2, DBUS_TYPE_BYTE, &data, p->size);
        dbus_message_iter_close_container(&sub, &sub2);

    }
    dbus_message_iter_close_container(&iter, &sub);
}

int avahi_dbus_read_rdata(DBusMessage *m, int idx, void **rdata, uint32_t *size) {
    DBusMessageIter iter, sub;
    int n, j;
    uint8_t *k;

    assert(m);

    dbus_message_iter_init(m, &iter);

    for (j = 0; j < idx; j++)
       dbus_message_iter_next(&iter);

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_BYTE)
        goto fail;

    dbus_message_iter_recurse(&iter, &sub);
    dbus_message_iter_get_fixed_array(&sub, &k, &n);

    *rdata = k;
    *size = n;

    return 0;

fail:
    avahi_log_warn("Error parsing data");

    *rdata = NULL;
    size = 0;
    return -1;
}

int avahi_dbus_read_strlst(DBusMessage *m, int idx, AvahiStringList **l) {
    DBusMessageIter iter, sub;
    int j;
    AvahiStringList *strlst = NULL;

    assert(m);
    assert(l);

    dbus_message_iter_init(m, &iter);

    for (j = 0; j < idx; j++)
        dbus_message_iter_next(&iter);

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_ARRAY)
        goto fail;

    dbus_message_iter_recurse(&iter, &sub);

    for (;;) {
        int at, n;
        const uint8_t *k;
        DBusMessageIter sub2;

        if ((at = dbus_message_iter_get_arg_type(&sub)) == DBUS_TYPE_INVALID)
            break;

        assert(at == DBUS_TYPE_ARRAY);

        if (dbus_message_iter_get_element_type(&sub) != DBUS_TYPE_BYTE)
            goto fail;

        dbus_message_iter_recurse(&sub, &sub2);

        k = (const uint8_t*) "";
        n = 0;
        dbus_message_iter_get_fixed_array(&sub2, &k, &n);

        if (!k)
            k = (const uint8_t*) "";

        strlst = avahi_string_list_add_arbitrary(strlst, k, n);

        dbus_message_iter_next(&sub);
    }

    *l = strlst;

    return 0;

fail:
    avahi_log_warn("Error parsing TXT data");

    avahi_string_list_free(strlst);
    *l = NULL;
    return -1;
}

int avahi_dbus_is_our_own_service(Client *c, AvahiIfIndex interface, AvahiProtocol protocol, const char *name, const char *type, const char *domain) {
    AvahiSEntryGroup *g;

    if (avahi_server_get_group_of_service(avahi_server, interface, protocol, name, type, domain, &g) == AVAHI_OK) {
        EntryGroupInfo *egi;

        for (egi = c->entry_groups; egi; egi = egi->entry_groups_next)
            if (egi->entry_group == g)
                return 1;
    }

    return 0;
}

int avahi_dbus_append_rdata(DBusMessage *message, const void *rdata, size_t size) {
    DBusMessageIter iter, sub;

    assert(message);

    dbus_message_iter_init_append(message, &iter);

    if (!(dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &sub)) ||
        !(dbus_message_iter_append_fixed_array(&sub, DBUS_TYPE_BYTE, &rdata, size)) ||
        !(dbus_message_iter_close_container(&iter, &sub)))
        return -1;

    return 0;
}
