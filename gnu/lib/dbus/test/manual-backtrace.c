/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* test-backtrace.c backtrace test app
 *
 * Copyright (C) 2015 Ralf Habacker <ralf.habacker@freenet.de>
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

#include "config.h"

#include <dbus/dbus.h>
#include <dbus/dbus-sysdeps.h>

#include <stdio.h>

static void
test2 (void)
{
  _dbus_print_backtrace();
}

static void
test1 (void)
{
  test2();
}

static void
test (void)
{
  test1();
}

int
main (int argc, char **argv)
{
  if (argc == 2)
    {
      fprintf(stderr, "dbus_abort test\n");
      _dbus_abort ();
    }
  else
    {
      test();
    }
  return 0;
}
