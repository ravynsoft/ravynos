/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-test-tool - D-Bus swiss army knife
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

#include <config.h>
#include "test-tool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dbus/dbus.h>

static struct {
    const char *name;
    int (*callback) (int, char **);
} subcommands[] = {
      { "black-hole", dbus_test_tool_black_hole },
      { "echo",       dbus_test_tool_echo },
      { "spam",       dbus_test_tool_spam },
      { NULL, NULL }
};

static void usage (int exit_with) _DBUS_GNUC_NORETURN;

static void
usage (int exit_with)
{
  int i;

  fprintf (stderr,
           "Usage: dbus-test-tool SUBCOMMAND [OPTIONS]\n"
           "\n"
           "Known SUBCOMMANDs are:\n"
           "\n"
           );

  for (i = 0; subcommands[i].name != NULL; i++)
    {
      fprintf (stderr, "- %s\n", subcommands[i].name);
    }

  fprintf (stderr,
           "\n"
           "For more information: dbus-test-tool SUBCOMMAND --help\n"
           );

  exit (exit_with);
}

int
main (int argc, char **argv)
{
  int i;

  if (argc < 2)
    {
      usage (2);
    }

  for (i = 0; subcommands[i].name != NULL; i++)
    {
      if (!strcmp (argv[1], subcommands[i].name))
        return subcommands[i].callback (argc, argv);
    }

  usage (2);
  return 2;
}
