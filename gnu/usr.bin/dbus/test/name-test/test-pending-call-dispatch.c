/*
 * Copyright © 2006 Red Hat Inc.
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
* Test to make sure we don't get stuck polling a dbus connection
* which has no data on the socket.  This was an issue where
* one pending call would read all the data off the bus
* and the second pending call would not check to see
* if its data had already been read before polling the connection
* and blocking.
**/

#include <config.h>
#include <dbus/dbus.h>
#include <dbus/dbus-sysdeps.h>
#include <dbus/dbus-valgrind-internal.h>
#include <stdio.h>
#include <stdlib.h>

static void
_run_iteration (DBusConnection *conn)
{
  DBusPendingCall *echo_pending;
  DBusPendingCall *dbus_pending;
  DBusMessage *method;
  DBusMessage *reply;
  const char *echo = "echo";

  /* send the first message */
  method = dbus_message_new_method_call ("org.freedesktop.DBus.TestSuiteEchoService",
                                         "/org/freedesktop/TestSuite",
                                         "org.freedesktop.TestSuite",
                                         "Echo");

  if (!dbus_message_append_args (method, DBUS_TYPE_STRING, &echo, NULL))
    {
      fprintf (stderr, "Bail out! Failed to append arguments: OOM\n");
      exit (1);
    }

  dbus_connection_send_with_reply (conn, method, &echo_pending, -1);
  dbus_message_unref (method);
  
  /* send the second message */
  method = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
                                         DBUS_PATH_DBUS,
                                         "org.freedesktop.Introspectable",
                                         "Introspect");

  dbus_connection_send_with_reply (conn, method, &dbus_pending, -1);
  dbus_message_unref (method);

  /* block on the second message (should return immediately) */
  dbus_pending_call_block (dbus_pending);

  /* block on the first message */
  /* if it does not return immediately chances 
     are we hit the block in poll bug */
  dbus_pending_call_block (echo_pending);

  /* check the reply only to make sure we
     are not getting errors unrelated
     to the block in poll bug */
  reply = dbus_pending_call_steal_reply (echo_pending);

  if (reply == NULL)
    {
      printf ("Bail out! Reply is NULL ***\n");
      exit (1);
    }

  if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_ERROR)
    {
      printf ("Bail out! Reply is error: %s ***\n", dbus_message_get_error_name (reply));
      exit (1);
    } 

  dbus_message_unref (reply);
  dbus_pending_call_unref (dbus_pending);
  dbus_pending_call_unref (echo_pending);
  
}

/* This test outputs TAP syntax: http://testanything.org/ */
int
main (int argc, char *argv[])
{
  long start_tv_sec, start_tv_usec;
  long end_tv_sec, end_tv_usec;
  int i;
  DBusMessage *method;
  DBusConnection *conn;
  DBusError error;

  if (RUNNING_ON_VALGRIND)
    {
      printf ("1..0 # SKIP Not ready to run under valgrind yet\n");
      return 0;
    }

  /* Time each iteration and make sure it doesn't take more than 5 seconds
     to complete.  Outside influences may cause connections to take longer
     but if it does and we are stuck in a poll call then we know the 
     stuck in poll bug has come back to haunt us */

  printf ("# Testing stuck in poll\n");

  dbus_error_init (&error);

  conn = dbus_bus_get (DBUS_BUS_SESSION, &error);

  /* run 100 times to make sure */
  for (i = 0; i < 100; i++)
    {
      long delta;
      
      _dbus_get_monotonic_time (&start_tv_sec, &start_tv_usec);
      _run_iteration (conn);
      _dbus_get_monotonic_time (&end_tv_sec, &end_tv_usec);

      /* we just care about seconds */
      delta = end_tv_sec - start_tv_sec;
      printf ("ok %d - %lis\n", i + 1, delta);
      if (delta >= 5)
        {
	  printf ("Bail out! Looks like we might have been be stuck in poll ***\n");
	  exit (1);
	}
    }
 
  method = dbus_message_new_method_call ("org.freedesktop.TestSuiteEchoService",
                                         "/org/freedesktop/TestSuite",
                                         "org.freedesktop.TestSuite",
                                         "Exit");
  dbus_connection_send (conn, method, NULL);
  dbus_message_unref (method);

  printf ("# Testing completed\n1..%d\n", i);
  exit (0);
}
