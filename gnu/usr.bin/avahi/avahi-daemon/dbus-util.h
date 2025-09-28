#ifndef foodbusutilhfoo
#define foodbusutilhfoo

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

#include <inttypes.h>

#include <dbus/dbus.h>

#include <avahi-common/strlst.h>
#include <avahi-common/defs.h>

#include "dbus-internal.h"

DBusHandlerResult avahi_dbus_respond_error(DBusConnection *c, DBusMessage *m, int error, const char *text);
DBusHandlerResult avahi_dbus_respond_string(DBusConnection *c, DBusMessage *m, const char *text);
DBusHandlerResult avahi_dbus_respond_int32(DBusConnection *c, DBusMessage *m, int32_t i);
DBusHandlerResult avahi_dbus_respond_uint32(DBusConnection *c, DBusMessage *m, uint32_t u);
DBusHandlerResult avahi_dbus_respond_boolean(DBusConnection *c, DBusMessage *m, int b);
DBusHandlerResult avahi_dbus_respond_ok(DBusConnection *c, DBusMessage *m);
DBusHandlerResult avahi_dbus_respond_path(DBusConnection *c, DBusMessage *m, const char *path);

void avahi_dbus_append_server_error(DBusMessage *reply);

const char *avahi_dbus_map_browse_signal_name(AvahiBrowserEvent e);

const char *avahi_dbus_map_resolve_signal_name(AvahiResolverEvent e);

DBusHandlerResult avahi_dbus_handle_introspect(DBusConnection *c, DBusMessage *m, const char *fname);

void avahi_dbus_append_string_list(DBusMessage *reply, AvahiStringList *txt);

int avahi_dbus_read_rdata(DBusMessage *m, int idx, void **rdata, uint32_t *size);
int avahi_dbus_read_strlst(DBusMessage *m, int idx, AvahiStringList **l);

int avahi_dbus_is_our_own_service(Client *c, AvahiIfIndex interface, AvahiProtocol protocol, const char *name, const char *type, const char *domain);

int avahi_dbus_append_rdata(DBusMessage *message, const void *rdata, size_t size);

#endif
