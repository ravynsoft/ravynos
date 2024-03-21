/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* tool-common - common functionality for dbus-test-tool modules
 *
 * Copyright © 2003 Philip Blundell <philb@gnu.org>
 * Copyright © 2011 Nokia Corporation
 * Copyright © 2014 Collabora Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef DBUS_TOOL_COMMON_H
#define DBUS_TOOL_COMMON_H

#include <dbus/dbus.h>

#if 0
#define VERBOSE fprintf
#else
#define VERBOSE(...) do {} while (0)
#endif

void tool_oom (const char *doing) _DBUS_GNUC_NORETURN;
dbus_bool_t tool_write_all (int fd, const void *buf, size_t size);
void tool_stderr_error (const char *context, DBusError *error);

#endif
