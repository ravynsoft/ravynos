/* Integration tests for the dbus-daemon
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2008 Red Hat, Inc.
 * Copyright © 2010-2011 Nokia Corporation
 * Copyright © 2015-2018 Collabora Ltd.
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

#include <errno.h>
#include <string.h>

#include <dbus/dbus.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include "bus/stats.h"
#include "test-utils-glib.h"

#include <string.h>

#ifdef DBUS_UNIX
# include <pwd.h>
# include <unistd.h>
# include <stdlib.h>
# include <search.h>
# include <sys/types.h>

# ifdef HAVE_GIO_UNIX
    /* The CMake build system doesn't know how to check for this yet */
#   include <gio/gunixfdmessage.h>
# endif

# ifdef HAVE_SYS_RESOURCE_H
#   include <sys/resource.h>
# endif

# ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
# endif
#endif

/* Platforms where we know that credentials-passing passes both the
 * uid and the pid. Please keep these in alphabetical order.
 *
 * These platforms should #error in _dbus_read_credentials_socket()
 * if we didn't detect their flavour of credentials-passing, since that
 * would be a regression.
 */
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || \
  defined(__linux__) || \
  defined(__NetBSD__) || \
  defined(__OpenBSD__)
# define UNIX_USER_SHOULD_WORK
# define PID_SHOULD_WORK
#endif

/* Platforms where we know that credentials-passing passes the
 * uid, but not necessarily the pid. Again, alphabetical order please.
 *
 * These platforms should also #error in _dbus_read_credentials_socket()
 * if we didn't detect their flavour of credentials-passing.
 */
#if 0 /* defined(__your_platform_here__) */
# define UNIX_USER_SHOULD_WORK
#endif

typedef struct {
    gboolean skip;

    TestMainContext *ctx;

    DBusError e;
    GError *ge;

    GPid daemon_pid;
    gchar *address;

    DBusConnection *left_conn;
    gboolean left_conn_shouted_signal_filter;

    DBusConnection *right_conn;
    GQueue held_messages;
    gboolean right_conn_echo;
    gboolean right_conn_hold;
    gboolean wait_forever_called;
    guint activation_forking_counter;
    guint signal_counter;

    gchar *tmp_runtime_dir;
    gchar *saved_runtime_dir;
} Fixture;

static DBusHandlerResult
echo_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  Fixture *f = user_data;
  DBusMessage *reply;

  if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  /* WaitForever() never replies, emulating a service that has got stuck */
  if (dbus_message_is_method_call (message, "com.example", "WaitForever"))
    {
      f->wait_forever_called = TRUE;
      return DBUS_HANDLER_RESULT_HANDLED;
    }

  reply = dbus_message_new_method_return (message);

  if (reply == NULL)
    g_error ("OOM");

  if (!dbus_connection_send (connection, reply, NULL))
    g_error ("OOM");

  dbus_clear_message (&reply);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
hold_filter (DBusConnection *connection,
    DBusMessage *message,
    void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  g_queue_push_tail (&f->held_messages, dbus_message_ref (message));

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
shouted_signal_filter (DBusConnection *connection,
                       DBusMessage *message,
                       void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_is_signal (message, "com.example", "Shouted"))
    f->signal_counter++;

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

typedef struct {
    const char *bug_ref;
    guint min_messages;
    const char *config_file;
    TestUser user;
    enum { SPECIFY_ADDRESS = 0, RELY_ON_DEFAULT } connect_mode;
} Config;

static void
setup (Fixture *f,
    gconstpointer context)
{
  const Config *config = context;

  /* Some tests are fairly slow, so make the test timeout per-test */
  test_timeout_reset (1);

  f->ctx = test_main_context_get ();
  f->ge = NULL;
  dbus_error_init (&f->e);

  if (config != NULL && config->connect_mode == RELY_ON_DEFAULT)
    {
      /* this is chosen to be something needing escaping */
      f->tmp_runtime_dir = g_dir_make_tmp ("dbus=daemon=test.XXXXXX", &f->ge);
      g_assert_no_error (f->ge);

      /* we're relying on being single-threaded for this to be safe */
      f->saved_runtime_dir = g_strdup (g_getenv ("XDG_RUNTIME_DIR"));
      g_setenv ("XDG_RUNTIME_DIR", f->tmp_runtime_dir, TRUE);
      g_unsetenv ("DBUS_SESSION_BUS_ADDRESS");
    }

  f->address = test_get_dbus_daemon (config ? config->config_file : NULL,
                                     config ? config->user : TEST_USER_ME,
                                     NULL, &f->daemon_pid);

  if (f->address == NULL)
    {
      f->skip = TRUE;
      return;
    }

  f->left_conn = test_connect_to_bus (f->ctx, f->address);

  if (config != NULL && config->connect_mode == RELY_ON_DEFAULT)
    {
      /* use the default bus for the echo service ("right"), to check that
       * it ends up on the same bus as the client ("left") */
      f->right_conn = dbus_bus_get_private (DBUS_BUS_SESSION, &f->e);
      test_assert_no_error (&f->e);

      test_connection_setup (f->ctx, f->right_conn);
    }
  else
    {
      f->right_conn = test_connect_to_bus (f->ctx, f->address);
    }
}

static void
add_echo_filter (Fixture *f)
{
  if (!dbus_connection_add_filter (f->right_conn, echo_filter, f, NULL))
    g_error ("OOM");

  f->right_conn_echo = TRUE;
}

static void
add_hold_filter (Fixture *f)
{
  if (!dbus_connection_add_filter (f->right_conn, hold_filter, f, NULL))
    g_error ("OOM");

  f->right_conn_hold = TRUE;
}

static void
add_shouted_signal_filter (Fixture *f)
{
  if (!dbus_connection_add_filter (f->left_conn, shouted_signal_filter, f, NULL))
    g_error ("OOM");

  f->left_conn_shouted_signal_filter = TRUE;
}

static void
right_conn_emit_shouted (Fixture *f)
{
  DBusMessage *m;

  m = dbus_message_new_signal ("/", "com.example", "Shouted");

  if (m == NULL)
    g_error ("OOM");

  if (!dbus_connection_send (f->right_conn, m, NULL))
    g_error ("OOM");

  dbus_clear_message (&m);
}

static void
pc_count (DBusPendingCall *pc,
    void *data)
{
  guint *received_p = data;

  (*received_p)++;
}

static void
pc_enqueue (DBusPendingCall *pc,
            void *data)
{
  GQueue *q = data;
  DBusMessage *m = dbus_pending_call_steal_reply (pc);

  g_test_message ("message of type %d", dbus_message_get_type (m));
  g_queue_push_tail (q, m);
}

static void
echo_left_to_right (Fixture *f,
                    guint count)
{
  guint sent;
  guint received = 0;

  for (sent = 0; sent < count; sent++)
    {
      DBusMessage *m = dbus_message_new_method_call (
          dbus_bus_get_unique_name (f->right_conn), "/",
          "com.example", "Spam");
      DBusPendingCall *pc;

      if (m == NULL)
        g_error ("OOM");

      if (!dbus_connection_send_with_reply (f->left_conn, m, &pc,
                                            DBUS_TIMEOUT_INFINITE) ||
          pc == NULL)
        g_error ("OOM");

      if (dbus_pending_call_get_completed (pc))
        pc_count (pc, &received);
      else if (!dbus_pending_call_set_notify (pc, pc_count, &received,
            NULL))
        g_error ("OOM");

      dbus_clear_pending_call (&pc);
      dbus_clear_message (&m);
    }

  while (received < count)
    test_main_context_iterate (f->ctx, TRUE);
}

static void
test_echo (Fixture *f,
    gconstpointer context)
{
  const Config *config = context;
  guint count = 2000;
  double elapsed;

  if (f->skip)
    return;

  if (config != NULL && config->bug_ref != NULL)
    g_test_bug (config->bug_ref);

  if (g_test_perf ())
    count = 100000;

  if (config != NULL)
    count = MAX (config->min_messages, count);

  add_echo_filter (f);

  g_test_timer_start ();

  echo_left_to_right (f, count);

  elapsed = g_test_timer_elapsed ();

  g_test_maximized_result (count / elapsed, "%u messages / %f seconds",
      count, elapsed);
}

static void
test_no_reply (Fixture *f,
    gconstpointer context)
{
  const Config *config = context;
  DBusMessage *m;
  DBusPendingCall *pc;
  DBusMessage *reply = NULL;
  enum { TIMEOUT, DISCONNECT } mode;
  gboolean ok;

  if (f->skip)
    return;

  g_test_bug ("76112");

  if (config != NULL && config->config_file != NULL)
    mode = TIMEOUT;
  else
    mode = DISCONNECT;

  m = dbus_message_new_method_call (
      dbus_bus_get_unique_name (f->right_conn), "/",
      "com.example", "WaitForever");

  add_echo_filter (f);

  if (m == NULL)
    g_error ("OOM");

  /* Not using test_main_context_call_and_wait() here because we need to
   * do things with the right connection as a side-effect */
  if (!dbus_connection_send_with_reply (f->left_conn, m, &pc,
                                        DBUS_TIMEOUT_INFINITE) ||
      pc == NULL)
    g_error ("OOM");

  if (dbus_pending_call_get_completed (pc))
    test_pending_call_store_reply (pc, &reply);
  else if (!dbus_pending_call_set_notify (pc, test_pending_call_store_reply,
        &reply, NULL))
    g_error ("OOM");

  dbus_clear_pending_call (&pc);
  dbus_clear_message (&m);

  if (mode == DISCONNECT)
    {
      while (!f->wait_forever_called)
        test_main_context_iterate (f->ctx, TRUE);

      dbus_connection_remove_filter (f->right_conn, echo_filter, f);
      test_connection_shutdown (f->ctx, f->right_conn);
      dbus_connection_close (f->right_conn);
      dbus_clear_connection (&f->right_conn);
    }

  while (reply == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  /* using inefficient string comparison for better assertion message */
  g_assert_cmpstr (
      dbus_message_type_to_string (dbus_message_get_type (reply)), ==,
      dbus_message_type_to_string (DBUS_MESSAGE_TYPE_ERROR));
  ok = dbus_set_error_from_message (&f->e, reply);
  g_assert (ok);
  g_assert_cmpstr (f->e.name, ==, DBUS_ERROR_NO_REPLY);

  if (mode == DISCONNECT)
    g_assert_cmpstr (f->e.message, ==,
        "Message recipient disconnected from message bus without replying");
  else
    g_assert_cmpstr (f->e.message, ==,
        "Message did not receive a reply (timeout by message bus)");

  dbus_clear_message (&reply);
}

#ifdef G_OS_UNIX
static int
gid_cmp (const void *ap, const void *bp)
{
  gid_t a = *(const gid_t *)ap;
  gid_t b = *(const gid_t *)bp;
  if (a < b)
    return -1;
  if (a > b)
    return 1;
  return 0;
}
#endif

static void
test_creds (Fixture *f,
    gconstpointer context)
{
  const char *unique = dbus_bus_get_unique_name (f->left_conn);
  DBusMessage *m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetConnectionCredentials");
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  DBusMessageIter pair_iter;
  DBusMessageIter var_iter;
  enum {
      SEEN_UNIX_USER = 1,
      SEEN_PID = 2,
      SEEN_WINDOWS_SID = 4,
      SEEN_LINUX_SECURITY_LABEL = 8,
      SEEN_UNIX_GROUPS = 16,
  } seen = 0;

  if (m == NULL)
    g_error ("OOM");

  if (!dbus_message_append_args (m,
        DBUS_TYPE_STRING, &unique,
        DBUS_TYPE_INVALID))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  g_assert_cmpstr (dbus_message_get_signature (reply), ==, "a{sv}");

  dbus_message_iter_init (reply, &args_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_ARRAY);
  g_assert_cmpuint (dbus_message_iter_get_element_type (&args_iter), ==,
      DBUS_TYPE_DICT_ENTRY);
  dbus_message_iter_recurse (&args_iter, &arr_iter);

  while (dbus_message_iter_get_arg_type (&arr_iter) != DBUS_TYPE_INVALID)
    {
      const char *name;

      dbus_message_iter_recurse (&arr_iter, &pair_iter);
      g_assert_cmpuint (dbus_message_iter_get_arg_type (&pair_iter), ==,
          DBUS_TYPE_STRING);
      dbus_message_iter_get_basic (&pair_iter, &name);
      dbus_message_iter_next (&pair_iter);
      g_assert_cmpuint (dbus_message_iter_get_arg_type (&pair_iter), ==,
          DBUS_TYPE_VARIANT);
      dbus_message_iter_recurse (&pair_iter, &var_iter);

      if (g_strcmp0 (name, "UnixUserID") == 0)
        {
#ifdef G_OS_UNIX
          guint32 u32;

          g_assert (!(seen & SEEN_UNIX_USER));
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&var_iter), ==,
              DBUS_TYPE_UINT32);
          dbus_message_iter_get_basic (&var_iter, &u32);
          g_test_message ("%s of this process is %u", name, u32);
          g_assert_cmpuint (u32, ==, geteuid ());
          seen |= SEEN_UNIX_USER;
#else
          g_assert_not_reached ();
#endif
        }
      else if (g_strcmp0 (name, "UnixGroupIDs") == 0)
        {
#ifdef G_OS_UNIX
          guint32 *groups;
          gid_t egid = getegid();
          gid_t *actual_groups;
          int len, ret, i;
          size_t nmemb;
          DBusMessageIter array_iter;

          g_assert (!(seen & SEEN_UNIX_GROUPS));
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&var_iter), ==,
              DBUS_TYPE_ARRAY);
          dbus_message_iter_recurse (&var_iter, &array_iter);
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&array_iter), ==,
              DBUS_TYPE_UINT32);
          dbus_message_iter_get_fixed_array (&array_iter, &groups, &len);
          g_test_message ("%s of this process present (%d groups)", name, len);
          g_assert_cmpint (len, >=, 1);

          actual_groups = g_new0 (gid_t, len+1);
          ret = getgroups (len, actual_groups);
          if (ret < 0)
            g_error ("getgroups: %s", g_strerror (errno));
          nmemb = ret;
          if (!lfind (&egid, actual_groups, &nmemb, sizeof (gid_t), gid_cmp))
            actual_groups[ret++] = egid;
          g_assert_cmpint (ret, ==, len);
          qsort (actual_groups, len, sizeof (gid_t), gid_cmp);
          for (i = 0; i < len; i++)
            g_assert_true (groups[i] == actual_groups[i]);
          g_free (actual_groups);

          seen |= SEEN_UNIX_GROUPS;
