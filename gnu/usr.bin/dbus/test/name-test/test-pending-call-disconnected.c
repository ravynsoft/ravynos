/*
 * Copyright © 2006 Red Hat Inc.
 * Copyright © 2017 Shin-ichi MORITA <shin1morita@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
* Test to make sure that pending calls unref error messages
* when blocked after disconnected.
**/

#include <config.h>
#include <dbus/dbus.h>
#include <dbus/dbus-sysdeps.h>
#include <dbus/dbus-valgrind-internal.h>
#include <stdio.h>
#include <stdlib.h>

static size_t count = 0;

static void
free_data (void *data)
{
  --count;
  printf ("# Freed: %s\n", (const char*)data);
}

/* This test outputs TAP syntax: http://testanything.org/ */
int
main (int argc, char *argv[])
{
  dbus_int32_t slot_connection = -1;
  dbus_int32_t slot_message = -1;
  dbus_int32_t slot_pending = -1;
  DBusError error;
  DBusConnection *conn;
  DBusMessage *method;
  DBusPendingCall *pending;
  DBusMessage *reply;

  if (RUNNING_ON_VALGRIND)
    {
      printf ("1..0 # SKIP Not ready to run under valgrind yet\n");
      return 0;
    }

  printf ("# Testing pending call error\n");

  dbus_connection_allocate_data_slot (&slot_connection);
  dbus_message_allocate_data_slot (&slot_message);
  dbus_pending_call_allocate_data_slot (&slot_pending);

  dbus_error_init (&error);
  conn = dbus_bus_get_private (DBUS_BUS_SESSION, &error);
  dbus_connection_set_data (conn, slot_connection, (void*)"connection", free_data);
  ++count;
  dbus_connection_set_exit_on_disconnect (conn, FALSE);

  method = dbus_message_new_method_call ("org.freedesktop.TestSuiteEchoService",
                                         "/org/freedesktop/TestSuite",
                                         "org.freedesktop.TestSuite",
                                         "Exit");
  dbus_message_set_data (method, slot_message, (void*)"method", free_data);
  ++count;

  dbus_connection_send_with_reply (conn, method, &pending, -1);
  dbus_message_unref (method);
  dbus_pending_call_set_data (pending, slot_pending, (void*)"pending", free_data);
  ++count;

  dbus_connection_close (conn);

  dbus_pending_call_block (pending);
  reply = dbus_pending_call_steal_reply (pending);
  dbus_pending_call_unref (pending);
  if (reply == NULL)
    {
      printf ("Bail out! Reply is NULL ***\n");
      exit (1);
    }
  dbus_message_set_data (reply, slot_message, (void*)"reply", free_data);
  ++count;
  if (dbus_message_get_type (reply) != DBUS_MESSAGE_TYPE_ERROR)
    {
      printf ("Bail out! Reply is not error ***\n");
      exit (1);
    }
  dbus_message_unref (reply);

  dbus_connection_unref (conn);

  dbus_connection_free_data_slot (&slot_connection);
  dbus_message_free_data_slot (&slot_message);
  dbus_pending_call_free_data_slot (&slot_pending);

  if (count != 0)
    {
      printf ("not ok # Not all refs were unrefed ***\n");
      exit (1);
    }
  else
    {
      printf ("ok\n# Testing completed\n1..1\n");
      exit (0);
    }
}
