/* -*- mode: C; c-file-style: "gnu" -*- */
/* test-main.c  main() for make check
 *
 * Copyright 2003-2007 Red Hat, Inc.
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

#include <dbus/dbus-test-tap.h>

#include "test/test-utils.h"

#if !defined(DBUS_ENABLE_EMBEDDED_TESTS) || !defined(DBUS_UNIX)
#error This file is only relevant for the embedded tests on Unix
#endif

static DBusTestCase test =
{
  "config-parser-trivial",
  bus_config_parser_trivial_test
};

int
main (int argc, char **argv)
{
  return _dbus_test_main (argc, argv, 1, &test,
                          (DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS |
                           DBUS_TEST_FLAGS_REQUIRE_DATA),
                          NULL, NULL);
}