#else
          g_assert_not_reached ();
#endif
        }
      else if (g_strcmp0 (name, "WindowsSID") == 0)
        {
#ifdef G_OS_WIN32
          gchar *sid;
          char *self_sid;

          g_assert (!(seen & SEEN_WINDOWS_SID));
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&var_iter), ==,
              DBUS_TYPE_STRING);
          dbus_message_iter_get_basic (&var_iter, &sid);
          g_test_message ("%s of this process is %s", name, sid);
          if (_dbus_getsid (&self_sid, 0))
            {
              g_assert_cmpstr (self_sid, ==, sid);
              LocalFree(self_sid);
            }
          seen |= SEEN_WINDOWS_SID;
#else
          g_assert_not_reached ();
#endif
        }
      else if (g_strcmp0 (name, "ProcessID") == 0)
        {
          guint32 u32;

          g_assert (!(seen & SEEN_PID));
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&var_iter), ==,
              DBUS_TYPE_UINT32);
          dbus_message_iter_get_basic (&var_iter, &u32);
          g_test_message ("%s of this process is %u", name, u32);
#ifdef G_OS_UNIX
          g_assert_cmpuint (u32, ==, getpid ());
#elif defined(G_OS_WIN32)
          g_assert_cmpuint (u32, ==, GetCurrentProcessId ());
#else
          g_assert_not_reached ();
#endif
          seen |= SEEN_PID;
        }
      else if (g_strcmp0 (name, "LinuxSecurityLabel") == 0)
        {
#ifdef __linux__
          gchar *label;
          int len;
          DBusMessageIter ay_iter;

          g_assert (!(seen & SEEN_LINUX_SECURITY_LABEL));
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&var_iter), ==,
              DBUS_TYPE_ARRAY);
          dbus_message_iter_recurse (&var_iter, &ay_iter);
          g_assert_cmpuint (dbus_message_iter_get_arg_type (&ay_iter), ==,
              DBUS_TYPE_BYTE);
          dbus_message_iter_get_fixed_array (&ay_iter, &label, &len);
          g_test_message ("%s of this process is %s", name, label);
          g_assert_cmpuint (strlen (label) + 1, ==, len);
          seen |= SEEN_LINUX_SECURITY_LABEL;

          /*
           * At this point we would like to do something like:
           *
           * g_assert_cmpstr (label, ==, real_security_label);
           *
           * but there is no LSM-agnostic way to find out our real security
           * label in a way that matches SO_PEERSEC. The closest thing
           * available is reading /proc/self/attr/current, but that is only
           * equal to SO_PEERSEC after applying LSM-specific
           * canonicalization (for example for AppArmor you have to remove
           * a trailing newline from /proc/self/attr/current).
           */
#else
          g_assert_not_reached ();
#endif
        }

      dbus_message_iter_next (&arr_iter);
    }

#ifdef UNIX_USER_SHOULD_WORK
  g_assert (seen & SEEN_UNIX_USER);
#endif

#ifdef PID_SHOULD_WORK
  g_assert (seen & SEEN_PID);
#endif

#ifdef G_OS_WIN32
  g_assert (seen & SEEN_WINDOWS_SID);
#endif

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_processid (Fixture *f,
    gconstpointer context)
{
  const char *unique = dbus_bus_get_unique_name (f->left_conn);
  DBusMessage *m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "GetConnectionUnixProcessID");
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  guint32 pid;

  if (m == NULL)
    g_error ("OOM");

  if (!dbus_message_append_args (m,
        DBUS_TYPE_STRING, &unique,
        DBUS_TYPE_INVALID))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (dbus_set_error_from_message (&error, reply))
    {
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNIX_PROCESS_ID_UNKNOWN);

#ifdef PID_SHOULD_WORK
      g_error ("Expected pid to be passed, but got %s: %s",
          error.name, error.message);
