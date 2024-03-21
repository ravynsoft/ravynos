/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-string-util.c Would be in dbus-string.c, but not used in libdbus
 *
 * Copyright 2002-2008 Red Hat, Inc.
 * Copyright 2003 CodeFactory AB
 * Copyright 2003 Mark McLoughlin
 * Copyright 2004 Michael Meeks
 * Copyright 2006-2015 Ralf Habacker <ralf.habacker@freenet.de>
 * Copyright 2007 Allison Lortie
 * Copyright 2011 Roberto Guido
 * Copyright 2016-2018 Collabora Ltd.
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

#include "dbus/dbus-internals.h"
#include "dbus/dbus-string.h"
#define DBUS_CAN_USE_DBUS_STRING_PRIVATE 1
#include "dbus/dbus-string-private.h"
#include <dbus/dbus-test-tap.h>

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus/dbus-test.h"
#include <stdio.h>

static void
test_hex_roundtrip (const char *data,
                    int         len)
{
  DBusString orig;
  DBusString encoded;
  DBusString decoded;
  int end;

  if (len < 0)
    len = strlen (data);

  if (!_dbus_string_init (&orig))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_init (&encoded))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_init (&decoded))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_append_len (&orig, data, len))
    _dbus_test_fatal ("couldn't append orig data");

  if (!_dbus_string_hex_encode (&orig, 0, &encoded, 0))
    _dbus_test_fatal ("could not encode");

  if (!_dbus_string_hex_decode (&encoded, 0, &end, &decoded, 0))
    _dbus_test_fatal ("could not decode");

  _dbus_assert (_dbus_string_get_length (&encoded) == end);

  if (!_dbus_string_equal (&orig, &decoded))
    {
      const char *s;

      _dbus_test_diag ("Original string %d bytes encoded %d bytes decoded %d bytes",
              _dbus_string_get_length (&orig),
              _dbus_string_get_length (&encoded),
              _dbus_string_get_length (&decoded));
      _dbus_test_diag ("Original: %s", data);
      s = _dbus_string_get_const_data (&decoded);
      _dbus_test_diag ("Decoded: %s", s);
      _dbus_test_fatal ("original string not the same as string decoded from hex");
    }

  _dbus_string_free (&orig);
  _dbus_string_free (&encoded);
  _dbus_string_free (&decoded);
}

typedef void (* TestRoundtripFunc) (const char *data,
                                    int         len);
static void
test_roundtrips (TestRoundtripFunc func)
{
  (* func) ("Hello this is a string\n", -1);
  (* func) ("Hello this is a string\n1", -1);
  (* func) ("Hello this is a string\n12", -1);
  (* func) ("Hello this is a string\n123", -1);
  (* func) ("Hello this is a string\n1234", -1);
  (* func) ("Hello this is a string\n12345", -1);
  (* func) ("", 0);
  (* func) ("1", 1);
  (* func) ("12", 2);
  (* func) ("123", 3);
  (* func) ("1234", 4);
  (* func) ("12345", 5);
  (* func) ("", 1);
  (* func) ("1", 2);
  (* func) ("12", 3);
  (* func) ("123", 4);
  (* func) ("1234", 5);
  (* func) ("12345", 6);
  {
    unsigned char buf[512];
    int i;

    i = 0;
    while (i < _DBUS_N_ELEMENTS (buf))
      {
        buf[i] = (i & 0xff);
        ++i;
      }
    i = 0;
    while (i < _DBUS_N_ELEMENTS (buf))
      {
        (* func) ((const char *) buf, i);
        ++i;
      }
  }
}

_DBUS_STRING_DEFINE_STATIC (test_static_string, "hello");

/**
 * @ingroup DBusStringInternals
 * Unit test for DBusString.
 *
 * @todo Need to write tests for _dbus_string_copy() and
 * _dbus_string_move() moving to/from each of start/middle/end of a
 * string. Also need tests for _dbus_string_move_len ()
 *
 * @returns #TRUE on success.
 */
