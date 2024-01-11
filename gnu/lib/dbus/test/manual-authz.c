/* Simple sanity-check for authentication and authorization.
 *
 * Copyright © 2010-2011 Nokia Corporation
 * Copyright © 2012 Collabora Ltd.
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

#include <stdlib.h>
#ifdef G_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#endif

#include "test-utils.h"

typedef struct {
    DBusError e;
    TestMainContext *ctx;

    DBusServer *normal_server;
    DBusServer *anon_allowed_server;
    DBusServer *anon_only_server;
    DBusServer *anon_mech_only_server;
    DBusServer *anon_disallowed_server;
    DBusServer *permissive_server;
    DBusServer *unhappy_server;
    DBusServer *same_uid_server;
    DBusServer *same_uid_or_anon_server;
} Fixture;

static void oom (void) G_GNUC_NORETURN;
static void
oom (void)
{
  g_error ("out of memory");
  abort ();
}

static void
assert_no_error (const DBusError *e)
{
  if (G_UNLIKELY (dbus_error_is_set (e)))
    g_error ("expected success but got error: %s: %s", e->name, e->message);
}

static DBusHandlerResult
server_message_cb (DBusConnection *conn,
    DBusMessage *message,
    void *data)
{
  if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected"))
    {
      dbus_connection_unref (conn);

      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

  if (dbus_message_get_type (message) == DBUS_MESSAGE_TYPE_METHOD_CALL)
    {
      DBusMessage *reply = dbus_message_new_method_return (message);
      const char *hello = "Hello, world!";
      unsigned long uid;
      char *sid;

      if (dbus_connection_get_unix_user (conn, &uid))
        {
          g_message ("message from uid %lu", uid);
        }
      else if (dbus_connection_get_windows_user (conn, &sid))
        {
          if (sid == NULL)
            oom ();

          g_message ("message from sid \"%s\"", sid);
          dbus_free (sid);
        }
      else if (dbus_connection_get_is_anonymous (conn))
        {
          g_message ("message from Anonymous");
        }
      else
        {
          g_message ("message from ... someone?");
        }

      if (reply == NULL)
        oom ();

      if (!dbus_message_append_args (reply,
            DBUS_TYPE_STRING, &hello,
            DBUS_TYPE_INVALID))
        oom ();

      if (!dbus_connection_send (conn, reply, NULL))
        oom ();

      dbus_message_unref (reply);

      return DBUS_HANDLER_RESULT_HANDLED;
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static dbus_bool_t
permissive_unix_func (DBusConnection *conn,
    unsigned long uid,
    void *data)
{
  g_message ("accepting Unix user %lu", uid);
  return TRUE;
}

static dbus_bool_t
permissive_win_func (DBusConnection *conn,
    const char *sid,
    void *data)
{
  g_message ("accepting Windows user \"%s\"", sid);
  return TRUE;
}

static dbus_bool_t
broken_unix_func (DBusConnection *conn,
    unsigned long uid,
    void *data)
{
  g_error ("libdbus called the Unix user function for an ANONYMOUS-only "
      "connection");
  return FALSE;
}

static dbus_bool_t
broken_win_func (DBusConnection *conn,
    const char *sid,
    void *data)
{
  g_error ("libdbus called the Windows user function for an ANONYMOUS-only "
      "connection");
  return FALSE;
}

static dbus_bool_t
unhappy_unix_func (DBusConnection *conn,
    unsigned long uid,
    void *data)
{
  g_message ("rejecting Unix user %lu", uid);
  return FALSE;
}

static dbus_bool_t
unhappy_win_func (DBusConnection *conn,
    const char *sid,
    void *data)
{
  g_message ("rejecting Windows user \"%s\"", sid);
  return FALSE;
}

static dbus_bool_t
same_uid_unix_func (DBusConnection *conn,
    unsigned long uid,
    void *data)
{
  g_message ("checking whether Unix user %lu owns this process", uid);
  /* I'd use _dbus_unix_user_is_process_owner(), but it's private... */
#ifdef G_OS_UNIX
  return (geteuid () == uid);
#else
  return FALSE;
#endif
}

static dbus_bool_t
same_uid_win_func (DBusConnection *conn,
    const char *sid,
    void *data)
{
  g_message ("checking whether Windows user \"%s\" owns this process", sid);
  g_message ("Stub implementation consistent with dbus-sysdeps-util-win: "
      "assume they do");
  return TRUE;
}