#endif

      dbus_error_free (&error);
    }
  else if (dbus_message_get_args (reply, &error,
        DBUS_TYPE_UINT32, &pid,
        DBUS_TYPE_INVALID))
    {
      g_assert_cmpstr (dbus_message_get_signature (reply), ==, "u");
      test_assert_no_error (&error);

      g_test_message ("GetConnectionUnixProcessID returned %u", pid);

#ifdef G_OS_UNIX
      g_assert_cmpuint (pid, ==, getpid ());
#elif defined(G_OS_WIN32)
      g_assert_cmpuint (pid, ==, GetCurrentProcessId ());
#else
      g_assert_not_reached ();
#endif
    }
  else
    {
      g_error ("Unexpected error: %s: %s", error.name, error.message);
    }

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_canonical_path_uae (Fixture *f,
    gconstpointer context)
{
  DBusMessage *m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);

  /* Append an empty a{ss} (string => string dictionary). */
  if (!dbus_message_iter_open_container (&args_iter, DBUS_TYPE_ARRAY,
        "{ss}", &arr_iter) ||
      !dbus_message_iter_close_container (&args_iter, &arr_iter))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  /* it succeeds */
  g_assert_cmpint (dbus_message_get_type (reply), ==,
      DBUS_MESSAGE_TYPE_METHOD_RETURN);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);

  /* Now try with the wrong object path */
  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      "/com/example/Wrong", DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");

  if (m == NULL)
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);

  /* Append an empty a{ss} (string => string dictionary). */
  if (!dbus_message_iter_open_container (&args_iter, DBUS_TYPE_ARRAY,
        "{ss}", &arr_iter) ||
      !dbus_message_iter_close_container (&args_iter, &arr_iter))
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  /* it fails, yielding an error message with one string argument */
  g_assert_cmpint (dbus_message_get_type (reply), ==, DBUS_MESSAGE_TYPE_ERROR);
  g_assert_cmpstr (dbus_message_get_error_name (reply), ==,
      DBUS_ERROR_ACCESS_DENIED);
  g_assert_cmpstr (dbus_message_get_signature (reply), ==, "s");

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static Config max_connections_per_user_config = {
    NULL, 1, "valid-config-files/max-connections-per-user.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static void
test_max_connections (Fixture *f,
    gconstpointer context)
{
  DBusError error = DBUS_ERROR_INIT;
  DBusConnection *third_conn;
  DBusConnection *failing_conn;
#ifdef DBUS_WIN
  const Config *config = context;
#endif

  if (f->skip)
    return;

#ifdef DBUS_WIN
  if (config == &max_connections_per_user_config)
    {
      /* <limit name="max_connections_per_user"/> is currently only
       * implemented in terms of Unix uids. It could be implemented for
       * Windows SIDs too, but there wouldn't be much point, because we
       * don't support use of a multi-user dbus-daemon on Windows, so
       * in practice all connections have the same SID. */
      g_test_skip ("Maximum connections per Windows SID are not "
                   "implemented");
      return;
    }
#endif

  /* We have two connections already */
  g_assert (f->left_conn != NULL);
  g_assert (f->right_conn != NULL);

  /* Our configuration file sets the limit to 3 connections, either globally
   * or per uid, so this one is the last that will work */
  third_conn = test_connect_to_bus (f->ctx, f->address);

  /* This one is going to fail. We don't guarantee whether it will fail
   * now, or while registering (implementation detail: it's the latter). */
  failing_conn = dbus_connection_open_private (f->address, &error);

  if (failing_conn != NULL)
    {
      gboolean ok = dbus_bus_register (failing_conn, &error);

      g_assert (!ok);
    }

  g_assert (dbus_error_is_set (&error));
  g_assert_cmpstr (error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);

  if (failing_conn != NULL)
    dbus_connection_close (failing_conn);

  dbus_clear_connection (&failing_conn);
  test_connection_shutdown (f->ctx, third_conn);
  dbus_connection_close (third_conn);
  dbus_clear_connection (&third_conn);
  dbus_error_free (&error);
}

static void
test_max_replies_per_connection (Fixture *f,
    gconstpointer context)
{
  GQueue received = G_QUEUE_INIT;
  GQueue errors = G_QUEUE_INIT;
  DBusMessage *m;
  DBusPendingCall *pc;
  guint i;
  DBusError error = DBUS_ERROR_INIT;

  if (f->skip)
    return;

  add_hold_filter (f);

  /* The configured limit is 3 replies per connection. */
  for (i = 0; i < 3; i++)
    {
      m = dbus_message_new_method_call (
          dbus_bus_get_unique_name (f->right_conn), "/",
          "com.example", "Spam");

      if (m == NULL)
        g_error ("OOM");

      if (!dbus_connection_send_with_reply (f->left_conn, m, &pc,
                                            DBUS_TIMEOUT_INFINITE) ||
          pc == NULL)
        g_error ("OOM");

      if (dbus_pending_call_get_completed (pc))
        pc_enqueue (pc, &received);
      else if (!dbus_pending_call_set_notify (pc, pc_enqueue, &received,
            NULL))
        g_error ("OOM");

      dbus_pending_call_unref (pc);
      dbus_message_unref (m);
    }

  while (g_queue_get_length (&f->held_messages) < 3)
    test_main_context_iterate (f->ctx, TRUE);

  g_assert_cmpuint (g_queue_get_length (&received), ==, 0);
  g_assert_cmpuint (g_queue_get_length (&errors), ==, 0);

  /* Go a couple of messages over the limit. */
  for (i = 0; i < 2; i++)
    {
      m = dbus_message_new_method_call (
          dbus_bus_get_unique_name (f->right_conn), "/",
          "com.example", "Spam");

      if (m == NULL)
        g_error ("OOM");

      if (!dbus_connection_send_with_reply (f->left_conn, m, &pc,
                                            DBUS_TIMEOUT_INFINITE) ||
          pc == NULL)
        g_error ("OOM");

      if (dbus_pending_call_get_completed (pc))
        pc_enqueue (pc, &errors);
      else if (!dbus_pending_call_set_notify (pc, pc_enqueue, &errors,
            NULL))
        g_error ("OOM");

      dbus_pending_call_unref (pc);
      dbus_message_unref (m);
    }

  /* Reply to the held messages. */
  for (m = g_queue_pop_head (&f->held_messages);
       m != NULL;
       m = g_queue_pop_head (&f->held_messages))
    {
      DBusMessage *reply = dbus_message_new_method_return (m);

      if (reply == NULL)
        g_error ("OOM");

      if (!dbus_connection_send (f->right_conn, reply, NULL))
        g_error ("OOM");

      dbus_clear_message (&m);
      dbus_clear_message (&reply);
    }

  /* Wait for all 5 replies to come in. */
  while (g_queue_get_length (&received) + g_queue_get_length (&errors) < 5)
    test_main_context_iterate (f->ctx, TRUE);

  /* The first three succeeded. */
  for (i = 0; i < 3; i++)
    {
      m = g_queue_pop_head (&received);
      g_assert (m != NULL);

      if (dbus_set_error_from_message (&error, m))
        g_error ("Unexpected error: %s: %s", error.name, error.message);

      dbus_clear_message (&m);
    }

  /* The last two failed. */
  for (i = 0; i < 2; i++)
    {
      m = g_queue_pop_head (&errors);
      g_assert (m != NULL);

      if (!dbus_set_error_from_message (&error, m))
        g_error ("Unexpected success");

      g_assert_cmpstr (error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);
      dbus_error_free (&error);
      dbus_clear_message (&m);
    }

  g_assert_cmpuint (g_queue_get_length (&received), ==, 0);
  g_assert_cmpuint (g_queue_get_length (&errors), ==, 0);
  g_queue_clear (&received);
  g_queue_clear (&errors);
}

static void
test_max_match_rules_per_connection (Fixture *f,
    gconstpointer context)
{
  DBusError error = DBUS_ERROR_INIT;

  if (f->skip)
    return;

  dbus_bus_add_match (f->left_conn, "sender='com.example.C1'", &error);
  test_assert_no_error (&error);
  dbus_bus_add_match (f->left_conn, "sender='com.example.C2'", &error);
  test_assert_no_error (&error);
  dbus_bus_add_match (f->left_conn, "sender='com.example.C3'", &error);
  test_assert_no_error (&error);

  dbus_bus_add_match (f->left_conn, "sender='com.example.C4'", &error);
  g_assert_cmpstr (error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);
  dbus_error_free (&error);

  dbus_bus_remove_match (f->left_conn, "sender='com.example.C3'", &error);
  test_assert_no_error (&error);

  dbus_bus_add_match (f->left_conn, "sender='com.example.C4'", &error);
  test_assert_no_error (&error);
}

static void
test_max_names_per_connection (Fixture *f,
    gconstpointer context)
{
  DBusError error = DBUS_ERROR_INIT;
  int ret;

  if (f->skip)
    return;

  /* The limit in the configuration file is set to 4, but we only own 3
   * names here - remember that the unique name is a name too. */

  ret = dbus_bus_request_name (f->left_conn, "com.example.C1", 0, &error);
  test_assert_no_error (&error);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  ret = dbus_bus_request_name (f->left_conn, "com.example.C2", 0, &error);
  test_assert_no_error (&error);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  ret = dbus_bus_request_name (f->left_conn, "com.example.C3", 0, &error);
  test_assert_no_error (&error);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  ret = dbus_bus_request_name (f->left_conn, "com.example.C4", 0, &error);
  g_assert_cmpstr (error.name, ==, DBUS_ERROR_LIMITS_EXCEEDED);
  dbus_error_free (&error);
  g_assert_cmpint (ret, ==, -1);

  ret = dbus_bus_release_name (f->left_conn, "com.example.C3", &error);
  test_assert_no_error (&error);
  g_assert_cmpint (ret, ==, DBUS_RELEASE_NAME_REPLY_RELEASED);

  ret = dbus_bus_request_name (f->left_conn, "com.example.C4", 0, &error);
  test_assert_no_error (&error);
  g_assert_cmpint (ret, ==, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
}

#if defined(DBUS_UNIX) && defined(HAVE_UNIX_FD_PASSING) && defined(HAVE_GIO_UNIX)

static DBusHandlerResult
wait_for_disconnected_cb (DBusConnection *client_conn,
    DBusMessage *message,
    void *data)
{
  gboolean *disconnected = data;

  if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected"))
    {
      *disconnected = TRUE;
      return DBUS_HANDLER_RESULT_HANDLED;
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static const guchar partial_message[] =
{
  DBUS_LITTLE_ENDIAN,
  DBUS_MESSAGE_TYPE_METHOD_CALL,
  0, /* flags */
  1, /* version */
  0xff, 0xff, 0, 0, /* length of body = 65535 bytes */
  1, 2, 3, 4, /* cookie */
  0xff, 0xff, 0, 0, /* length of header fields array = 65535 bytes */
  42 /* pretending to be the beginning of the header fields array */
};

static void
send_all_with_fd (GSocket *socket,
                  const guchar *local_partial_message,
                  gsize len,
                  int fd)
{
  GSocketControlMessage *fdm = g_unix_fd_message_new ();
  GError *error = NULL;
  gssize sent;
  GOutputVector vector = { local_partial_message, len };

  g_unix_fd_message_append_fd (G_UNIX_FD_MESSAGE (fdm), fd, &error);
  g_assert_no_error (error);

  sent = g_socket_send_message (socket, NULL, &vector, 1, &fdm, 1,
      G_SOCKET_MSG_NONE, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpint (sent, >=, 1);
  g_assert_cmpint (sent, <=, vector.size);

  while (((gsize) sent) < vector.size)
    {
      vector.size -= sent;
      vector.buffer = ((const guchar *) vector.buffer) + sent;
      sent = g_socket_send_message (socket, NULL, &vector, 1, NULL, 0,
          G_SOCKET_MSG_NONE, NULL, &error);
      g_assert_no_error (error);
      g_assert_cmpint (sent, >=, 1);
      g_assert_cmpint (sent, <=, vector.size);
    }

  g_object_unref (fdm);
}

static void
test_pending_fd_timeout (Fixture *f,
    gconstpointer context)
{
  GError *error = NULL;
  gint64 start;
  int fd;
  GSocket *socket;
  gboolean have_mem;
  gboolean disconnected = FALSE;

  if (f->skip)
    return;

  if (getuid () == 0)
    {
      g_test_skip ("Cannot test, uid 0 is immune to this limit");
      return;
    }

  have_mem = dbus_connection_add_filter (f->left_conn, wait_for_disconnected_cb,
      &disconnected, NULL);
  g_assert (have_mem);

  /* This is not API. Never do this. */

  if (!dbus_connection_get_socket (f->left_conn, &fd))
    g_error ("failed to steal fd from left connection");

  socket = g_socket_new_from_fd (fd, &error);
  g_assert_no_error (error);
  g_assert (socket != NULL);

  /* We send part of a message that contains a fd, then stop. */
  start = g_get_monotonic_time ();
  send_all_with_fd (socket, partial_message, G_N_ELEMENTS (partial_message),
                    fd);

  while (!disconnected)
    {
      test_progress ('.');
      test_main_context_iterate (f->ctx, TRUE);

      /* It should take 0.5s to get disconnected, as configured in
       * valid-config-files/pending-fd-timeout.conf; but this test
       * might get starved by other processes running in parallel
       * (particularly on shared CI systems), so we have to be a lot
       * more generous. Allow up to 10 seconds. */
      g_assert_cmpint (g_get_monotonic_time (), <=,
                       start + (10 * G_USEC_PER_SEC));
    }

  g_object_unref (socket);
}

typedef struct
{
  const gchar *path;
  guint n_fds;
  gboolean should_work;
} CountFdsVector;

static const CountFdsVector count_fds_vectors[] =
{
  /* Deny sending if number of fds <= 2 */
  { "/test/DenySendMax2", 1, FALSE },
  { "/test/DenySendMax2", 2, FALSE },
  { "/test/DenySendMax2", 3, TRUE },
  { "/test/DenySendMax2", 4, TRUE },

  /* Deny receiving if number of fds <= 3 */
  { "/test/DenyReceiveMax3", 2, FALSE },
  { "/test/DenyReceiveMax3", 3, FALSE },
  { "/test/DenyReceiveMax3", 4, TRUE },
  { "/test/DenyReceiveMax3", 5, TRUE },

  /* Deny sending if number of fds >= 4 */
  { "/test/DenySendMin4", 2, TRUE },
  { "/test/DenySendMin4", 3, TRUE },
  { "/test/DenySendMin4", 4, FALSE },
  { "/test/DenySendMin4", 5, FALSE },

  /* Deny receiving if number of fds >= 5 */
  { "/test/DenyReceiveMin5", 3, TRUE },
  { "/test/DenyReceiveMin5", 4, TRUE },
  { "/test/DenyReceiveMin5", 5, FALSE },
  { "/test/DenyReceiveMin5", 6, FALSE },
};

static void
test_count_fds (Fixture *f,
    gconstpointer context)
{
  GQueue received = G_QUEUE_INIT;
  DBusMessage *m;
  DBusPendingCall *pc;
  guint i;
  DBusError error = DBUS_ERROR_INIT;
  const int stdin_fd = 0;

  if (f->skip)
    return;

  add_hold_filter (f);

  for (i = 0; i < G_N_ELEMENTS (count_fds_vectors); i++)
    {
      const CountFdsVector *vector = &count_fds_vectors[i];
      guint j;

      m = dbus_message_new_method_call (
          dbus_bus_get_unique_name (f->right_conn), vector->path,
          "com.example", "Spam");

      if (m == NULL)
        g_error ("OOM");

      for (j = 0; j < vector->n_fds; j++)
        {
          if (!dbus_message_append_args (m,
                                         DBUS_TYPE_UNIX_FD, &stdin_fd,
                                         DBUS_TYPE_INVALID))
            g_error ("OOM");
        }

      if (!dbus_connection_send_with_reply (f->left_conn, m, &pc,
                                            DBUS_TIMEOUT_INFINITE) ||
          pc == NULL)
        g_error ("OOM");

      if (dbus_pending_call_get_completed (pc))
        pc_enqueue (pc, &received);
      else if (!dbus_pending_call_set_notify (pc, pc_enqueue, &received,
            NULL))
        g_error ("OOM");

      dbus_pending_call_unref (pc);
      dbus_message_unref (m);

      if (vector->should_work)
        {
          DBusMessage *reply;

          while (g_queue_get_length (&f->held_messages) < 1)
            test_main_context_iterate (f->ctx, TRUE);

          g_assert_cmpint (g_queue_get_length (&f->held_messages), ==, 1);

          m = g_queue_pop_head (&f->held_messages);

          g_assert_cmpint (g_queue_get_length (&f->held_messages), ==, 0);

          reply = dbus_message_new_method_return (m);

          if (reply == NULL)
            g_error ("OOM");

          if (!dbus_connection_send (f->right_conn, reply, NULL))
            g_error ("OOM");

          dbus_message_unref (reply);
          dbus_message_unref (m);
        }

      while (g_queue_get_length (&received) < 1)
        test_main_context_iterate (f->ctx, TRUE);

      g_assert_cmpint (g_queue_get_length (&received), ==, 1);
      m = g_queue_pop_head (&received);
      g_assert (m != NULL);
      g_assert_cmpint (g_queue_get_length (&received), ==, 0);

      if (vector->should_work)
        {
          if (dbus_set_error_from_message (&error, m))
            g_error ("Unexpected error: %s: %s", error.name, error.message);

          g_test_message ("Sending %u fds to %s was not denied, as expected",
                          vector->n_fds, vector->path);
        }
      else if (!dbus_set_error_from_message (&error, m))
        {
          g_error ("Unexpected success");
        }
      else
        {
          g_assert_cmpstr (error.name, ==, DBUS_ERROR_ACCESS_DENIED);
          dbus_error_free (&error);
          g_test_message ("Sending %u fds to %s was denied, as expected",
                          vector->n_fds, vector->path);
        }

      dbus_message_unref (m);
    }
}

#endif

static void
test_peer_get_machine_id (Fixture *f,
                          gconstpointer context)
{
  char *what_i_think;
  const char *what_daemon_thinks;
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;

  if (f->skip)
    return;

  what_i_think = dbus_try_get_local_machine_id (&error);

  if (what_i_think == NULL)
    {
      if (g_getenv ("DBUS_TEST_UNINSTALLED") != NULL)
        {
          /* When running unit tests during make check or make installcheck,
           * tolerate this */
          g_test_skip ("Machine UUID not available");
          dbus_error_free (&error);
          return;
        }
      else
        {
          /* When running integration tests, don't tolerate it */
          g_error ("%s", error.message);
        }
    }

  /* Check that the dbus-daemon agrees with us. */
  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
                                    DBUS_PATH_DBUS,
                                    DBUS_INTERFACE_PEER,
                                    "GetMachineId");

  if (m == NULL)
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_message_get_args (reply, &error,
        DBUS_TYPE_STRING, &what_daemon_thinks,
        DBUS_TYPE_INVALID))
    g_error ("%s: %s", error.name, error.message);

  g_assert_cmpstr (what_i_think, ==, what_daemon_thinks);
  g_assert_nonnull (what_daemon_thinks);
  g_assert_cmpuint (strlen (what_daemon_thinks), ==, 32);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
  dbus_free (what_i_think);
}

static void
test_peer_ping (Fixture *f,
                gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PEER, "Ping");

  if (m == NULL)
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_message_get_args (reply, &error, DBUS_TYPE_INVALID))
    g_error ("%s: %s", error.name, error.message);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_get_invalid_path (Fixture *f,
                       gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Interfaces";

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS, "/",
      DBUS_INTERFACE_PROPERTIES, "Get");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  /* That object path does not have that interface */
  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_get_invalid_iface (Fixture *f,
                        gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = "com.example.Nope";
  const char *property = "Whatever";

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_get_invalid (Fixture *f,
                  gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Whatever";

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_PROPERTY);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_get_all_invalid_iface (Fixture *f,
                            gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = "com.example.Nope";

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "GetAll");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_get_all_invalid_path (Fixture *f,
                           gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      "/", DBUS_INTERFACE_PROPERTIES, "GetAll");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  /* That object path does not have that interface */
  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_set_invalid_iface (Fixture *f,
                        gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = "com.example.Nope";
  const char *property = "Whatever";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Set");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID))
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);

  if (!dbus_message_iter_open_container (&args_iter,
        DBUS_TYPE_VARIANT, "b", &var_iter) ||
      !dbus_message_iter_append_basic (&var_iter, DBUS_TYPE_BOOLEAN, &b) ||
      !dbus_message_iter_close_container (&args_iter, &var_iter))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_set_invalid_path (Fixture *f,
                       gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Interfaces";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      "/", DBUS_INTERFACE_PROPERTIES, "Set");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID))
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);

  if (!dbus_message_iter_open_container (&args_iter,
        DBUS_TYPE_VARIANT, "b", &var_iter) ||
      !dbus_message_iter_append_basic (&var_iter, DBUS_TYPE_BOOLEAN, &b) ||
      !dbus_message_iter_close_container (&args_iter, &var_iter))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_INTERFACE);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_set_invalid (Fixture *f,
                  gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Whatever";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Set");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID))
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);

  if (!dbus_message_iter_open_container (&args_iter,
        DBUS_TYPE_VARIANT, "b", &var_iter) ||
      !dbus_message_iter_append_basic (&var_iter, DBUS_TYPE_BOOLEAN, &b) ||
      !dbus_message_iter_close_container (&args_iter, &var_iter))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  g_assert_cmpstr (error.name, ==, DBUS_ERROR_UNKNOWN_PROPERTY);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_set (Fixture *f,
          gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusError error = DBUS_ERROR_INIT;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *property = "Features";
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  dbus_bool_t b = FALSE;

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Set");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID))
    g_error ("OOM");

  dbus_message_iter_init_append (m, &args_iter);

  if (!dbus_message_iter_open_container (&args_iter,
        DBUS_TYPE_VARIANT, "b", &var_iter) ||
      !dbus_message_iter_append_basic (&var_iter, DBUS_TYPE_BOOLEAN, &b) ||
      !dbus_message_iter_close_container (&args_iter, &var_iter))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_set_error_from_message (&error, reply))
    g_error ("Unexpected success");

  g_assert_cmpstr (error.name, ==, DBUS_ERROR_PROPERTY_READ_ONLY);
  dbus_error_free (&error);

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
check_features (DBusMessageIter *var_iter)
{
  DBusMessageIter arr_iter;
  gboolean have_systemd_activation = FALSE;
  gboolean have_header_filtering = FALSE;

  g_assert_cmpint (dbus_message_iter_get_arg_type (var_iter), ==,
      DBUS_TYPE_ARRAY);
  g_assert_cmpint (dbus_message_iter_get_element_type (var_iter), ==,
      DBUS_TYPE_STRING);
  dbus_message_iter_recurse (var_iter, &arr_iter);

  while (dbus_message_iter_get_arg_type (&arr_iter) != DBUS_TYPE_INVALID)
    {
      const char *feature;

      g_assert_cmpint (dbus_message_iter_get_arg_type (&arr_iter), ==,
          DBUS_TYPE_STRING);
      dbus_message_iter_get_basic (&arr_iter, &feature);

      g_test_message ("Feature: %s", feature);

      if (g_strcmp0 (feature, "HeaderFiltering") == 0)
        have_header_filtering = TRUE;
      else if (g_strcmp0 (feature, "SystemdActivation") == 0)
        have_systemd_activation = TRUE;

      dbus_message_iter_next (&arr_iter);
    }

  g_assert_true (have_header_filtering);
  /* We pass --systemd-activation to the daemon for this unit test on Unix
   * (it can only work in practice on Linux, but there's nothing
   * inherently Linux-specific about the protocol). */
#ifdef DBUS_UNIX
  g_assert_true (have_systemd_activation);
#else
  g_assert_false (have_systemd_activation);
#endif
}

