/* Targeted unit tests for OOM paths in DBusServer
 *
 * Copyright © 2017 Collabora Ltd.
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

#include <string.h>

#include <glib.h>

#include <dbus/dbus.h>
#include "dbus/dbus-internals.h"
#include "dbus/dbus-pipe.h"
#include "dbus/dbus-server-socket.h"
#include "test-utils-glib.h"

/* Return TRUE if the right thing happens, but the right thing might include
 * OOM. */
static dbus_bool_t
test_new_server (void        *user_data,
                 dbus_bool_t  have_memory)
{
  const char *listen_address = user_data;
  DBusError error = DBUS_ERROR_INIT;
  DBusServer *server = NULL;
  dbus_bool_t result = FALSE;

#ifdef DBUS_WIN
  if (strstr (listen_address, "bind=*") != NULL)
    {
      g_test_skip ("bind=* not tested on Windows to avoid a firewall-exception request (dbus#64)");
      result = TRUE;
      goto out;
    }
#endif

  server = dbus_server_listen (listen_address, &error);

  if (server == NULL)
    goto out;

  result = TRUE;

out:
  if (have_memory || result)
    {
      test_assert_no_error (&error);
    }
  else
    {
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_NO_MEMORY);
      result = TRUE;
    }

  if (server != NULL)
    dbus_server_disconnect (server);

  dbus_clear_server (&server);
  dbus_error_free (&error);

  return result;
}

typedef struct
{
  const gchar *name;
  DBusTestMemoryFunction function;
  const void *data;
} OOMTestCase;

static void
test_oom_wrapper (gconstpointer data)
{
  const OOMTestCase *test = data;

  if ((g_str_has_prefix (test->data, "tcp:") ||
       g_str_has_prefix (test->data, "nonce-tcp:")) &&
      !test_check_tcp_works ())
    return;

  if (!_dbus_test_oom_handling (test->name, test->function,
                                (void *) test->data))
    {
      g_test_message ("OOM test failed");
      g_test_fail ();
    }
}

static GQueue *test_cases_to_free = NULL;

static void
add_oom_test (const gchar *name,
              DBusTestMemoryFunction function,
              const void *data)
{
  /* By using GLib memory allocation here, we avoid being affected by
   * dbus_shutdown() or contributing to
   * _dbus_get_malloc_blocks_outstanding() */
  OOMTestCase *test_case = g_new0 (OOMTestCase, 1);

  test_case->name = name;
  test_case->function = function;
  test_case->data = data;
  g_test_add_data_func (name, test_case, test_oom_wrapper);
  g_queue_push_tail (test_cases_to_free, test_case);
}

int
main (int argc,
      char **argv)
{
  int ret;

  test_init (&argc, &argv);

  test_cases_to_free = g_queue_new ();
  add_oom_test ("/server/new-tcp", test_new_server, "tcp:host=127.0.0.1,bind=127.0.0.1");
  add_oom_test ("/server/new-nonce-tcp", test_new_server, "nonce-tcp:host=127.0.0.1,bind=127.0.0.1");
  add_oom_test ("/server/new-tcp-star", test_new_server, "tcp:host=127.0.0.1,bind=*");
  add_oom_test ("/server/new-tcp-v4", test_new_server, "tcp:host=127.0.0.1,bind=127.0.0.1,family=ipv4");
#ifdef DBUS_UNIX
  add_oom_test ("/server/unix", test_new_server, "unix:tmpdir=/tmp");
#endif

  ret = g_test_run ();

  g_queue_free_full (test_cases_to_free, g_free);
  dbus_shutdown ();
  return ret;
}
