/* Regression test for passing unmodified messages between connections
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2010-2011 Nokia Corporation
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

#include <config.h>

#include <glib.h>

#include <dbus/dbus.h>

#include "test-utils-glib.h"

/* This is basically a miniature dbus-daemon. We relay messages from the client
 * on the left to the client on the right.
 *
 * left      socket     left      dispatch     right    socket     right
 * client ===========>  server --------------> server ===========> client
 * conn                 conn                   conn                conn
 *
 * In the real dbus-daemon, the client connections would be out-of-process,
 * but here we're cheating and doing everything in-process.
 */

typedef struct {
    TestMainContext *ctx;
    DBusError e;
    gboolean skip;

    DBusServer *server;

    DBusConnection *left_client_conn;
    DBusConnection *left_server_conn;

    DBusConnection *right_server_conn;
    DBusConnection *right_client_conn;
    /* queue of DBusMessage received by right_client_conn */
    GQueue messages;
} Fixture;

static void
assert_no_error (const DBusError *e)
{
  if (G_UNLIKELY (dbus_error_is_set (e)))
    g_error ("expected success but got error: %s: %s", e->name, e->message);
}

static DBusHandlerResult
server_message_cb (DBusConnection *server_conn,
    DBusMessage *message,
    void *data)
{
  Fixture *f = data;

  g_assert (server_conn == f->left_server_conn);
  g_assert (f->right_server_conn != NULL);

  dbus_connection_send (f->right_server_conn, message, NULL);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
right_client_message_cb (DBusConnection *client_conn,
    DBusMessage *message,
    void *data)
{
  Fixture *f = data;

  g_assert (client_conn == f->right_client_conn);
  g_queue_push_tail (&f->messages, dbus_message_ref (message));

  return DBUS_HANDLER_RESULT_HANDLED;
}

static void
new_conn_cb (DBusServer *server,
    DBusConnection *server_conn,
    void *data)
{
  Fixture *f = data;
  dbus_bool_t have_mem;

  if (f->left_server_conn == NULL)
    {
      f->left_server_conn = dbus_connection_ref (server_conn);

      have_mem = dbus_connection_add_filter (server_conn,
          server_message_cb, f, NULL);
      g_assert (have_mem);
    }
  else
    {
      g_assert (f->right_server_conn == NULL);
      f->right_server_conn = dbus_connection_ref (server_conn);
    }

  test_connection_setup (f->ctx, server_conn);
}

static void
setup (Fixture *f,
       gconstpointer address)
{
  test_timeout_reset (1);

  f->ctx = test_main_context_get ();
  dbus_error_init (&f->e);
  g_queue_init (&f->messages);

  if ((g_str_has_prefix (address, "tcp:") ||
       g_str_has_prefix (address, "nonce-tcp:")) &&
      !test_check_tcp_works ())
    {
      f->skip = TRUE;
      return;
    }

  f->server = dbus_server_listen (address, &f->e);
  assert_no_error (&f->e);
  g_assert (f->server != NULL);

  dbus_server_set_new_connection_function (f->server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->server);
}

static void
test_connect (Fixture *f,
    gconstpointer data G_GNUC_UNUSED)
{
  dbus_bool_t have_mem;
  char *address;

  if (f->skip)
    return;

  g_assert (f->left_server_conn == NULL);
  g_assert (f->right_server_conn == NULL);

  address = dbus_server_get_address (f->server);
  g_assert (address != NULL);

  f->left_client_conn = dbus_connection_open_private (address, &f->e);
  assert_no_error (&f->e);
  g_assert (f->left_client_conn != NULL);
  test_connection_setup (f->ctx, f->left_client_conn);

  while (f->left_server_conn == NULL)
    {
      test_progress ('.');
      test_main_context_iterate (f->ctx, TRUE);
    }

  f->right_client_conn = dbus_connection_open_private (address, &f->e);
  assert_no_error (&f->e);
  g_assert (f->right_client_conn != NULL);
  test_connection_setup (f->ctx, f->right_client_conn);

  dbus_free (address);

  while (f->right_server_conn == NULL)
    {
      test_progress ('.');
      test_main_context_iterate (f->ctx, TRUE);
    }

  have_mem = dbus_connection_add_filter (f->right_client_conn,
      right_client_message_cb, f, NULL);
  g_assert (have_mem);
}

static dbus_uint32_t
send_one (Fixture *f,
    const char *member)
{
  dbus_bool_t have_mem;
  dbus_uint32_t serial;
  DBusMessage *outgoing;

  outgoing = dbus_message_new_signal ("/com/example/Hello",
      "com.example.Hello", member);
  g_assert (outgoing != NULL);

  have_mem = dbus_connection_send (f->left_client_conn, outgoing, &serial);
  g_assert (have_mem);
  g_assert (serial != 0);

  dbus_message_unref (outgoing);
  return serial;
}

static void
test_relay (Fixture *f,
    gconstpointer data)
{
  DBusMessage *incoming;

  if (f->skip)
    return;

  test_connect (f, data);

  send_one (f, "First");
  send_one (f, "Second");

  while (g_queue_get_length (&f->messages) < 2)
    {
      test_progress ('.');
      test_main_context_iterate (f->ctx, TRUE);
    }

  g_assert_cmpuint (g_queue_get_length (&f->messages), ==, 2);

  incoming = g_queue_pop_head (&f->messages);
  g_assert_cmpstr (dbus_message_get_member (incoming), ==, "First");
  dbus_message_unref (incoming);

  incoming = g_queue_pop_head (&f->messages);
  g_assert_cmpstr (dbus_message_get_member (incoming), ==, "Second");
  dbus_message_unref (incoming);
}

/* An arbitrary number of messages */
#define MANY 8192

static void
test_limit (Fixture *f,
    gconstpointer data)
{
  DBusMessage *incoming;
  guint i;

  if (f->skip)
    return;

  test_connect (f, data);

  /* This was an attempt to reproduce fd.o #34393. It didn't work. */
  g_test_bug ("34393");
  dbus_connection_set_max_received_size (f->left_server_conn, 1);
  test_main_context_iterate (f->ctx, TRUE);

  for (i = 0; i < MANY; i++)
    {
      gchar *buf = g_strdup_printf ("Message%u", i);

      send_one (f, buf);
      g_free (buf);
    }

  i = 0;

  while (i < MANY)
    {
      while (g_queue_is_empty (&f->messages))
        {
          test_main_context_iterate (f->ctx, TRUE);
        }

      while ((incoming = g_queue_pop_head (&f->messages)) != NULL)
        {
          i++;
          dbus_message_unref (incoming);
        }
    }
}

static void
teardown (Fixture *f,
    gconstpointer data G_GNUC_UNUSED)
{
  if (f->left_client_conn != NULL)
    {
      test_connection_shutdown(NULL, f->left_client_conn);
      dbus_connection_close (f->left_client_conn);
      dbus_connection_unref (f->left_client_conn);
      f->left_client_conn = NULL;
    }

  if (f->right_client_conn != NULL)
    {
      test_connection_shutdown(NULL, f->right_client_conn);
      dbus_connection_close (f->right_client_conn);
      dbus_connection_unref (f->right_client_conn);
      f->right_client_conn = NULL;
    }

  if (f->left_server_conn != NULL)
    {
      test_connection_shutdown(NULL, f->left_server_conn);
      dbus_connection_close (f->left_server_conn);
      dbus_connection_unref (f->left_server_conn);
      f->left_server_conn = NULL;
    }

  if (f->right_server_conn != NULL)
    {
      test_connection_shutdown(NULL, f->right_server_conn);
      dbus_connection_close (f->right_server_conn);
      dbus_connection_unref (f->right_server_conn);
      f->right_server_conn = NULL;
    }

  if (f->server != NULL)
    {
      test_server_shutdown (f->ctx, f->server);
      dbus_server_unref (f->server);
      f->server = NULL;
    }

  test_main_context_unref (f->ctx);
}

int
main (int argc,
    char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/connect/tcp", Fixture, "tcp:host=127.0.0.1", setup,
      test_connect, teardown);
  g_test_add ("/relay/tcp", Fixture, "tcp:host=127.0.0.1", setup,
      test_relay, teardown);
  g_test_add ("/limit/tcp", Fixture, "tcp:host=127.0.0.1", setup,
      test_limit, teardown);

#ifdef DBUS_UNIX
  g_test_add ("/connect/unix", Fixture, "unix:tmpdir=/tmp", setup,
      test_connect, teardown);
  g_test_add ("/relay/unix", Fixture, "unix:tmpdir=/tmp", setup,
      test_relay, teardown);
  g_test_add ("/limit/unix", Fixture, "unix:tmpdir=/tmp", setup,
      test_limit, teardown);
#endif

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