static void
test_features (Fixture *f,
               gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *features = "Features";

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &features,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_message_iter_init (reply, &args_iter))
    g_error ("Reply has no arguments");

  g_assert_cmpint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_VARIANT);

  dbus_message_iter_recurse (&args_iter, &var_iter);
  check_features (&var_iter);

  if (dbus_message_iter_next (&args_iter))
    g_error ("Reply has too many arguments");

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
check_interfaces (DBusMessageIter *var_iter)
{
  DBusMessageIter arr_iter;
  gboolean have_monitoring = FALSE;
  gboolean have_stats = FALSE;
  gboolean have_verbose = FALSE;

  g_assert_cmpint (dbus_message_iter_get_arg_type (var_iter), ==,
      DBUS_TYPE_ARRAY);
  g_assert_cmpint (dbus_message_iter_get_element_type (var_iter), ==,
      DBUS_TYPE_STRING);
  dbus_message_iter_recurse (var_iter, &arr_iter);

  while (dbus_message_iter_get_arg_type (&arr_iter) != DBUS_TYPE_INVALID)
    {
      const char *iface;

      g_assert_cmpint (dbus_message_iter_get_arg_type (&arr_iter), ==,
          DBUS_TYPE_STRING);
      dbus_message_iter_get_basic (&arr_iter, &iface);
      g_test_message ("Interface: %s", iface);

      g_assert_cmpstr (iface, !=, DBUS_INTERFACE_DBUS);
      g_assert_cmpstr (iface, !=, DBUS_INTERFACE_PROPERTIES);
      g_assert_cmpstr (iface, !=, DBUS_INTERFACE_INTROSPECTABLE);
      g_assert_cmpstr (iface, !=, DBUS_INTERFACE_PEER);

      if (g_strcmp0 (iface, DBUS_INTERFACE_MONITORING) == 0)
        have_monitoring = TRUE;
      else if (g_strcmp0 (iface, BUS_INTERFACE_STATS) == 0)
        have_stats = TRUE;
      else if (g_strcmp0 (iface, DBUS_INTERFACE_VERBOSE) == 0)
        have_verbose = TRUE;

      dbus_message_iter_next (&arr_iter);
    }

  g_assert_true (have_monitoring);

#ifdef DBUS_ENABLE_STATS
  g_assert_true (have_stats);
#else
  g_assert_false (have_stats);
#endif

#ifdef DBUS_ENABLE_VERBOSE_MODE
  g_assert_true (have_verbose);
#else
  g_assert_false (have_verbose);
#endif
}

