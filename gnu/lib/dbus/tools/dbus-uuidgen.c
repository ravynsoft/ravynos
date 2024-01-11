/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-uuidgen.c  Utility program to create UUIDs
 *
 * Copyright (C) 2006 Red Hat, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dbus/dbus-uuidgen.h>
#include <dbus/dbus.h>

static void usage (const char *name, int ecode) _DBUS_GNUC_NORETURN;

static void
usage (const char *name,
       int ecode)
{
  if (name == NULL)
    name = "dbus-uuidgen";
  
  fprintf (stderr, "Usage: %s [--ensure[=FILENAME]] [--get[=FILENAME]]\n", name);
  exit (ecode);
}

static void version (void) _DBUS_GNUC_NORETURN;

static void
version (void)
{
  printf ("D-Bus UUID Generator %s\n"
          "Copyright (C) 2006 Red Hat, Inc.\n"
          "This is free software; see the source for copying conditions.\n"
          "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
          VERSION);
  exit (0);
}

static dbus_bool_t
get_arg (const char  *arg,
         const char  *option,
         const char **value_p)
{
  const char *fn;

  if (strlen(arg) < strlen(option))
    return FALSE;
  
  fn = arg + strlen(option);

  if (!(*fn == '=' || *fn == ' ' || *fn == '\0'))
    {
      usage (NULL, 1);
    }
  
  if (*fn == '=')
    ++fn;
  
  while (*fn == ' ' && *fn != '\0')
    ++fn;
  
  if (*fn != '\0')
    {
      *value_p = fn;
      return TRUE;
    }

  return FALSE;
}

int
main (int argc, char *argv[])
{
  int i;
  const char *filename;
  dbus_bool_t ensure_uuid;
  dbus_bool_t get_uuid;
  DBusError error;

  ensure_uuid = FALSE;
  get_uuid = FALSE;
  
  filename = NULL;

  for (i = 1; i < argc; i++)
    {
      char *arg = argv[i];

      if (strncmp (arg, "--ensure", strlen("--ensure")) == 0)
        {
          get_arg (arg, "--ensure", &filename);
          ensure_uuid = TRUE;
        }
      else if (strncmp (arg, "--get", strlen("--get")) == 0)
        {
          get_arg (arg, "--get", &filename);
          get_uuid = TRUE;
        }
      else if (strcmp (arg, "--help") == 0)
	usage (argv[0], 0);
      else if (strcmp (arg, "--version") == 0)
        version ();
      else
        usage (argv[0], 1);
    }

  if (get_uuid && ensure_uuid)
    {
      fprintf (stderr, "Can't specify both --get and --ensure\n");
      exit (1);
    }

  dbus_error_init (&error);
  
  if (get_uuid || ensure_uuid)
    {
      char *uuid;
      if (_dbus_get_uuid (filename, &uuid, ensure_uuid, &error))
        {
          if (get_uuid) /* print nothing on --ensure */
            printf ("%s\n", uuid);
          dbus_free (uuid);
        }
    }
  else
    {
      char *uuid;

      if (_dbus_create_uuid (&uuid, &error))
        {
          printf ("%s\n", uuid);
          dbus_free (uuid);
        }
    }

  if (dbus_error_is_set (&error))
    {
      fprintf (stderr, "%s\n", error.message);
      dbus_error_free (&error);
      exit (1);
    }
  else
    {
      exit (0);
    }
}
