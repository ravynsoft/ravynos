/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-echo.c - a plain libdbus echo server
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <dbus/dbus.h>

#include "dbus/dbus-sysdeps.h"
#include "test-tool.h"
#include "tool-common.h"

static int sleep_ms = -1;
static dbus_bool_t noreply = FALSE;
static dbus_bool_t noread = FALSE;

static void usage_echo (int exit_with) _DBUS_GNUC_NORETURN;

static void
usage_echo (int exit_with)
{
  fprintf (stderr,
           "Usage: dbus-test-tool echo [OPTIONS]\n"
           "\n"
           "Respond to all method calls with an empty reply.\n"
           "\n"
           "Options:\n"
           "\n"
           "    --name=NAME   claim this well-known name first\n"
           "\n"
           "    --sleep-ms=N  sleep N milliseconds before sending each reply\n"
           "\n"
           "    --session     use the session bus (default)\n"
           "    --system      use the system bus\n"
           );
  exit (exit_with);
}

static void usage_black_hole (int exit_with) _DBUS_GNUC_NORETURN;

static void
usage_black_hole (int exit_with)
{
  fprintf (stderr,
           "Usage: dbus-test-tool black-hole [OPTIONS]\n"
           "\n"
           "Receive method calls but do not reply.\n"
           "\n"
           "Options:\n"
           "\n"
           "    --name=NAME   claim this well-known name first\n"
           "\n"
           "    --no-read     don't read anything on the D-Bus socket\n"
           "\n"
           "    --session     use the session bus (default)\n"
           "    --system      use the system bus\n"
           );
  exit (exit_with);
}

static DBusHandlerResult
filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  DBusMessage *reply;

  if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  if (sleep_ms > 0)
    {
      _dbus_sleep_milliseconds (sleep_ms);
    }

  if (!noreply)
    {
      reply = dbus_message_new_method_return (message);

      if (reply == NULL)
        tool_oom ("allocating reply");

      if (!dbus_connection_send (connection, reply, NULL))
        tool_oom ("sending reply");

      dbus_message_unref (reply);
    }

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusConnection *
init_connection (DBusBusType type, const char *name)
{
  DBusConnection *connection;
  DBusError error = DBUS_ERROR_INIT;

  connection = dbus_bus_get (type, &error);

  if (connection == NULL)
    {
      fprintf (stderr, "Failed to connect to bus: %s: %s\n",
               error.name, error.message);
      dbus_error_free (&error);
      exit (1);
    }

  if (name != NULL)
    {
      if (dbus_bus_request_name (connection, name, DBUS_NAME_FLAG_DO_NOT_QUEUE,
                                 NULL) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
        {
          fprintf (stderr, "failed to take bus name %s\n", name);
          exit (1);
        }
    }
  else
    {
      printf ("%s\n", dbus_bus_get_unique_name (connection));
    }

  if (!dbus_connection_add_filter (connection, filter, NULL, NULL))
    tool_oom ("adding message filter");

  return connection;
}

int
dbus_test_tool_echo (int argc, char **argv)
{
  DBusConnection *connection;
  DBusBusType type = DBUS_BUS_SESSION;
  int i;
  const char *name = NULL;

  /* argv[1] is the tool name, so start from 2 */

  for (i = 2; i < argc; i++)
    {
      const char *arg = argv[i];

      if (strcmp (arg, "--system") == 0)
        {
          type = DBUS_BUS_SYSTEM;
        }
      else if (strcmp (arg, "--session") == 0)
        {
          type = DBUS_BUS_SESSION;
        }
      else if (strstr (arg, "--name=") == arg)
        {
          name = arg + strlen ("--name=");
        }
      else if (strstr (arg, "--sleep-ms=") == arg)
        {
          sleep_ms = atoi (arg + strlen ("--sleep-ms="));
        }
      else
        {
          usage_echo (2);
        }
    }

  connection = init_connection (type, name);

  while (dbus_connection_read_write_dispatch (connection, -1))
    {}

  dbus_connection_unref (connection);
  return 0;
}

int
dbus_test_tool_black_hole (int argc, char **argv)
{
  DBusConnection *connection;
  DBusBusType type = DBUS_BUS_SESSION;
  int i;
  const char *name = NULL;

  /* argv[1] is the tool name, so start from 2 */

  for (i = 2; i < argc; i++)
    {
      const char *arg = argv[i];

      if (strcmp (arg, "--system") == 0)
        {
          type = DBUS_BUS_SYSTEM;
        }
      else if (strcmp (arg, "--session") == 0)
        {
          type = DBUS_BUS_SESSION;
        }
      else if (strstr (arg, "--name=") == arg)
        {
          name = arg + strlen ("--name=");
        }
      else if (strcmp (arg, "--no-read") == 0)
        {
          noread = TRUE;
        }
      else
        {
          usage_black_hole (2);
        }
    }

  connection = init_connection (type, name);

  if (noread)
    {
      while (1)
        _dbus_sleep_milliseconds (3600);
    }

  noreply = TRUE;

  while (dbus_connection_read_write_dispatch (connection, -1))
    {}

  dbus_connection_unref (connection);
  return 0;
}