static void
test_interfaces (Fixture *f,
                 gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter var_iter;
  const char *iface = DBUS_INTERFACE_DBUS;
  const char *ifaces = "Interfaces";

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "Get");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &ifaces,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  if (!dbus_message_iter_init (reply, &args_iter))
    g_error ("Reply has no arguments");

  if (dbus_message_iter_get_arg_type (&args_iter) != DBUS_TYPE_VARIANT)
    g_error ("Reply does not have a variant argument");

  dbus_message_iter_recurse (&args_iter, &var_iter);
  check_interfaces (&var_iter);

  if (dbus_message_iter_next (&args_iter))
    g_error ("Reply has too many arguments");

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

static void
test_get_all (Fixture *f,
              gconstpointer context)
{
  DBusMessage *m = NULL;
  DBusMessage *reply = NULL;
  DBusMessageIter args_iter;
  DBusMessageIter arr_iter;
  DBusMessageIter pair_iter;
  DBusMessageIter var_iter;
  const char *iface = DBUS_INTERFACE_DBUS;
  gboolean have_features = FALSE;
  gboolean have_interfaces = FALSE;

  if (f->skip)
    return;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_PROPERTIES, "GetAll");

  if (m == NULL ||
      !dbus_message_append_args (m,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_INVALID))
    test_oom ();

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, m,
      DBUS_TIMEOUT_USE_DEFAULT);

  dbus_message_iter_init (reply, &args_iter);
  g_assert_cmpuint (dbus_message_iter_get_arg_type (&args_iter), ==,
      DBUS_TYPE_ARRAY);
  g_assert_cmpuint (dbus_message_iter_get_element_type (&args_iter), ==,
      DBUS_TYPE_DICT_ENTRY);
  dbus_message_iter_recurse (&args_iter, &arr_iter);

  while (dbus_message_iter_get_arg_type (&arr_iter) != DBUS_TYPE_INVALID)
    {
      const char *name;

      dbus_message_iter_recurse (&arr_iter, &pair_iter);
      g_assert_cmpuint (dbus_message_iter_get_arg_type (&pair_iter), ==,
          DBUS_TYPE_STRING);
      dbus_message_iter_get_basic (&pair_iter, &name);
      dbus_message_iter_next (&pair_iter);
      g_assert_cmpuint (dbus_message_iter_get_arg_type (&pair_iter), ==,
          DBUS_TYPE_VARIANT);
      dbus_message_iter_recurse (&pair_iter, &var_iter);

      if (g_strcmp0 (name, "Features") == 0)
        {
          check_features (&var_iter);
          have_features = TRUE;
        }
      else if (g_strcmp0 (name, "Interfaces") == 0)
        {
          check_interfaces (&var_iter);
          have_interfaces = TRUE;
        }

      dbus_message_iter_next (&arr_iter);
    }

  g_assert_true (have_features);
  g_assert_true (have_interfaces);

  if (dbus_message_iter_next (&args_iter))
    g_error ("Reply has too many arguments");

  dbus_clear_message (&reply);
  dbus_clear_message (&m);
}

#define DESIRED_RLIMIT 65536

#ifdef DBUS_UNIX
static void
test_fd_limit (Fixture *f,
               gconstpointer context)
{
#ifdef HAVE_PRLIMIT
  struct rlimit lim;
  struct rlimit new_limit;
  const struct passwd *pwd = NULL;
#endif

  if (f->skip)
    return;

#ifdef HAVE_PRLIMIT

  if (getuid () != 0)
    {
      g_test_skip ("Cannot test, only uid 0 is expected to raise fd limit");
      return;
    }

  pwd = getpwnam (DBUS_USER);

  if (pwd == NULL)
    {
      gchar *message = g_strdup_printf ("user '%s' does not exist",
          DBUS_USER);

      g_test_skip (message);
      g_free (message);
      return;
    }

  if (prlimit (getpid (), RLIMIT_NOFILE, NULL, &lim) < 0)
    g_error ("get prlimit (self): %s", g_strerror (errno));

  g_test_message ("our RLIMIT_NOFILE: rlim_cur: %ld, rlim_max: %ld",
                  (long) lim.rlim_cur, (long) lim.rlim_max);

  if (lim.rlim_cur == RLIM_INFINITY || lim.rlim_cur >= DESIRED_RLIMIT)
    {
      /* The dbus-daemon will have inherited our large rlimit */
      g_test_skip ("Cannot test, our own fd limit was already large");
      return;
    }

  new_limit = lim;
  new_limit.rlim_cur = DESIRED_RLIMIT;
  new_limit.rlim_max = DESIRED_RLIMIT;

  /* Try to increase the rlimit ourselves. If we're root in an
   * unprivileged Linux container, then we won't have CAP_SYS_RESOURCE
   * and this will fail with EPERM. If so, the dbus-daemon wouldn't be
   * able to increase its rlimit either. */
  if (prlimit (getpid (), RLIMIT_NOFILE, &new_limit, NULL) < 0)
    {
      gchar *message;

      message = g_strdup_printf ("Cannot test, we cannot change the rlimit so "
                                 "presumably neither can the dbus-daemon: %s",
                                 g_strerror (errno));
      g_test_skip (message);
      g_free (message);
      return;
    }

  /* Immediately put our original limit back so it won't interfere with
   * subsequent tests. This should always succeed. */
  if (prlimit (getpid (), RLIMIT_NOFILE, &lim, NULL) < 0)
    g_error ("Cannot restore our original limits: %s", g_strerror (errno));

  if (prlimit (f->daemon_pid, RLIMIT_NOFILE, NULL, &lim) < 0)
    g_error ("get prlimit (dbus-daemon): %s", g_strerror (errno));

  g_test_message ("dbus-daemon's RLIMIT_NOFILE: rlim_cur: %ld, rlim_max: %ld",
                  (long) lim.rlim_cur, (long) lim.rlim_max);

  if (lim.rlim_cur != RLIM_INFINITY)
    g_assert_cmpint (lim.rlim_cur, >=, DESIRED_RLIMIT);

#else /* !HAVE_PRLIMIT */

  g_test_skip ("prlimit() not supported on this platform");

#endif /* !HAVE_PRLIMIT */
}

