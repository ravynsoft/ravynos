/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-address.c  Server address parser.
 *
 * Copyright (C) 2003  CodeFactory AB
 * Copyright (C) 2004-2006  Red Hat, Inc.
 * Copyright (C) 2007  Ralf Habacker
 * Copyright (C) 2013  Chengwei Yang / Intel
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

#include <stdlib.h>
#include <string.h>

#include <dbus/dbus.h>
#include "dbus/dbus-internals.h"
#include "dbus/dbus-test-tap.h"

typedef struct
{
  const char *escaped;
  const char *unescaped;
} EscapeTest;

static const EscapeTest escape_tests[] = {
  { "abcde", "abcde" },
  { "", "" },
  { "%20%20", "  " },
  { "%24", "$" },
  { "%25", "%" },
  { "abc%24", "abc$" },
  { "%24abc", "$abc" },
  { "abc%24abc", "abc$abc" },
  { "/", "/" },
  { "-", "-" },
  { "_", "_" },
  { "A", "A" },
  { "I", "I" },
  { "Z", "Z" },
  { "a", "a" },
  { "i", "i" },
  { "z", "z" },
  /* Bug: https://bugs.freedesktop.org/show_bug.cgi?id=53499 */
  { "%c3%b6", "\xc3\xb6" }
};

static const char* invalid_escaped_values[] = {
  "%a",
  "%q",
  "%az",
  "%%",
  "%$$",
  "abc%a",
  "%axyz",
  "%",
  "$",
  " ",
};

dbus_bool_t
_dbus_address_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusAddressEntry **entries;
  int len;
  DBusError error = DBUS_ERROR_INIT;
  int i;

  i = 0;
  while (i < _DBUS_N_ELEMENTS (escape_tests))
    {
      const EscapeTest *test = &escape_tests[i];
      char *escaped;
      char *unescaped;

      escaped = dbus_address_escape_value (test->unescaped);
      if (escaped == NULL)
        _dbus_test_fatal ("oom");

      if (strcmp (escaped, test->escaped) != 0)
        {
          _dbus_warn ("Escaped '%s' as '%s' should have been '%s'",
                      test->unescaped, escaped, test->escaped);
          exit (1);
        }
      dbus_free (escaped);

      unescaped = dbus_address_unescape_value (test->escaped, &error);
      if (unescaped == NULL)
        {
          _dbus_warn ("Failed to unescape '%s': %s",
                      test->escaped, error.message);
          dbus_error_free (&error);
          exit (1);
        }

      if (strcmp (unescaped, test->unescaped) != 0)
        {
          _dbus_warn ("Unescaped '%s' as '%s' should have been '%s'",
                      test->escaped, unescaped, test->unescaped);
          exit (1);
        }
      dbus_free (unescaped);

      ++i;
    }

  i = 0;
  while (i < _DBUS_N_ELEMENTS (invalid_escaped_values))
    {
      char *unescaped;

      unescaped = dbus_address_unescape_value (invalid_escaped_values[i],
                                               &error);
      if (unescaped != NULL)
        {
          _dbus_warn ("Should not have successfully unescaped '%s' to '%s'",
                      invalid_escaped_values[i], unescaped);
          dbus_free (unescaped);
          exit (1);
        }

      _dbus_assert (dbus_error_is_set (&error));
      dbus_error_free (&error);

      ++i;
    }

  if (!dbus_parse_address ("unix:path=/tmp/foo;debug:name=test,sliff=sloff;",
			   &entries, &len, &error))
    _dbus_test_fatal ("could not parse address");
  _dbus_assert (len == 2);
  _dbus_assert (strcmp (dbus_address_entry_get_value (entries[0], "path"), "/tmp/foo") == 0);
  _dbus_assert (strcmp (dbus_address_entry_get_value (entries[1], "name"), "test") == 0);
  _dbus_assert (strcmp (dbus_address_entry_get_value (entries[1], "sliff"), "sloff") == 0);

  dbus_address_entries_free (entries);

  /* Different possible errors */
  if (dbus_parse_address ("", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo:bar", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo:bar,baz", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo:bar=foo,baz", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo:bar=foo;baz", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo:=foo", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo:foo=", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  if (dbus_parse_address ("foo:foo,bar=baz", &entries, &len, &error))
    _dbus_test_fatal ("Parsed incorrect address.");
  else
    dbus_error_free (&error);

  return TRUE;
}
