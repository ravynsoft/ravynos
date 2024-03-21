/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-marshal-recursive-util.c  Would be in dbus-marshal-recursive.c, but only used in bus/tests
 *
 * Copyright (C) 2004, 2005 Red Hat, Inc.
 *
 * Licensed under the Academic Free License version 2.1
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

#ifndef TEST_INTERNALS_DBUS_MARSHAL_RECURSIVE_UTIL_H
#define TEST_INTERNALS_DBUS_MARSHAL_RECURSIVE_UTIL_H

#include <dbus/dbus-string.h>
#include <dbus/dbus-types.h>

dbus_bool_t _dbus_marshal_recursive_test (const char *test_data_dir);
dbus_bool_t _dbus_test_generate_bodies   (int         sequence,
                                          int         byte_order,
                                          DBusString *signature,
                                          DBusString *body);

#endif