#define ECHO_SERVICE "org.freedesktop.DBus.TestSuiteEchoService"
#define FORKING_ECHO_SERVICE "org.freedesktop.DBus.TestSuiteForkingEchoService"
#define ECHO_SERVICE_PATH "/org/freedesktop/TestSuite"
#define ECHO_SERVICE_INTERFACE "org.freedesktop.TestSuite"

#ifdef ENABLE_TRADITIONAL_ACTIVATION
/*
 * Helper for test_activation_forking: whenever the forking service is
 * activated, start it again.
 */
static DBusHandlerResult
activation_forking_signal_filter (DBusConnection *connection,
                                  DBusMessage *message,
                                  void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_is_signal (message, DBUS_INTERFACE_DBUS,
                              "NameOwnerChanged"))
    {
      dbus_bool_t ok;
      const char *name;
      const char *old_owner;
      const char *new_owner;

      ok = dbus_message_get_args (message, &f->e,
                                  DBUS_TYPE_STRING, &name,
                                  DBUS_TYPE_STRING, &old_owner,
                                  DBUS_TYPE_STRING, &new_owner,
                                  DBUS_TYPE_INVALID);
      test_assert_no_error (&f->e);
      g_assert_true (ok);

      g_test_message ("owner of \"%s\": \"%s\" -> \"%s\"",
                      name, old_owner, new_owner);

      if (g_strcmp0 (name, FORKING_ECHO_SERVICE) != 0)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

      if (f->activation_forking_counter > 10)
        {
          g_test_message ("Activated 10 times OK, TestSuiteForkingEchoService pass");
          return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

      f->activation_forking_counter++;

      if (g_strcmp0 (new_owner, "") == 0)
        {
          /* Reactivate it, and tell it to exit immediately. */
          DBusMessage *echo_call = NULL;
          DBusMessage *exit_call = NULL;
          gchar *payload = NULL;

          payload = g_strdup_printf ("counter %u", f->activation_forking_counter);
          echo_call = dbus_message_new_method_call (FORKING_ECHO_SERVICE,
                                                    ECHO_SERVICE_PATH,
                                                    ECHO_SERVICE_INTERFACE,
                                                    "Echo");
          exit_call = dbus_message_new_method_call (FORKING_ECHO_SERVICE,
                                                    ECHO_SERVICE_PATH,
                                                    ECHO_SERVICE_INTERFACE,
                                                    "Exit");

          if (echo_call == NULL ||
              !dbus_message_append_args (echo_call,
                                         DBUS_TYPE_STRING, &payload,
                                         DBUS_TYPE_INVALID) ||
              exit_call == NULL ||
              !dbus_connection_send (connection, echo_call, NULL) ||
              !dbus_connection_send (connection, exit_call, NULL))
            g_error ("OOM");

          dbus_clear_message (&echo_call);
          dbus_clear_message (&exit_call);
          g_free (payload);
        }
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/*
 * Assert that Unix services are allowed to daemonize, and this does not
 * cause us to signal an activation failure.
 */
static void
test_activation_forking (Fixture *f,
                         gconstpointer context G_GNUC_UNUSED)
{
  DBusMessage *call = NULL;
  DBusMessage *reply = NULL;
  const char *hello = "hello world";

  if (f->skip)
    return;

  if (!dbus_connection_add_filter (f->left_conn,
                                   activation_forking_signal_filter,
                                   f, NULL))
    g_error ("OOM");

  /* Start it up */
  call = dbus_message_new_method_call (FORKING_ECHO_SERVICE,
                                       ECHO_SERVICE_PATH,
                                       ECHO_SERVICE_INTERFACE,
                                       "Echo");

  if (call == NULL ||
      !dbus_message_append_args (call,
                                 DBUS_TYPE_STRING, &hello,
                                 DBUS_TYPE_INVALID))
    g_error ("OOM");

  dbus_bus_add_match (f->left_conn,
                      "sender='org.freedesktop.DBus'",
                      &f->e);
  test_assert_no_error (&f->e);

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, call,
                                           DBUS_TIMEOUT_USE_DEFAULT);
  dbus_clear_message (&call);
  g_test_message ("TestSuiteForkingEchoService initial reply OK");
  dbus_clear_message (&reply);

  /* Now monitor for exits: when that happens, start it up again.
   * The goal here is to try to hit any race conditions in activation. */
  f->activation_forking_counter = 0;

  call = dbus_message_new_method_call (FORKING_ECHO_SERVICE,
                                       ECHO_SERVICE_PATH,
                                       ECHO_SERVICE_INTERFACE,
                                       "Exit");

  if (call == NULL || !dbus_connection_send (f->left_conn, call, NULL))
    g_error ("OOM");

  dbus_clear_message (&call);

  while (f->activation_forking_counter <= 10)
    test_main_context_iterate (f->ctx, TRUE);

  dbus_connection_remove_filter (f->left_conn,
                                 activation_forking_signal_filter, f);
}

/*
 * Helper for test_system_signals: Receive Foo signals and add them to
 * the held_messages queue.
 */
static DBusHandlerResult
foo_signal_filter (DBusConnection *connection,
                   DBusMessage *message,
                   void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_is_signal (message, ECHO_SERVICE_INTERFACE, "Foo"))
    g_queue_push_tail (&f->held_messages, dbus_message_ref (message));

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/*
 * Assert that the system bus(-like) configuration allows services
 * to emit signals, even if there is no service-specific configuration
 * to allow it.
 *
 * Essentially equivalent to the old test/name-test/test-wait-for-echo.py.
 */
static void
test_system_signals (Fixture *f,
                     gconstpointer context G_GNUC_UNUSED)
{
  DBusMessage *call = NULL;
  DBusMessage *response = NULL;

  g_test_bug ("18229");

  if (f->skip)
    return;

  if (!dbus_connection_add_filter (f->left_conn, foo_signal_filter,
                                   f, NULL))
    g_error ("OOM");

  dbus_bus_add_match (f->left_conn,
                      "interface='" ECHO_SERVICE_INTERFACE "'",
                      &f->e);
  test_assert_no_error (&f->e);

  call = dbus_message_new_method_call (ECHO_SERVICE,
                                       ECHO_SERVICE_PATH,
                                       ECHO_SERVICE_INTERFACE,
                                       "EmitFoo");

  if (call == NULL || !dbus_connection_send (f->left_conn, call, NULL))
    g_error ("OOM");

  dbus_clear_message (&call);

  while (g_queue_get_length (&f->held_messages) < 1)
    test_main_context_iterate (f->ctx, TRUE);

  g_test_message ("got signal");
  g_assert_cmpuint (g_queue_get_length (&f->held_messages), ==, 1);
  response = g_queue_pop_head (&f->held_messages);
  g_assert_cmpint (dbus_message_get_type (response), ==,
                   DBUS_MESSAGE_TYPE_SIGNAL);
  g_assert_cmpstr (dbus_message_get_interface (response), ==,
                   ECHO_SERVICE_INTERFACE);
  g_assert_cmpstr (dbus_message_get_path (response), ==,
                   ECHO_SERVICE_PATH);
  g_assert_cmpstr (dbus_message_get_signature (response), ==, "d");
  g_assert_cmpstr (dbus_message_get_member (response), ==, "Foo");
  dbus_clear_message (&response);

  dbus_connection_remove_filter (f->left_conn, foo_signal_filter, f);
}
#endif
#endif

static void
take_well_known_name (DBusConnection *conn,
                      const char *name,
                      DBusError *error,
                      int ownership_type)
{
  int ret = dbus_bus_request_name (conn, name, 0, error);
  test_assert_no_error (error);
  g_assert_cmpint (ret, ==, ownership_type);
}

static void
drop_well_known_name (DBusConnection *conn,
                      const char *name,
                      DBusError *error)
{
  int ret = dbus_bus_release_name (conn, name, error);
  test_assert_no_error (error);
  g_assert_cmpint (ret, ==, DBUS_RELEASE_NAME_REPLY_RELEASED);
}

static void
helper_send_destination_prefix_check (Fixture *f,
                                      const char *name,
                                      const char *interface,
                                      const char *member,
                                      dbus_bool_t allowed,
                                      const char *additional_name,
                                      int ownership_type)
{
  DBusMessage *call = NULL;
  DBusMessage *reply = NULL;

  take_well_known_name (f->right_conn, name, &f->e, ownership_type);

  if (additional_name)
    take_well_known_name (f->right_conn, additional_name, &f->e, ownership_type);

  call = dbus_message_new_method_call (dbus_bus_get_unique_name (f->right_conn),
                                       "/",
                                       interface,
                                       member);

  if (call == NULL)
    g_error ("OOM");

  reply = test_main_context_call_and_wait (f->ctx, f->left_conn, call,
                                           DBUS_TIMEOUT_USE_DEFAULT);
  dbus_clear_message (&call);
  g_test_message ("reply from %s(%d):%s OK", name, ownership_type, member);
  if (allowed)
    {
      g_test_message ("checking reply from %s for correct method_return", name);
      g_assert_cmpint (dbus_message_get_type (reply), ==,
                       DBUS_MESSAGE_TYPE_METHOD_RETURN);
    }
  else
    {
      g_test_message ("checking reply from %s for correct access_denied", name);
      g_assert_cmpint (dbus_message_get_type (reply), ==,
                       DBUS_MESSAGE_TYPE_ERROR);
      g_assert_cmpstr (dbus_message_get_error_name (reply), ==,
                       DBUS_ERROR_ACCESS_DENIED);
    }
  dbus_clear_message (&reply);

  drop_well_known_name (f->right_conn, name, &f->e);

  if (additional_name)
    drop_well_known_name (f->right_conn, additional_name, &f->e);
}

static void
helper_send_destination_prefix (Fixture *f,
                                const char *name,
                                const char *interface,
                                const char *member,
                                dbus_bool_t allowed,
                                const char *additional_name)
{
  /* check with primary ownership */
  helper_send_destination_prefix_check (f, name, interface, member, allowed, additional_name, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  /* check with queued ownership */
  take_well_known_name (f->left_conn, name, &f->e, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
  if (additional_name)
    take_well_known_name (f->left_conn, additional_name, &f->e, DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);

  helper_send_destination_prefix_check (f, name, interface, member, allowed, additional_name, DBUS_REQUEST_NAME_REPLY_IN_QUEUE);

  drop_well_known_name (f->left_conn, name, &f->e);
  if (additional_name)
    drop_well_known_name (f->left_conn, additional_name, &f->e);
}

static void
test_send_destination_prefix (Fixture *f,
                              gconstpointer context G_GNUC_UNUSED)
{
  if (f->skip)
    return;

  add_echo_filter (f);

  /*
   * Names are constructed with prefix foo.bar.test.dest_prefix followed by some of the tokens:
   * - a - allow send_destination for this name
   * - d - deny send_destination for this name
   * - ap - allow send_destination_prefix for this name
   * - dp - deny send_destination_prefix for this name
   * - f, f1, f2, f3 - fillers for generating names down the name hierarchy
   * - apf, dpf, ao, do - just some neighbour names
   * - m - names with 'm' have rules for interface and member
   * - apxdp, dpxap - names that have contradicting rules, e.g. for apxdp there are "allow send_destination_prefix"
   *   rules first, followed by "deny send_destination_prefix" rules
   */

  /* basic checks - base allow */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap",           "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f.f.f.f.f", "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.apf",          "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.apf.f.f.f.f",  "com.example.Anything", "Anything", FALSE, NULL);
  /* basic checks - base deny */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp",           "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f.f.f.f.f", "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dpf",          "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dpf.f.f.f.f",  "com.example.Anything", "Anything", FALSE, NULL);
  /* With interface and method in the policy:
   * everything is allowed, except foo.bar.a.CallDeny and whole foo.bar.d minus foo.bar.d.CallAllow.*/
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m", "foo.bar.a", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m", "foo.bar.a", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m", "foo.bar.a", "NonExistent", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m", "foo.bar.d", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m", "foo.bar.d", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m", "foo.bar.d", "NonExistent", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m", "foo.bar.none", "NonExistent", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m.f.f.f.f.f", "foo.bar.a", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m.f.f.f.f.f", "foo.bar.a", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m.f.f.f.f.f", "foo.bar.a", "NonExistent", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m.f.f.f.f.f", "foo.bar.d", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m.f.f.f.f.f", "foo.bar.d", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m.f.f.f.f.f", "foo.bar.d", "NonExistent", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.m.f.f.f.f.f", "foo.bar.none", "NonExistent", TRUE, NULL);
  /* With interface and method in the policy:
   * everything is denied, except foo.bar.d.CallAllow and whole foo.bar.a minus foo.bar.a.CallDeny */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m", "foo.bar.a", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m", "foo.bar.a", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m", "foo.bar.a", "NonExistent", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m", "foo.bar.d", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m", "foo.bar.d", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m", "foo.bar.d", "NonExistent", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m", "foo.bar.none", "NonExistent", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m.f.f.f.f.f", "foo.bar.a", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m.f.f.f.f.f", "foo.bar.a", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m.f.f.f.f.f", "foo.bar.a", "NonExistent", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m.f.f.f.f.f", "foo.bar.d", "CallDeny", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m.f.f.f.f.f", "foo.bar.d", "CallAllow", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m.f.f.f.f.f", "foo.bar.d", "NonExistent", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.m.f.f.f.f.f", "foo.bar.none", "NonExistent", FALSE, NULL);
  /* multiple names owned - everything is allowed */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ao",   "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.ap.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f", "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.ao");
  /* multiple names owned - mixed allow/deny, but denied wins */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f",   "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ap.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f",   "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.dp.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.do",     "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ap.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f",   "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.do");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ao",     "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.dp.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f",   "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ao");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f.f", "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ao.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ao.f",   "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.dp.f.f");
  /* multiple names owned - mixed allow/deny, but allowed wins */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f",       "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.ao.ao");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ao.ao",      "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.dp.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f",       "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.dp.f1.ap.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.f", "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.dp.f");
  /* multiple names owned - everything is denied */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f",   "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.do.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.do.f",   "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.dp.f");
  /* holes in default allow */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.d",          "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.dp",         "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.dp.f.f.f.f", "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.dp.f.f.f.f", "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ao");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.dp.f.f.f.f", "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ap");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ao",               "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ap.f1.dp.f.f.f.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap",               "com.example.Anything", "Anything", FALSE, "foo.bar.test.dest_prefix.ap.f1.dp.f.f.f.f");
  /* holes in holes in default allow */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.d.ap",          "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.d.ap.f.f.f.f",  "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.dp.ap",         "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.dp.ap.f.f.f.f", "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f1.dp.ap.a",       "com.example.Anything", "Anything", TRUE, NULL);
  /* redefinitions in default allow */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp",               "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.f.f.f.f",       "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.dp",            "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.dp.f.f.f.f",    "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.dp.ap",         "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.dp.ap.f.f.f.f", "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.dp.ap.d",       "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.dp.a",           "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.dp.ap.f.a",      "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.f.f.f.ap",       "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f2.apxdp.f.f.f.ap.f.f.f", "com.example.Anything", "Anything", TRUE, NULL);
  /* cancelled definitions in default allow */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap",                  "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.f.f.f.f",          "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.ap",               "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.ap.f.f.f",         "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.ap.dp",            "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.ap.dp.f.f.f.f",    "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.ap.dp.ap",         "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.ap.dp.ap.f.f.f.f", "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ap.f3.dpxap.ap.dp.a",          "com.example.Anything", "Anything", TRUE, NULL);
  /* holes in default deny */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.a",          "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.a.f.f.f.f",  "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap",         "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.f.f.f.f", "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.f.f.f",   "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.do");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.do",               "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.dp.f1.ap.f.f.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.f.f.f",   "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.do.f.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.do.f.f",           "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.dp.f1.ap.f.f.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.f.f",     "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.dp.f.f.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f.f.f",         "com.example.Anything", "Anything", TRUE, "foo.bar.test.dest_prefix.dp.f1.ap.f.f");
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.ao",               "com.example.Anything", "Anything", TRUE, NULL);
  /* holes in holes in default deny */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.a.dp",          "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.a.dp.f.f.f.f",  "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.dp",         "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.dp.f.f.f.f", "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.d",          "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f1.ap.d.f.f.f.f",  "com.example.Anything", "Anything", TRUE, NULL);
  /* redefinitions in default deny */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap",                "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.f.f.f.f",        "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap",             "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.f.f.f.f",     "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.dp",          "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.dp.f.f.f.f",  "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.dp.a",        "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.dp.a.f.f.f",  "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.d",           "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.d.f.f.f",     "com.example.Anything", "Anything", TRUE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.ap.dp.f.d",      "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.f.f.f.dp",       "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f2.dpxap.f.f.f.dp.f.f.f", "com.example.Anything", "Anything", FALSE, NULL);
  /* cancelled definitions in default deny */
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp",                  "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.f.f.f.f",          "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp",               "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp.f.f.f.f",       "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp.ap",            "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp.ap.f.f.f.f",    "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp.ap.dp",         "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp.ap.dp.f.f.f.f", "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp.ap.d",          "com.example.Anything", "Anything", FALSE, NULL);
  helper_send_destination_prefix (f, "foo.bar.test.dest_prefix.dp.f3.apxdp.dp.ap.d.f.f.f.f",  "com.example.Anything", "Anything", FALSE, NULL);
}

