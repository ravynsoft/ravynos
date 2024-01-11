/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Regression test for fd.o #102839: blocking on pending calls in threads
 *
 * Copyright © 2018 KPIT Technologies Ltd.
 * Copyright © 2018 Manish Narang <manrock007@gmail.com>
 * Copyright © 2018 Collabora Ltd.
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
 */

#include <config.h>

#include <dbus/dbus.h>

#include <glib.h>
#include <gio/gio.h>

#include "test-utils-glib.h"

typedef struct
{
  DBusError e;
  GError *ge;
  GPid daemon_pid;
  gchar *address;

  DBusConnection *service_conn;
  GThread *service_thread;
  const gchar *service_name;

  GThread *client_dispatcher_thread;
  GThread **client_caller_threads;
  DBusConnection *client_conn;
  GMutex callers_remaining_mutex;
  GCond callers_remaining_cond;
  gsize callers_remaining;

  gsize n_caller_threads;
  gsize calls_per_thread;
} Fixture;

static DBusHandlerResult
echo_filter (DBusConnection *connection,
             DBusMessage *message,
             void *user_data)
{
  DBusMessage *reply;

  if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  reply = dbus_message_new_method_return (message);

  if (reply == NULL ||
      !dbus_connection_send (connection, reply, NULL))
    g_error ("OOM");

  dbus_clear_message (&reply);

  return DBUS_HANDLER_RESULT_HANDLED;
}

/*
 * Thread function to dispatch the service connection. This function runs
 * in its own thread, dispatching the service connection and replying to
 * method calls from the callers, until the service connection is
 * disconnected by the main thread.
 */
static gpointer
service_thread_cb (gpointer user_data)
{
  Fixture *f = user_data;

  /* In principle we should be able to wait indefinitely (-1) but that
   * seems to prevent closing the connection from the main thread from
   * having its desired effect */
  while (dbus_connection_read_write_dispatch (f->service_conn, 1000))
    {
      /* Disconnected message not received; keep processing */
    }

  return NULL;
}

/*
 * Much like service_thread_cb, but for the client connection. In a real
 * program this would often be the main thread, running an event loop
 * like the one provided by GLib.
 */
static gpointer
client_dispatcher_thread_cb (gpointer user_data)
{
  Fixture *f = user_data;

  /* This thread needs to yield occasionally, otherwise the caller
   * threads won't ever get a chance to send their messages.
   * This is not a recommended I/O pattern, but in principle it was
   * always meant to work... */
  while (dbus_connection_read_write_dispatch (f->client_conn, 10))
    {
      /* Disconnected message not received; keep processing */
    }

  return NULL;
}

/*
 * Worker thread representing some background task. Some programs
 * dispatch a DBusConnection in one thread (in this test that's the
 * "client dispatcher thread"), and expect to be able to block on
 * pending calls in other threads. This represents one of those other
 * threads.
 */
static gpointer
client_caller_thread_cb (gpointer user_data)
{
  Fixture *f = user_data;
  gsize i;

  for (i = 0; i < f->calls_per_thread; i++)
    {
      DBusMessage *call, *reply;
      DBusError error = DBUS_ERROR_INIT;
      gint64 time_before, time_after;

      /* This deliberately isn't g_test_message() to avoid buffering
       * issues: stderr is line-buffered */
      if (i % 100 == 0)
        g_printerr ("# thread %p: %" G_GSIZE_FORMAT "/%" G_GSIZE_FORMAT "\n",
                        g_thread_self (), i, f->calls_per_thread);

      call = dbus_message_new_method_call (f->service_name, "/",
                                           "com.example.Echo", "Echo");
      g_assert_nonnull (call);

      time_before = g_get_monotonic_time ();
      reply = dbus_connection_send_with_reply_and_block (f->client_conn, call,
                                                         30000, &error);
      time_after = g_get_monotonic_time ();
      test_assert_no_error (&error);
      g_assert_nonnull (reply);
      /* We don't expect it to have taken long - a few seconds max, even
       * with all the other threads contending with us. If we were
       * anywhere near the 30 second timeout then that's a failure. */
      g_assert_cmpint (time_after - time_before, <=, 10 * G_USEC_PER_SEC);
      dbus_clear_message (&reply);
      dbus_clear_message (&call);
    }

  g_printerr ("# thread %p: finishing\n", g_thread_self ());
  g_mutex_lock (&f->callers_remaining_mutex);
  f->callers_remaining--;
  g_cond_signal (&f->callers_remaining_cond);
  g_mutex_unlock (&f->callers_remaining_mutex);

  return NULL;
}