dbus_bool_t
_dbus_string_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusString str = _DBUS_STRING_INIT_INVALID;
  DBusString other;
  int i, a, end;
  long v;
  int lens[] = { 0, 1, 2, 3, 4, 5, 10, 16, 17, 18, 25, 31, 32, 33, 34, 35, 63, 64, 65, 66, 67, 68, 69, 70, 71, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136 };
  char *s;
  DBusRealString *real_test_static_string = (DBusRealString *) &test_static_string;

  _dbus_assert (real_test_static_string->str != NULL);
  _dbus_assert (strcmp ((const char *) real_test_static_string->str, "hello") == 0);
  _dbus_assert (real_test_static_string->len == 5);
  _dbus_assert (real_test_static_string->allocated > 5);
  _dbus_assert (real_test_static_string->constant);
  _dbus_assert (real_test_static_string->locked);
  _dbus_assert (real_test_static_string->valid);
  _dbus_assert (real_test_static_string->align_offset == 0);

  /* Test that _DBUS_STRING_INIT_INVALID has the desired effect */
  _dbus_string_free (&str);
  _dbus_string_free (&str);
  _dbus_string_free (&str);

  /* Test shortening and setting length */
  i = 0;
  while (i < _DBUS_N_ELEMENTS (lens))
    {
      int j;

      if (!_dbus_string_init (&str))
        _dbus_test_fatal ("failed to init string");

      if (!_dbus_string_set_length (&str, lens[i]))
        _dbus_test_fatal ("failed to set string length");

      j = lens[i];
      while (j > 0)
        {
          _dbus_assert (_dbus_string_get_length (&str) == j);
          if (j > 0)
            {
              _dbus_string_shorten (&str, 1);
              _dbus_assert (_dbus_string_get_length (&str) == (j - 1));
            }
          --j;
        }

      _dbus_string_free (&str);
      /* Test that a cleared string is effectively _DBUS_STRING_INIT_INVALID */
      _dbus_string_free (&str);
      _dbus_string_free (&str);

      ++i;
    }

  /* Test equality */
  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("oom");

  if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("oom");

  _dbus_string_init_const (&other, "H");
  _dbus_assert (_dbus_string_equal_substring (&str, 0, 1, &other, 0));
  _dbus_assert (_dbus_string_equal_substring (&str, 1, 0, &other, 1));
  _dbus_string_init_const (&other, "Hello");
  _dbus_assert (_dbus_string_equal_substring (&str, 0, 5, &other, 0));
  _dbus_assert (_dbus_string_equal_substring (&str, 1, 4, &other, 1));
  _dbus_assert (_dbus_string_equal_substring (&str, 2, 3, &other, 2));
  _dbus_assert (_dbus_string_equal_substring (&str, 3, 2, &other, 3));
  _dbus_assert (_dbus_string_equal_substring (&str, 4, 1, &other, 4));
  _dbus_assert (_dbus_string_equal_substring (&str, 5, 0, &other, 5));

  _dbus_assert (_dbus_string_equal_substring (&other, 0, 5, &str, 0));
  _dbus_assert (_dbus_string_equal_substring (&other, 1, 4, &str, 1));
  _dbus_assert (_dbus_string_equal_substring (&other, 2, 3, &str, 2));
  _dbus_assert (_dbus_string_equal_substring (&other, 3, 2, &str, 3));
  _dbus_assert (_dbus_string_equal_substring (&other, 4, 1, &str, 4));
  _dbus_assert (_dbus_string_equal_substring (&other, 5, 0, &str, 5));


  _dbus_string_init_const (&other, "World");
  _dbus_assert (_dbus_string_equal_substring (&str, 6,  5, &other, 0));
  _dbus_assert (_dbus_string_equal_substring (&str, 7,  4, &other, 1));
  _dbus_assert (_dbus_string_equal_substring (&str, 8,  3, &other, 2));
  _dbus_assert (_dbus_string_equal_substring (&str, 9,  2, &other, 3));
  _dbus_assert (_dbus_string_equal_substring (&str, 10, 1, &other, 4));
  _dbus_assert (_dbus_string_equal_substring (&str, 11, 0, &other, 5));

  _dbus_assert (_dbus_string_equal_substring (&other, 0, 5, &str, 6));
  _dbus_assert (_dbus_string_equal_substring (&other, 1, 4, &str, 7));
  _dbus_assert (_dbus_string_equal_substring (&other, 2, 3, &str, 8));
  _dbus_assert (_dbus_string_equal_substring (&other, 3, 2, &str, 9));
  _dbus_assert (_dbus_string_equal_substring (&other, 4, 1, &str, 10));
  _dbus_assert (_dbus_string_equal_substring (&other, 5, 0, &str, 11));

  _dbus_string_free (&str);

  /* Test appending data */
  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("failed to init string");

  i = 0;
  while (i < 10)
    {
      if (!_dbus_string_append (&str, "a"))
        _dbus_test_fatal ("failed to append string to string");

      _dbus_assert (_dbus_string_get_length (&str) == i * 2 + 1);

      if (!_dbus_string_append_byte (&str, 'b'))
        _dbus_test_fatal ("failed to append byte to string");

      _dbus_assert (_dbus_string_get_length (&str) == i * 2 + 2);

      ++i;
    }

  _dbus_string_free (&str);

  /* Check steal_data */

  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("failed to init string");

  if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("could not append to string");

  i = _dbus_string_get_length (&str);

  if (!_dbus_string_steal_data (&str, &s))
    _dbus_test_fatal ("failed to steal data");

  _dbus_assert (_dbus_string_get_length (&str) == 0);
  _dbus_assert (((int)strlen (s)) == i);

  dbus_free (s);

  /* Check move */

  if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("could not append to string");

  i = _dbus_string_get_length (&str);

  if (!_dbus_string_init (&other))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_move (&str, 0, &other, 0))
    _dbus_test_fatal ("could not move");

  _dbus_assert (_dbus_string_get_length (&str) == 0);
  _dbus_assert (_dbus_string_get_length (&other) == i);

  if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("could not append to string");

  if (!_dbus_string_move (&str, 0, &other, _dbus_string_get_length (&other)))
    _dbus_test_fatal ("could not move");

  _dbus_assert (_dbus_string_get_length (&str) == 0);
  _dbus_assert (_dbus_string_get_length (&other) == i * 2);

    if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("could not append to string");

  if (!_dbus_string_move (&str, 0, &other, _dbus_string_get_length (&other) / 2))
    _dbus_test_fatal ("could not move");

  _dbus_assert (_dbus_string_get_length (&str) == 0);
  _dbus_assert (_dbus_string_get_length (&other) == i * 3);

  _dbus_string_free (&other);

  /* Check copy */

  if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("could not append to string");

  i = _dbus_string_get_length (&str);

  if (!_dbus_string_init (&other))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_copy (&str, 0, &other, 0))
    _dbus_test_fatal ("could not copy");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == i);

  if (!_dbus_string_copy (&str, 0, &other, _dbus_string_get_length (&other)))
    _dbus_test_fatal ("could not copy");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == i * 2);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "Hello WorldHello World"));

  if (!_dbus_string_copy (&str, 0, &other, _dbus_string_get_length (&other) / 2))
    _dbus_test_fatal ("could not copy");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == i * 3);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "Hello WorldHello WorldHello World"));

  _dbus_string_free (&str);
  _dbus_string_free (&other);

  /* Check replace */

  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("failed to init string");

  if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("could not append to string");

  i = _dbus_string_get_length (&str);

  if (!_dbus_string_init (&other))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_replace_len (&str, 0, _dbus_string_get_length (&str),
                                 &other, 0, _dbus_string_get_length (&other)))
    _dbus_test_fatal ("could not replace");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == i);
  _dbus_assert (_dbus_string_equal_c_str (&other, "Hello World"));

  if (!_dbus_string_replace_len (&str, 0, _dbus_string_get_length (&str),
                                 &other, 5, 1))
    _dbus_test_fatal ("could not replace center space");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == i * 2 - 1);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "HelloHello WorldWorld"));


  if (!_dbus_string_replace_len (&str, 1, 1,
                                 &other,
                                 _dbus_string_get_length (&other) - 1,
                                 1))
    _dbus_test_fatal ("could not replace end character");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == i * 2 - 1);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "HelloHello WorldWorle"));

  _dbus_string_free (&str);
  _dbus_string_free (&other);

  /* Different tests are provided because different behaviours are
   * implemented in _dbus_string_replace_len() in function of replacing and
   * replaced lengths
   */

  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("failed to init string");

  if (!_dbus_string_append (&str, "Hello World"))
    _dbus_test_fatal ("could not append to string");

  i = _dbus_string_get_length (&str);

  if (!_dbus_string_init (&other))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_append (&other, "Foo String"))
    _dbus_test_fatal ("could not append to string");

  a = _dbus_string_get_length (&other);

  if (!_dbus_string_replace_len (&str, 0, 6,
                                 &other, 4, 0))
    _dbus_test_fatal ("could not replace 0 length");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == a + 6);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "Foo Hello String"));

  if (!_dbus_string_replace_len (&str, 5, 6,
                                 &other,
                                 _dbus_string_get_length (&other),
                                 0))
    _dbus_test_fatal ("could not replace at the end");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == a + 6 + 6);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "Foo Hello String World"));

  if (!_dbus_string_replace_len (&str, 0, 5,
                                 &other,
                                 _dbus_string_get_length (&other) - 5,
                                 5))
    _dbus_test_fatal ("could not replace same length");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == a + 6 + 6);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "Foo Hello String Hello"));

  if (!_dbus_string_replace_len (&str, 6, 5,
                                 &other, 4, 12))
    _dbus_test_fatal ("could not replace with shorter string");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == a + 5);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "Foo World Hello"));

  if (!_dbus_string_replace_len (&str, 0, 1,
                                 &other, 0, 3))
    _dbus_test_fatal ("could not replace at the beginning");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == a + 3);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "H World Hello"));

  if (!_dbus_string_replace_len (&str, 6, 5,
                                 &other,
                                 _dbus_string_get_length (&other) - 5,
                                 5))
    _dbus_test_fatal ("could not replace same length");

  _dbus_assert (_dbus_string_get_length (&str) == i);
  _dbus_assert (_dbus_string_get_length (&other) == a + 3);
  _dbus_assert (_dbus_string_equal_c_str (&other,
                                          "H World World"));

  _dbus_string_free (&str);
  _dbus_string_free (&other);

  /* Check insert/set/get byte */

  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("failed to init string");

  if (!_dbus_string_append (&str, "Hello"))
    _dbus_test_fatal ("failed to append Hello");

  _dbus_assert (_dbus_string_get_byte (&str, 0) == 'H');
  _dbus_assert (_dbus_string_get_byte (&str, 1) == 'e');
  _dbus_assert (_dbus_string_get_byte (&str, 2) == 'l');
  _dbus_assert (_dbus_string_get_byte (&str, 3) == 'l');
  _dbus_assert (_dbus_string_get_byte (&str, 4) == 'o');

  _dbus_string_set_byte (&str, 1, 'q');
  _dbus_assert (_dbus_string_get_byte (&str, 1) == 'q');

  if (!_dbus_string_insert_bytes (&str, 0, 1, 255))
    _dbus_test_fatal ("can't insert byte");

  if (!_dbus_string_insert_bytes (&str, 2, 4, 'Z'))
    _dbus_test_fatal ("can't insert byte");

  if (!_dbus_string_insert_bytes (&str, _dbus_string_get_length (&str), 1, 'W'))
    _dbus_test_fatal ("can't insert byte");

  _dbus_assert (_dbus_string_get_byte (&str, 0) == 255);
  _dbus_assert (_dbus_string_get_byte (&str, 1) == 'H');
  _dbus_assert (_dbus_string_get_byte (&str, 2) == 'Z');
  _dbus_assert (_dbus_string_get_byte (&str, 3) == 'Z');
  _dbus_assert (_dbus_string_get_byte (&str, 4) == 'Z');
  _dbus_assert (_dbus_string_get_byte (&str, 5) == 'Z');
  _dbus_assert (_dbus_string_get_byte (&str, 6) == 'q');
  _dbus_assert (_dbus_string_get_byte (&str, 7) == 'l');
  _dbus_assert (_dbus_string_get_byte (&str, 8) == 'l');
  _dbus_assert (_dbus_string_get_byte (&str, 9) == 'o');
  _dbus_assert (_dbus_string_get_byte (&str, 10) == 'W');

  _dbus_string_free (&str);

  /* Check append/parse int/double */

  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("failed to init string");

  if (!_dbus_string_append_int (&str, 27))
    _dbus_test_fatal ("failed to append int");

  i = _dbus_string_get_length (&str);

  if (!_dbus_string_parse_int (&str, 0, &v, &end))
    _dbus_test_fatal ("failed to parse int");

  _dbus_assert (v == 27);
  _dbus_assert (end == i);

  _dbus_string_free (&str);

  /* Test find */
  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("failed to init string");

  if (!_dbus_string_append (&str, "Hello"))
    _dbus_test_fatal ("couldn't append to string");

  if (!_dbus_string_find (&str, 0, "He", &i))
    _dbus_test_fatal ("didn't find 'He'");
  _dbus_assert (i == 0);

  if (!_dbus_string_find (&str, 0, "Hello", &i))
    _dbus_test_fatal ("didn't find 'Hello'");
  _dbus_assert (i == 0);

  if (!_dbus_string_find (&str, 0, "ello", &i))
    _dbus_test_fatal ("didn't find 'ello'");
  _dbus_assert (i == 1);

  if (!_dbus_string_find (&str, 0, "lo", &i))
    _dbus_test_fatal ("didn't find 'lo'");
  _dbus_assert (i == 3);

  if (!_dbus_string_find (&str, 2, "lo", &i))
    _dbus_test_fatal ("didn't find 'lo'");
  _dbus_assert (i == 3);

  if (_dbus_string_find (&str, 4, "lo", &i))
    _dbus_test_fatal ("did find 'lo'");

  if (!_dbus_string_find (&str, 0, "l", &i))
    _dbus_test_fatal ("didn't find 'l'");
  _dbus_assert (i == 2);

  if (!_dbus_string_find (&str, 0, "H", &i))
    _dbus_test_fatal ("didn't find 'H'");
  _dbus_assert (i == 0);

  if (!_dbus_string_find (&str, 0, "", &i))
    _dbus_test_fatal ("didn't find ''");
  _dbus_assert (i == 0);

  if (_dbus_string_find (&str, 0, "Hello!", NULL))
    _dbus_test_fatal ("Did find 'Hello!'");

  if (_dbus_string_find (&str, 0, "Oh, Hello", NULL))
    _dbus_test_fatal ("Did find 'Oh, Hello'");

  if (_dbus_string_find (&str, 0, "ill", NULL))
    _dbus_test_fatal ("Did find 'ill'");

  if (_dbus_string_find (&str, 0, "q", NULL))
    _dbus_test_fatal ("Did find 'q'");

  if (!_dbus_string_find_to (&str, 0, 2, "He", NULL))
    _dbus_test_fatal ("Didn't find 'He'");

  if (_dbus_string_find_to (&str, 0, 2, "Hello", NULL))
    _dbus_test_fatal ("Did find 'Hello'");

  if (!_dbus_string_find_byte_backward (&str, _dbus_string_get_length (&str), 'H', &i))
    _dbus_test_fatal ("Did not find 'H'");
  _dbus_assert (i == 0);

  if (!_dbus_string_find_byte_backward (&str, _dbus_string_get_length (&str), 'o', &i))
    _dbus_test_fatal ("Did not find 'o'");
  _dbus_assert (i == _dbus_string_get_length (&str) - 1);

  if (_dbus_string_find_byte_backward (&str, _dbus_string_get_length (&str) - 1, 'o', &i))
    _dbus_test_fatal ("Did find 'o'");
  _dbus_assert (i == -1);

  if (_dbus_string_find_byte_backward (&str, 1, 'e', &i))
    _dbus_test_fatal ("Did find 'e'");
  _dbus_assert (i == -1);

  if (!_dbus_string_find_byte_backward (&str, 2, 'e', &i))
    _dbus_test_fatal ("Didn't find 'e'");
  _dbus_assert (i == 1);

  _dbus_string_free (&str);

  /* Hex encoding */
  _dbus_string_init_const (&str, "cafebabe, this is a bogus hex string");
  if (!_dbus_string_init (&other))
    _dbus_test_fatal ("could not init string");

  if (!_dbus_string_hex_decode (&str, 0, &end, &other, 0))
    _dbus_test_fatal ("deccoded bogus hex string with no error");

  _dbus_assert (end == 8);

  _dbus_string_free (&other);

  test_roundtrips (test_hex_roundtrip);

  _dbus_string_free (&str);

  {
    int found, found_len;

    _dbus_string_init_const (&str, "012\r\n567\n90");

    if (!_dbus_string_find_eol (&str, 0, &found, &found_len) || found != 3 || found_len != 2)
      _dbus_test_fatal ("Did not find '\\r\\n'");
    if (found != 3 || found_len != 2)
      _dbus_test_fatal ("invalid return values");

    if (!_dbus_string_find_eol (&str, 5, &found, &found_len))
      _dbus_test_fatal ("Did not find '\\n'");
    if (found != 8 || found_len != 1)
      _dbus_test_fatal ("invalid return values");

    if (_dbus_string_find_eol (&str, 9, &found, &found_len))
      _dbus_test_fatal ("Found not expected '\\n'");
    else if (found != 11 || found_len != 0)
      _dbus_test_fatal ("invalid return values '\\n'");

    found = -1;
    found_len = -1;
    _dbus_string_init_const (&str, "");
    if (_dbus_string_find_eol (&str, 0, &found, &found_len))
      _dbus_test_fatal ("found an eol in an empty string");
    _dbus_assert (found == 0);
    _dbus_assert (found_len == 0);

    found = -1;
    found_len = -1;
    _dbus_string_init_const (&str, "foobar");
    if (_dbus_string_find_eol (&str, 0, &found, &found_len))
      _dbus_test_fatal ("found eol in string that lacks one");
    _dbus_assert (found == 6);
    _dbus_assert (found_len == 0);

    found = -1;
    found_len = -1;
    _dbus_string_init_const (&str, "foobar\n");
    if (!_dbus_string_find_eol (&str, 0, &found, &found_len))
      _dbus_test_fatal ("did not find eol in string that has one at end");
    _dbus_assert (found == 6);
    _dbus_assert (found_len == 1);
  }

  {
    DBusString line;

#define FIRST_LINE "this is a line"
#define SECOND_LINE "this is a second line"
    /* third line is empty */
#define THIRD_LINE ""
#define FOURTH_LINE "this is a fourth line"

    if (!_dbus_string_init (&str))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_append (&str, FIRST_LINE "\n" SECOND_LINE "\r\n" THIRD_LINE "\n" FOURTH_LINE))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_init (&line))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_pop_line (&str, &line))
      _dbus_test_fatal ("failed to pop first line");

    _dbus_assert (_dbus_string_equal_c_str (&line, FIRST_LINE));

    if (!_dbus_string_pop_line (&str, &line))
      _dbus_test_fatal ("failed to pop second line");

    _dbus_assert (_dbus_string_equal_c_str (&line, SECOND_LINE));

    if (!_dbus_string_pop_line (&str, &line))
      _dbus_test_fatal ("failed to pop third line");

    _dbus_assert (_dbus_string_equal_c_str (&line, THIRD_LINE));

    if (!_dbus_string_pop_line (&str, &line))
      _dbus_test_fatal ("failed to pop fourth line");

    _dbus_assert (_dbus_string_equal_c_str (&line, FOURTH_LINE));

    _dbus_string_free (&str);
    _dbus_string_free (&line);
  }

  {
    if (!_dbus_string_init (&str))
      _dbus_test_fatal ("no memory");

    for (i = 0; i < 10000; i++)
      if (!_dbus_string_append (&str, "abcdefghijklmnopqrstuvwxyz"))
        _dbus_test_fatal ("no memory");

    if (!_dbus_string_set_length (&str, 10))
      _dbus_test_fatal ("failed to set length");

    /* actually compact */
    if (!_dbus_string_compact (&str, 2048))
      _dbus_test_fatal ("failed to compact after set_length");

    /* peek inside to make sure it worked */
    if (((DBusRealString *)&str)->allocated > 30)
      _dbus_test_fatal ("compacting string didn't do anything");

    if (!_dbus_string_equal_c_str (&str, "abcdefghij"))
      _dbus_test_fatal ("unexpected content after compact");

    /* compact nothing */
    if (!_dbus_string_compact (&str, 2048))
      _dbus_test_fatal ("failed to compact 2nd time");

    if (!_dbus_string_equal_c_str (&str, "abcdefghij"))
      _dbus_test_fatal ("unexpected content after 2nd compact");

    /* and make sure it still works...*/
    if (!_dbus_string_append (&str, "123456"))
      _dbus_test_fatal ("failed to append after compact");

    if (!_dbus_string_equal_c_str (&str, "abcdefghij123456"))
      _dbus_test_fatal ("unexpected content after append");

    /* after growing automatically, this should do nothing */
    if (!_dbus_string_compact (&str, 20000))
      _dbus_test_fatal ("failed to compact after grow");

    /* but this one will do something */
    if (!_dbus_string_compact (&str, 0))
      _dbus_test_fatal ("failed to compact after grow");

    if (!_dbus_string_equal_c_str (&str, "abcdefghij123456"))
      _dbus_test_fatal ("unexpected content");

    if (!_dbus_string_append (&str, "!@#$%"))
      _dbus_test_fatal ("failed to append after compact");

    if (!_dbus_string_equal_c_str (&str, "abcdefghij123456!@#$%"))
      _dbus_test_fatal ("unexpected content");

    _dbus_string_free (&str);
  }

  {
    const char two_strings[] = "one\ttwo";

    if (!_dbus_string_init (&str))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_init (&other))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_append (&str, two_strings))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_split_on_byte (&str, '\t', &other))
      _dbus_test_fatal ("no memory or delimiter not found");

    if (strcmp (_dbus_string_get_data (&str), "one") != 0)
      _dbus_test_fatal ("left side after split on tab is wrong");

    if (strcmp (_dbus_string_get_data (&other), "two") != 0)
      _dbus_test_fatal ("right side after split on tab is wrong");

    _dbus_string_free (&str);
    _dbus_string_free (&other);
  }

  {
    const char upper_string[] = "TOUPPERSTRING";
    const char lower_string[] = "toupperstring";
    const char lower2_string[] = "toupperSTRING";

    if (!_dbus_string_init (&str))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_append (&str, upper_string))
      _dbus_test_fatal ("no memory");

    _dbus_string_tolower_ascii (&str, 0, _dbus_string_get_length(&str));

    if (!_dbus_string_equal_c_str (&str, lower_string))
      _dbus_test_fatal ("_dbus_string_tolower_ascii failed");

    _dbus_string_free (&str);

    if (!_dbus_string_init (&str))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_append (&str, upper_string))
      _dbus_test_fatal ("no memory");

    _dbus_string_tolower_ascii (&str, 0, 7);

    if (!_dbus_string_equal_c_str (&str, lower2_string))
      _dbus_test_fatal ("_dbus_string_tolower_ascii failed in partial conversion");

    _dbus_string_free (&str);
  }

  {
    const char lower_string[] = "toupperstring";
    const char upper_string[] = "TOUPPERSTRING";
    const char upper2_string[] = "TOUPPERstring";

    if (!_dbus_string_init (&str))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_append (&str, lower_string))
      _dbus_test_fatal ("no memory");

    _dbus_string_toupper_ascii (&str, 0, _dbus_string_get_length(&str));

    if (!_dbus_string_equal_c_str (&str, upper_string))
      _dbus_test_fatal ("_dbus_string_toupper_ascii failed");

    _dbus_string_free (&str);

    if (!_dbus_string_init (&str))
      _dbus_test_fatal ("no memory");

    if (!_dbus_string_append (&str, lower_string))
      _dbus_test_fatal ("no memory");

    _dbus_string_toupper_ascii (&str, 0, 7);

    if (!_dbus_string_equal_c_str (&str, upper2_string))
      _dbus_test_fatal ("_dbus_string_toupper_ascii failed in partial conversion");

    _dbus_string_free (&str);
  }

  return TRUE;
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
