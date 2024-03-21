/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-marshal-byteswap-util.c  Would be in dbus-marshal-byteswap.c but tests/bus only
 *
 * Copyright (C) 2005 Red Hat, Inc.
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

#include <config.h>

#include "misc-internals.h"

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus/dbus-marshal-byteswap.h"
#include "dbus/dbus-test.h"
#include <dbus/dbus-test-tap.h>
#include <stdio.h>

#include "dbus-marshal-recursive-util.h"

static void
do_byteswap_test (int byte_order)
{
  int sequence;
  DBusString signature;
  DBusString body;
  int opposite_order;

  if (!_dbus_string_init (&signature) || !_dbus_string_init (&body))
    _dbus_test_fatal ("oom");

  opposite_order = byte_order == DBUS_LITTLE_ENDIAN ? DBUS_BIG_ENDIAN : DBUS_LITTLE_ENDIAN;

  sequence = 0;
  while (_dbus_test_generate_bodies (sequence, byte_order, &signature, &body))
    {
      DBusString copy;
      DBusTypeReader body_reader;
      DBusTypeReader copy_reader;

      if (!_dbus_string_init (&copy))
        _dbus_test_fatal ("oom");

      if (!_dbus_string_copy (&body, 0, &copy, 0))
        _dbus_test_fatal ("oom");

      _dbus_marshal_byteswap (&signature, 0,
                              byte_order,
                              opposite_order,
                              &copy, 0);

      _dbus_type_reader_init (&body_reader, byte_order, &signature, 0,
                              &body, 0);
      _dbus_type_reader_init (&copy_reader, opposite_order, &signature, 0,
                              &copy, 0);

      if (!_dbus_type_reader_equal_values (&body_reader, &copy_reader))
        {
          _dbus_verbose_bytes_of_string (&signature, 0,
                                         _dbus_string_get_length (&signature));
          _dbus_verbose_bytes_of_string (&body, 0,
                                         _dbus_string_get_length (&body));
          _dbus_verbose_bytes_of_string (&copy, 0,
                                         _dbus_string_get_length (&copy));

          _dbus_test_fatal ("Byte-swapped data did not have same values as original data");
        }

      _dbus_string_free (&copy);

      _dbus_string_set_length (&signature, 0);
      _dbus_string_set_length (&body, 0);
      ++sequence;
    }

  _dbus_string_free (&signature);
  _dbus_string_free (&body);

  _dbus_test_diag ("  %d blocks swapped from order '%c' to '%c'",
          sequence, byte_order, opposite_order);
}

dbus_bool_t
_dbus_marshal_byteswap_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  do_byteswap_test (DBUS_LITTLE_ENDIAN);
  do_byteswap_test (DBUS_BIG_ENDIAN);

  return TRUE;
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
