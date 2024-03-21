/*
 * Copyright (c) 2023 Ralf Habacker
 * SPDX-License-Identifier: MIT
 */

#include <config.h>

#include "dbus/dbus-string.h"
#include "dbus/dbus-test.h"
#include "dbus/dbus-test-tap.h"
#include "test/test-utils.h"

static dbus_bool_t
_dbus_string_skip_blank_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  int end;
  DBusString s = _DBUS_STRING_INIT_INVALID;
  const char *p = " \rT\r\n";

  _dbus_string_init (&s);
  if (!_dbus_string_append (&s, p))
    {
      _dbus_string_free (&s);
      return FALSE;
    }

  _dbus_string_skip_blank (&s, 0, &end);
  _dbus_string_free (&s);
  return TRUE;
}

static const DBusTestCase test[] =
{
  { "skip_blank", _dbus_string_skip_blank_test },
};


int
main (int    argc,
      char **argv)
{
  return _dbus_test_main (argc, argv, sizeof(test) / sizeof (DBusTestCase), test,
                          DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS,
                          NULL, NULL);
}
