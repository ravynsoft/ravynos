/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2002-2009 Red Hat, Inc.
 * Copyright 2011-2018 Collabora Ltd.
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

#ifndef TEST_INTERNALS_MISC_INTERNALS_H
#define TEST_INTERNALS_MISC_INTERNALS_H

#include <dbus/dbus-types.h>

dbus_bool_t _dbus_address_test           (const char *test_data_dir);
dbus_bool_t _dbus_auth_test              (const char *test_data_dir);
dbus_bool_t _dbus_credentials_test       (const char *test_data_dir);
dbus_bool_t _dbus_marshal_byteswap_test  (const char *test_data_dir);
dbus_bool_t _dbus_marshal_validate_test  (const char *test_data_dir);
dbus_bool_t _dbus_mem_pool_test          (const char *test_data_dir);
dbus_bool_t _dbus_string_test            (const char *test_data_dir);
dbus_bool_t _dbus_sysdeps_test           (const char *test_data_dir);
dbus_bool_t _dbus_sha_test               (const char *test_data_dir);

#endif