static void
teardown (Fixture *f,
    gconstpointer context G_GNUC_UNUSED)
{
  dbus_error_free (&f->e);
  g_clear_error (&f->ge);

  if (f->left_conn != NULL)
    {
      if (f->left_conn_shouted_signal_filter)
        {
          dbus_connection_remove_filter (f->left_conn, shouted_signal_filter, f);
          f->left_conn_shouted_signal_filter = FALSE;
        }

      test_connection_shutdown (f->ctx, f->left_conn);
      dbus_connection_close (f->left_conn);
    }

  if (f->right_conn != NULL)
    {
      GList *link;

      if (f->right_conn_echo)
        {
          dbus_connection_remove_filter (f->right_conn, echo_filter, f);
          f->right_conn_echo = FALSE;
        }

      if (f->right_conn_hold)
        {
          dbus_connection_remove_filter (f->right_conn, hold_filter, f);
          f->right_conn_hold = FALSE;
        }

      for (link = f->held_messages.head; link != NULL; link = link->next)
        dbus_message_unref (link->data);

      g_queue_clear (&f->held_messages);

      test_connection_shutdown (f->ctx, f->right_conn);
      dbus_connection_close (f->right_conn);
    }

  dbus_clear_connection (&f->left_conn);
  dbus_clear_connection (&f->right_conn);

  if (f->daemon_pid != 0)
    {
      test_kill_pid (f->daemon_pid);
      g_spawn_close_pid (f->daemon_pid);
      f->daemon_pid = 0;
    }

  if (f->tmp_runtime_dir != NULL)
    {
      gchar *path;

      /* the socket may exist */
      path = g_strdup_printf ("%s/bus", f->tmp_runtime_dir);

      test_remove_if_exists (path);
      g_free (path);
      /* there shouldn't be anything else in there */
      test_rmdir_must_exist (f->tmp_runtime_dir);

      /* we're relying on being single-threaded for this to be safe */
      if (f->saved_runtime_dir != NULL)
        g_setenv ("XDG_RUNTIME_DIR", f->saved_runtime_dir, TRUE);
      else
        g_unsetenv ("XDG_RUNTIME_DIR");
      g_free (f->saved_runtime_dir);
      g_free (f->tmp_runtime_dir);
    }

  test_main_context_unref (f->ctx);
  g_free (f->address);
}

