/*
 * Copyright © 2018 Collabora Ltd.
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

#include <dbus/dbus.h>
#include "dbus/dbus-sysdeps.h"
#include "test-utils-glib.h"

typedef struct
{
  int dummy;
} Fixture;

static void
setup (Fixture *f G_GNUC_UNUSED,
       gconstpointer context G_GNUC_UNUSED)
{
}

/* In an unfortunate clash of terminology, GPid is actually a process
 * handle (a HANDLE, vaguely analogous to a file descriptor) on Windows.
 * For _dbus_command_for_pid() we need an actual process ID. */
#ifdef G_OS_UNIX
# include <errno.h>
# include <sys/wait.h>
# include <sys/types.h>

# define NO_PROCESS 0
# ifndef G_PID_FORMAT
#   define G_PID_FORMAT "i"
# endif

static unsigned long
get_pid (pid_t process)
{
  return process;
}

static void
terminate_and_wait (pid_t process)
{
  int ret;

  kill (process, SIGTERM);

  do
    {
      ret = waitpid (process, NULL, 0);
    }
  while (ret == -1 && errno == EINTR);
}
#else
# define NO_PROCESS NULL
# ifndef G_PID_FORMAT
#   define G_PID_FORMAT "p"
# endif

static unsigned long
get_pid (HANDLE process)
{
  return GetProcessId (process);
}

static void
terminate_and_wait (HANDLE process)
{
  TerminateProcess (process, 1);
  WaitForSingleObject (process, INFINITE);
}
#endif

static void
test_command_for_pid (Fixture *f,
                      gconstpointer context G_GNUC_UNUSED)
{
  gchar *argv[] = { NULL, NULL, NULL };
  GError *error = NULL;
  GPid process = NO_PROCESS;
  unsigned long pid;
  DBusError d_error = DBUS_ERROR_INIT;
  DBusString string;

  argv[0] = test_get_helper_executable ("test-sleep-forever" DBUS_EXEEXT);

  if (argv[0] == NULL)
    {
      g_test_skip ("DBUS_TEST_EXEC not set");
      return;
    }

  argv[1] = g_strdup ("bees");

  if (!g_spawn_async (NULL, argv, NULL,
                      (G_SPAWN_STDOUT_TO_DEV_NULL |
                       G_SPAWN_STDERR_TO_DEV_NULL |
                       G_SPAWN_DO_NOT_REAP_CHILD),
                      NULL, NULL,
                      &process, &error))
    g_error ("Unable to run %s: %s", argv[0], error->message);

  pid = get_pid (process);
  g_test_message ("Process ID of process handle %" G_PID_FORMAT ": %lu",
                  process, pid);

  if (!_dbus_string_init (&string))
    g_error ("out of memory");

  if (_dbus_command_for_pid (pid, &string, strlen (argv[0]) + 1024, &d_error))
    {
      gchar *expected;

      g_test_message ("Process %lu: \"%s\"", pid,
                      _dbus_string_get_const_data (&string));

      expected = g_strdup_printf ("%s %s", argv[0], argv[1]);

      g_assert_cmpstr (_dbus_string_get_const_data (&string), ==,
                       expected);
      g_assert_cmpuint (_dbus_string_get_length (&string), ==,
                        strlen (expected));
      g_free (expected);
    }
  else
    {
      g_test_message ("Unable to get command for process %lu: %s: %s",
                      pid, d_error.name, d_error.message);
      g_assert_nonnull (d_error.name);
      g_assert_nonnull (d_error.message);
      dbus_error_free (&d_error);
    }

  if (!_dbus_string_set_length (&string, 0))
    g_error ("out of memory");

  /* Test truncation */
  if (_dbus_command_for_pid (pid, &string, 10, NULL))
    {
      gchar *expected;

      g_test_message ("Process %lu (truncated): \"%s\"", pid,
                      _dbus_string_get_const_data (&string));

      expected = g_strdup_printf ("%s %s", argv[0], argv[1]);
      expected[10] = '\0';

      g_assert_cmpstr (_dbus_string_get_const_data (&string), ==,
                       expected);
      g_assert_cmpuint (_dbus_string_get_length (&string), ==, 10);
      g_free (expected);
    }
  else
    {
      g_test_message ("Unable to get command for process %lu", pid);
    }

  if (process != NO_PROCESS)
    terminate_and_wait (process);

  _dbus_string_free (&string);
  g_spawn_close_pid (process);
  g_free (argv[1]);
  g_free (argv[0]);
}

static void
teardown (Fixture *f G_GNUC_UNUSED,
          gconstpointer context G_GNUC_UNUSED)
{
}

int
main (int argc,
      char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/sysdeps/command_for_pid",
              Fixture, NULL, setup, test_command_for_pid, teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
