/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* test-main.c  main() for the OOM check of the launch helper
 *
 * Copyright 2007 Red Hat, Inc.
 * Copyright 2013-2018 Collabora Ltd.
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

#include "bus/test.h"

#include <dbus/dbus-internals.h>
#include <dbus/dbus-test-tap.h>

#include "bus/activation-helper.h"
#include "test/test-utils.h"

#if !defined(DBUS_ENABLE_EMBEDDED_TESTS) || !defined(DBUS_UNIX)
#error This file is only relevant for the embedded tests on Unix
#endif

/* Embed a version of the real activation helper that has been altered
 * to be testable. We monkey-patch it like this because we don't want to
 * compile test-only code into the real setuid executable, and Automake
 * versions older than 1.16 can't cope with expanding directory variables
 * in SOURCES when using subdir-objects. */
#define ACTIVATION_LAUNCHER_TEST
#define ACTIVATION_LAUNCHER_DO_OOM
#include "bus/activation-helper.c"

/* returns true if good things happen, or if we get OOM */
static dbus_bool_t
bus_activation_helper_oom_test (void        *data,
                                dbus_bool_t  have_memory)
{
  const char *service;
  DBusError error;
  dbus_bool_t retval;

  service = (const char *) data;
  retval = TRUE;

  dbus_error_init (&error);

  if (!run_launch_helper (service, &error))
    {
      _DBUS_ASSERT_ERROR_IS_SET (&error);
      /* we failed, but a OOM is good */
      if (!dbus_error_has_name (&error, DBUS_ERROR_NO_MEMORY))
        {
          _dbus_warn ("FAILED SELF TEST: Error: %s", error.message);
          retval = FALSE;
        }
      dbus_error_free (&error);
    }
  else
    {
      /* we succeeded, yay! */
      _DBUS_ASSERT_ERROR_IS_CLEAR (&error);
    }
  return retval;
}

static dbus_bool_t
bus_activation_helper_test (const char *test_data_dir)
{
  DBusString config_file;

  if (!_dbus_string_init (&config_file) ||
      !_dbus_string_append (&config_file, test_data_dir) ||
      !_dbus_string_append (&config_file, "/valid-config-files-system/debug-allow-all-pass.conf"))
    _dbus_test_fatal ("OOM during initialization");

  /* use a config file that will actually work... */
  dbus_setenv ("TEST_LAUNCH_HELPER_CONFIG",
               _dbus_string_get_const_data (&config_file));

  _dbus_string_free (&config_file);

  if (!_dbus_test_oom_handling ("dbus-daemon-launch-helper",
                                bus_activation_helper_oom_test,
                                (char *) "org.freedesktop.DBus.TestSuiteEchoService"))
    _dbus_test_fatal ("OOM test failed");

  /* ... otherwise it must have passed */
  return TRUE;
}

static DBusTestCase test =
{
  "activation-helper",
  bus_activation_helper_test,
};

int
main (int argc, char **argv)
{
  return _dbus_test_main (argc, argv, 1, &test,
                          (DBUS_TEST_FLAGS_REQUIRE_DATA |
                           DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS),
                          NULL, NULL);
}