static Config limited_config = {
    "34393", 10000, "valid-config-files/incoming-limit.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static Config finite_timeout_config = {
    NULL, 1, "valid-config-files/finite-timeout.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

#ifdef DBUS_UNIX
static Config listen_unix_runtime_config = {
    "61303", 1, "valid-config-files/listen-unix-runtime.conf",
    TEST_USER_ME, RELY_ON_DEFAULT
};
#endif

static Config max_completed_connections_config = {
    NULL, 1, "valid-config-files/max-completed-connections.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static Config max_replies_per_connection_config = {
    NULL, 1, "valid-config-files/max-replies-per-connection.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static Config max_match_rules_per_connection_config = {
    NULL, 1, "valid-config-files/max-match-rules-per-connection.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static Config max_names_per_connection_config = {
    NULL, 1, "valid-config-files/max-names-per-connection.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

#if defined(DBUS_UNIX) && defined(HAVE_UNIX_FD_PASSING) && defined(HAVE_GIO_UNIX)
static Config pending_fd_timeout_config = {
    NULL, 1, "valid-config-files/pending-fd-timeout.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static Config count_fds_config = {
    NULL, 1, "valid-config-files/count-fds.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};
#endif

#if defined(DBUS_UNIX)
static Config as_another_user_config = {
    NULL, 1, "valid-config-files/as-another-user.conf",
    /* We start the dbus-daemon as root and drop privileges, like the
     * real system bus does */
    TEST_USER_ROOT_DROP_TO_MESSAGEBUS, SPECIFY_ADDRESS
};

#ifdef ENABLE_TRADITIONAL_ACTIVATION
static Config tmp_session_config = {
    NULL, 1, "valid-config-files/tmp-session.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static Config nearly_system_config = {
    NULL, 1, "valid-config-files-system/tmp-session-like-system.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};
#endif
#endif

static Config send_destination_prefix_config = {
    NULL, 1, "valid-config-files/send-destination-prefix-rules.conf",
    TEST_USER_ME, SPECIFY_ADDRESS
};

static void
test_match_remove_fails (Fixture *f,
                         gconstpointer context G_GNUC_UNUSED)
{
  const char *match_rule = "type='signal'";

  if (f->skip)
    return;

  /* Unlike in test_match_remove_succeeds(), we never added this */
  dbus_bus_remove_match (f->left_conn, match_rule, &f->e);
  g_assert_cmpstr (f->e.name, ==, DBUS_ERROR_MATCH_RULE_NOT_FOUND);
}

static void
test_match_remove_succeeds (Fixture *f,
                            gconstpointer context G_GNUC_UNUSED)
{
  const char *match_rule = "type='signal'";

  if (f->skip)
    return;

  add_shouted_signal_filter (f);

  /* We use this to make sure that a method call from the "left" connection
   * will get a reply from the "right", to sync up */
  add_echo_filter (f);

  /* Emit a signal from the "right" connection, and assert that the "left"
   * does not receive it yet */
  f->signal_counter = 0;
  right_conn_emit_shouted (f);
  /* Because messages are totally-ordered, if the "left" connection was
   * going to receive the signal, it would receive it before it got
   * the reply from this async call to the "right" connection */
  echo_left_to_right (f, 1);
  g_assert_cmpuint (f->signal_counter, ==, 0);

  dbus_bus_add_match (f->left_conn, match_rule, &f->e);
  test_assert_no_error (&f->e);

  f->signal_counter = 0;

  /* Emit a signal from the "right" connection, and assert that the "left"
   * receives it */
  right_conn_emit_shouted (f);

  while (f->signal_counter < 1)
    test_main_context_iterate (f->ctx, TRUE);

  dbus_bus_remove_match (f->left_conn, match_rule, &f->e);
  test_assert_no_error (&f->e);

  /* Emit a signal from the "right" connection, and assert that the "left"
   * does not receive it this time */
  f->signal_counter = 0;
  right_conn_emit_shouted (f);
  /* Because messages are totally-ordered, if the "left" connection was
   * going to receive the signal, it would receive it before it got
   * the reply from this async call to the "right" connection */
  echo_left_to_right (f, 1);
  g_assert_cmpuint (f->signal_counter, ==, 0);
}

int
main (int argc,
    char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/echo/session", Fixture, NULL, setup, test_echo, teardown);
  g_test_add ("/echo/limited", Fixture, &limited_config,
      setup, test_echo, teardown);
  g_test_add ("/no-reply/disconnect", Fixture, NULL,
      setup, test_no_reply, teardown);
  g_test_add ("/no-reply/timeout", Fixture, &finite_timeout_config,
      setup, test_no_reply, teardown);
  g_test_add ("/creds", Fixture, NULL, setup, test_creds, teardown);
  g_test_add ("/processid", Fixture, NULL, setup, test_processid, teardown);
  g_test_add ("/canonical-path/uae", Fixture, NULL,
      setup, test_canonical_path_uae, teardown);
  g_test_add ("/limits/max-completed-connections", Fixture,
      &max_completed_connections_config,
      setup, test_max_connections, teardown);
  g_test_add ("/limits/max-connections-per-user", Fixture,
      &max_connections_per_user_config,
      setup, test_max_connections, teardown);
  g_test_add ("/limits/max-replies-per-connection", Fixture,
      &max_replies_per_connection_config,
      setup, test_max_replies_per_connection, teardown);
  g_test_add ("/limits/max-match-rules-per-connection", Fixture,
      &max_match_rules_per_connection_config,
      setup, test_max_match_rules_per_connection, teardown);
  g_test_add ("/limits/max-names-per-connection", Fixture,
      &max_names_per_connection_config,
      setup, test_max_names_per_connection, teardown);
  g_test_add ("/match/remove/fails", Fixture, NULL,
      setup, test_match_remove_fails, teardown);
  g_test_add ("/match/remove/succeeds", Fixture, NULL,
      setup, test_match_remove_succeeds, teardown);
  g_test_add ("/peer/ping", Fixture, NULL, setup, test_peer_ping, teardown);
  g_test_add ("/peer/get-machine-id", Fixture, NULL,
      setup, test_peer_get_machine_id, teardown);
  g_test_add ("/properties/get-invalid-iface", Fixture, NULL,
      setup, test_get_invalid_iface, teardown);
  g_test_add ("/properties/get-invalid-path", Fixture, NULL,
      setup, test_get_invalid_path, teardown);
  g_test_add ("/properties/get-invalid", Fixture, NULL,
      setup, test_get_invalid, teardown);
  g_test_add ("/properties/get-all-invalid-iface", Fixture, NULL, setup,
      test_get_all_invalid_iface, teardown);
  g_test_add ("/properties/get-all-invalid-path", Fixture, NULL, setup,
      test_get_all_invalid_path, teardown);
  g_test_add ("/properties/set-invalid-iface", Fixture, NULL,
      setup, test_set_invalid_iface, teardown);
  g_test_add ("/properties/set-invalid-path", Fixture, NULL,
      setup, test_set_invalid_path, teardown);
  g_test_add ("/properties/set-invalid", Fixture, NULL,
      setup, test_set_invalid, teardown);
  g_test_add ("/properties/set", Fixture, NULL,
      setup, test_set, teardown);
  g_test_add ("/properties/features", Fixture, NULL,
      setup, test_features, teardown);
  g_test_add ("/properties/interfaces", Fixture, NULL, setup,
      test_interfaces, teardown);
  g_test_add ("/properties/get-all", Fixture, NULL, setup,
      test_get_all, teardown);

#if defined(DBUS_UNIX) && defined(HAVE_UNIX_FD_PASSING) && defined(HAVE_GIO_UNIX)
  g_test_add ("/limits/pending-fd-timeout", Fixture,
      &pending_fd_timeout_config,
      setup, test_pending_fd_timeout, teardown);
  g_test_add ("/policy/count-fds", Fixture, &count_fds_config,
      setup, test_count_fds, teardown);
#endif

#ifdef DBUS_UNIX
  /* We can't test this in loopback.c with the rest of unix:runtime=yes,
   * because dbus_bus_get[_private] is the only way to use the default,
   * and that blocks on a round-trip to the dbus-daemon */
  g_test_add ("/unix-runtime-is-default", Fixture, &listen_unix_runtime_config,
      setup, test_echo, teardown);

  g_test_add ("/fd-limit/session", Fixture, NULL,
              setup, test_fd_limit, teardown);
  g_test_add ("/fd-limit/system", Fixture, &as_another_user_config,
              setup, test_fd_limit, teardown);

#ifdef ENABLE_TRADITIONAL_ACTIVATION
  g_test_add ("/activation/forking", Fixture, &tmp_session_config,
              setup, test_activation_forking, teardown);
  g_test_add ("/system-policy/allow-signals", Fixture, &nearly_system_config,
              setup, test_system_signals, teardown);
#endif
#endif

  g_test_add ("/system-policy/send-destination/prefix", Fixture, &send_destination_prefix_config,
              setup, test_send_destination_prefix, teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
