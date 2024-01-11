/* Manual regression test for syslog support
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2011 Nokia Corporation
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

#include <stdlib.h>

#include <glib.h>

#include <dbus/dbus.h>
#include <dbus/dbus-sysdeps.h>

#include "test-utils-glib.h"

typedef struct {
    int dummy;
} Fixture;

static void
setup (Fixture *f,
    gconstpointer data)
{
}

/* hopefully clear enough that people don't think these messages in syslog
 * are a bug */
#define MESSAGE "regression test for _dbus_log(): "

static void
test_syslog_normal (Fixture *f,
    gconstpointer data)
{
  if (g_test_subprocess ())
    {
      _dbus_init_system_log ("test-syslog",
          DBUS_LOG_FLAGS_SYSTEM_LOG | DBUS_LOG_FLAGS_STDERR);
      _dbus_log (DBUS_SYSTEM_LOG_INFO, MESSAGE "%d", 42);
      _dbus_log (DBUS_SYSTEM_LOG_WARNING, MESSAGE "%d", 45);
      _dbus_log (DBUS_SYSTEM_LOG_SECURITY, MESSAGE "%d", 666);
      _dbus_log (DBUS_SYSTEM_LOG_ERROR, MESSAGE "%d", 23);

      _dbus_init_system_log ("test-syslog-stderr", DBUS_LOG_FLAGS_STDERR);
      _dbus_log (DBUS_SYSTEM_LOG_INFO,
          MESSAGE "this should not appear in the syslog");
      _dbus_init_system_log ("test-syslog-both",
          DBUS_LOG_FLAGS_SYSTEM_LOG | DBUS_LOG_FLAGS_STDERR);
      _dbus_log (DBUS_SYSTEM_LOG_INFO,
          MESSAGE "this should appear in the syslog and on stderr");
      _dbus_init_system_log ("test-syslog-only", DBUS_LOG_FLAGS_SYSTEM_LOG);
      _dbus_log (DBUS_SYSTEM_LOG_INFO,
          MESSAGE "this should appear in the syslog only");

      exit (0);
    }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_passed ();
  /* Deliberately not matching newlines: in Windows they might come out as
   * either \n or \r\n, perhaps depending on GLib version or on whether
   * we're using Wine or real Windows. */
  g_test_trap_assert_stderr ("*" MESSAGE "42"
                             "*" MESSAGE "45"
                             "*" MESSAGE "666"
                             "*" MESSAGE "23"
                             "*test-syslog-stderr*" MESSAGE
                               "this should not appear in the syslog"
                             "*test-syslog-both*" MESSAGE
                               "this should appear in the syslog and "
                               "on stderr"
                             "*");
  g_test_trap_assert_stderr_unmatched ("*this should appear in the syslog "
                                       "only*");
  g_test_trap_assert_stderr_unmatched ("*test-syslog-only*");
}

static void
teardown (Fixture *f,
    gconstpointer data)
{
}

int
main (int argc,
    char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/syslog/normal", Fixture, NULL, setup, test_syslog_normal,
              teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