static void
setup (Fixture *f,
       gconstpointer context)
{
  f->ge = NULL;
  dbus_error_init (&f->e);

  f->address = test_get_dbus_daemon (NULL, TEST_USER_ME, NULL, &f->daemon_pid);
  g_assert_nonnull (f->address);

  f->service_conn = test_connect_to_bus (NULL, f->address);
  f->service_name = dbus_bus_get_unique_name (f->service_conn);
  f->client_conn = test_connect_to_bus (NULL, f->address);

  if (!dbus_connection_add_filter (f->service_conn, echo_filter, f, NULL))
    g_error ("OOM");

  f->service_thread = g_thread_new ("service", service_thread_cb, f);
  f->client_dispatcher_thread = g_thread_new ("client dispatcher",
                                              client_dispatcher_thread_cb, f);
}

/*
 * Assert that in the following situation:
 *
 * - one thread dispatches the server connection (in real life this would
 *   normally be a separate process, of course)
 * - one thread dispatches the client connection
 * - many threads make blocking method calls on the same client connection
 *
 * the caller threads are regularly dispatched, and never get blocked
 * until their method call timeout.
 */
static void
test_threads (Fixture *f,
              gconstpointer context)
{
  gsize i;

  g_test_bug ("102839");

  if (g_test_slow ())
    {
      test_timeout_reset (10);
      f->n_caller_threads = 20;
      f->calls_per_thread = 10000;
    }
  else
    {
      test_timeout_reset (1);
      f->n_caller_threads = 4;
      f->calls_per_thread = 1000;
    }

  f->client_caller_threads = g_new0 (GThread *, f->n_caller_threads);
  f->callers_remaining = f->n_caller_threads;

  /* Start the caller threads off */

  for (i = 0; i < f->n_caller_threads; i++)
    {
      gchar *name = g_strdup_printf ("client %" G_GSIZE_FORMAT, i);

      f->client_caller_threads[i] = g_thread_new (name,
                                                  client_caller_thread_cb,
                                                  f);

      g_free (name);
    }

  /* Wait for all caller threads to exit */

  g_mutex_lock (&f->callers_remaining_mutex);

  while (f->callers_remaining > 0)
    g_cond_wait (&f->callers_remaining_cond, &f->callers_remaining_mutex);

  g_mutex_unlock (&f->callers_remaining_mutex);

  for (i = 0; i < f->n_caller_threads; i++)
    g_thread_join (g_steal_pointer (&f->client_caller_threads[i]));

  /* If we haven't crashed out yet, then we're good! */
}

static void
teardown (Fixture *f,
          gconstpointer context G_GNUC_UNUSED)
{
  dbus_error_free (&f->e);
  g_clear_error (&f->ge);

  if (f->client_conn != NULL)
    dbus_connection_close (f->client_conn);

  if (f->service_conn != NULL)
    {
      dbus_connection_remove_filter (f->service_conn, echo_filter, f);
      dbus_connection_close (f->service_conn);
    }

  /* Now that the connections have been closed, the threads will exit */
  if (f->service_thread != NULL)
    g_thread_join (g_steal_pointer (&f->service_thread));

  if (f->client_dispatcher_thread != NULL)
    g_thread_join (g_steal_pointer (&f->client_dispatcher_thread));

  dbus_clear_connection (&f->service_conn);
  dbus_clear_connection (&f->client_conn);

  if (f->daemon_pid != 0)
    {
      test_kill_pid (f->daemon_pid);
      g_spawn_close_pid (f->daemon_pid);
      f->daemon_pid = 0;
    }

  g_free (f->address);
}

int
main (int argc,
    char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/thread-blocking", Fixture, NULL, setup, test_threads,
              teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