static void
new_conn_cb (DBusServer *server,
    DBusConnection *conn,
    void *data)
{
  Fixture *f = data;

  dbus_connection_ref (conn);
  test_connection_setup (f->ctx, conn);

  if (!dbus_connection_add_filter (conn, server_message_cb, f, NULL))
    oom ();

  if (server == f->normal_server)
    {
    }
  else if (server == f->anon_allowed_server)
    {
      dbus_connection_set_allow_anonymous (conn, TRUE);
    }
  else if (server == f->anon_only_server)
    {
      dbus_connection_set_allow_anonymous (conn, TRUE);

      dbus_connection_set_unix_user_function (conn, unhappy_unix_func,
          f, NULL);
      dbus_connection_set_windows_user_function (conn, unhappy_win_func,
          f, NULL);
    }
  else if (server == f->anon_mech_only_server)
    {
      dbus_connection_set_allow_anonymous (conn, TRUE);

      /* should never get called */
      dbus_connection_set_unix_user_function (conn, broken_unix_func,
          f, NULL);
      dbus_connection_set_windows_user_function (conn, broken_win_func,
          f, NULL);
    }
  else if (server == f->anon_disallowed_server)
    {
      dbus_connection_set_allow_anonymous (conn, FALSE);

      /* should never get called */
      dbus_connection_set_unix_user_function (conn, broken_unix_func,
          f, NULL);
      dbus_connection_set_windows_user_function (conn, broken_win_func,
          f, NULL);
    }
  else if (server == f->permissive_server)
    {
      dbus_connection_set_unix_user_function (conn, permissive_unix_func,
          f, NULL);
      dbus_connection_set_windows_user_function (conn, permissive_win_func,
          f, NULL);
    }
  else if (server == f->unhappy_server)
    {
      dbus_connection_set_unix_user_function (conn, unhappy_unix_func,
          f, NULL);
      dbus_connection_set_windows_user_function (conn, unhappy_win_func,
          f, NULL);
    }
  else if (server == f->same_uid_server)
    {
      dbus_connection_set_unix_user_function (conn, same_uid_unix_func,
          f, NULL);
      dbus_connection_set_windows_user_function (conn, same_uid_win_func,
          f, NULL);
    }
  else if (server == f->same_uid_or_anon_server)
    {
      dbus_connection_set_allow_anonymous (conn, TRUE);

      dbus_connection_set_unix_user_function (conn, same_uid_unix_func,
          f, NULL);
      dbus_connection_set_windows_user_function (conn, same_uid_win_func,
          f, NULL);
    }
  else
    {
      g_assert_not_reached ();
    }
}

static void
setup (Fixture *f,
    const gchar *listen_addr)
{
  const char *only_anon[] = { "ANONYMOUS", NULL };
  char *connect_addr;

  f->normal_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->normal_server != NULL);
  dbus_server_set_new_connection_function (f->normal_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->normal_server);
  connect_addr = dbus_server_get_address (f->normal_server);
  g_message ("Normal server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->anon_allowed_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->anon_allowed_server != NULL);
  dbus_server_set_new_connection_function (f->anon_allowed_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->anon_allowed_server);
  connect_addr = dbus_server_get_address (f->anon_allowed_server);
  g_message ("Anonymous-allowed server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->anon_only_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->anon_only_server != NULL);
  dbus_server_set_new_connection_function (f->anon_only_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->anon_only_server);
  connect_addr = dbus_server_get_address (f->anon_only_server);
  g_message ("Anonymous-only server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->anon_mech_only_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->anon_mech_only_server != NULL);
  dbus_server_set_auth_mechanisms (f->anon_mech_only_server, only_anon);
  dbus_server_set_new_connection_function (f->anon_mech_only_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->anon_mech_only_server);
  connect_addr = dbus_server_get_address (f->anon_mech_only_server);
  g_message ("Anon mech only server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->anon_disallowed_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->anon_disallowed_server != NULL);
  dbus_server_set_auth_mechanisms (f->anon_disallowed_server, only_anon);
  dbus_server_set_new_connection_function (f->anon_disallowed_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->anon_disallowed_server);
  connect_addr = dbus_server_get_address (f->anon_disallowed_server);
  g_message ("Anonymous-disallowed server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->permissive_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->permissive_server != NULL);
  dbus_server_set_new_connection_function (f->permissive_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->permissive_server);
  connect_addr = dbus_server_get_address (f->permissive_server);
  g_message ("Permissive server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->unhappy_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->unhappy_server != NULL);
  dbus_server_set_new_connection_function (f->unhappy_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->unhappy_server);
  connect_addr = dbus_server_get_address (f->unhappy_server);
  g_message ("Unhappy server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->same_uid_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->same_uid_server != NULL);
  dbus_server_set_new_connection_function (f->same_uid_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->same_uid_server);
  connect_addr = dbus_server_get_address (f->same_uid_server);
  g_message ("Same-UID server:\n%s", connect_addr);
  dbus_free (connect_addr);

  f->same_uid_or_anon_server = dbus_server_listen (listen_addr, &f->e);
  assert_no_error (&f->e);
  g_assert (f->same_uid_or_anon_server != NULL);
  dbus_server_set_new_connection_function (f->same_uid_or_anon_server,
      new_conn_cb, f, NULL);
  test_server_setup (f->ctx, f->same_uid_or_anon_server);
  connect_addr = dbus_server_get_address (f->same_uid_or_anon_server);
  g_message ("Same-UID-or-anon server:\n%s", connect_addr);
  dbus_free (connect_addr);
}

int
main (int argc,
    char **argv)
{
  Fixture f = { DBUS_ERROR_INIT, test_main_context_get () };

  if (argc >= 2)
    setup (&f, argv[1]);
  else
    setup (&f, "tcp:host=127.0.0.1");

  for (;;)
    test_main_context_iterate (f.ctx, TRUE);

  /* never returns */
}
