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
#include "dbus/dbus-internals.h"
#include "disable-crash-handling.h"
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

#if defined(DBUS_ENABLE_ASSERT) == defined(DBUS_DISABLE_ASSERT)
# error Macros contradict
#endif

#if defined(DBUS_ENABLE_CHECKS) == defined(DBUS_DISABLE_CHECKS)
# error Macros contradict
#endif

static void
test_assert (Fixture *f,
             gconstpointer context G_GNUC_UNUSED)
{
#ifdef DBUS_ENABLE_ASSERT
  if (!g_test_undefined ())
    {
      g_test_skip ("Not testing programming errors");
    }
  else if (g_test_subprocess ())
    {
      dbus_setenv ("DBUS_BLOCK_ON_ABORT", NULL);
      _dbus_disable_crash_handling ();
      _dbus_assert (42 == 23);
    }
  else
    {
      g_test_trap_subprocess (NULL, 0, 0);
      g_test_trap_assert_failed ();
      g_test_trap_assert_stderr ("*42 == 23*");
    }
#else
  g_test_skip ("Assertions disabled");
#endif
}

static void
test_assert_error_is_set (Fixture *f,
                          gconstpointer context G_GNUC_UNUSED)
{
  DBusError e = DBUS_ERROR_INIT;
  DBusError *ep = NULL;

#if defined(DBUS_ENABLE_ASSERT) && defined(DBUS_ENABLE_CHECKS)
  if (!g_test_undefined ())
    {
      g_test_skip ("Not testing programming errors");
    }
  else if (g_test_subprocess ())
    {
      dbus_setenv ("DBUS_BLOCK_ON_ABORT", NULL);
      _dbus_disable_crash_handling ();
      _DBUS_ASSERT_ERROR_IS_SET (&e);
    }
  else
    {
      g_test_trap_subprocess (NULL, 0, 0);
      g_test_trap_assert_failed ();
    }

  _DBUS_SET_OOM (&e);
  _DBUS_ASSERT_ERROR_IS_SET (&e);
  _DBUS_ASSERT_ERROR_IS_SET (ep);
  dbus_error_free (&e);
#else
  g_test_skip ("Assertions or checks disabled");
#endif
}

static void
test_assert_error_is_clear (Fixture *f,
                            gconstpointer context G_GNUC_UNUSED)
{
#if defined(DBUS_ENABLE_ASSERT) && defined(DBUS_ENABLE_CHECKS)
  DBusError e = DBUS_ERROR_INIT;
  DBusError *ep = NULL;

  if (!g_test_undefined ())
    {
      g_test_skip ("Not testing programming errors");
    }
  else if (g_test_subprocess ())
    {
      dbus_setenv ("DBUS_BLOCK_ON_ABORT", NULL);
      _dbus_disable_crash_handling ();
      _DBUS_SET_OOM (&e);
      _DBUS_ASSERT_ERROR_IS_CLEAR (&e);
    }
  else
    {
      g_test_trap_subprocess (NULL, 0, 0);
      g_test_trap_assert_failed ();
    }

  _DBUS_ASSERT_ERROR_IS_CLEAR (&e);
  _DBUS_ASSERT_ERROR_IS_CLEAR (ep);
#else
  g_test_skip ("Assertions or checks disabled");
#endif
}

static void
test_assert_error_xor_true (Fixture *f,
                            gconstpointer context G_GNUC_UNUSED)
{
#if defined(DBUS_ENABLE_ASSERT) && defined(DBUS_ENABLE_CHECKS)
  DBusError e = DBUS_ERROR_INIT;
  DBusError *ep = NULL;
  dbus_bool_t retval = TRUE;

  if (!g_test_undefined ())
    {
      g_test_skip ("Not testing programming errors");
    }
  else if (g_test_subprocess ())
    {
      dbus_setenv ("DBUS_BLOCK_ON_ABORT", NULL);
      _dbus_disable_crash_handling ();
      _DBUS_SET_OOM (&e);
      _DBUS_ASSERT_ERROR_XOR_BOOL (&e, retval);
    }
  else
    {
      g_test_trap_subprocess (NULL, 0, 0);
      g_test_trap_assert_failed ();
    }

  _DBUS_ASSERT_ERROR_XOR_BOOL (&e, retval);
  _DBUS_ASSERT_ERROR_XOR_BOOL (ep, retval);
#else
  g_test_skip ("Assertions or checks disabled");
#endif
}

static void
test_assert_error_xor_false (Fixture *f,
                             gconstpointer context G_GNUC_UNUSED)
{
#if defined(DBUS_ENABLE_ASSERT) && defined(DBUS_ENABLE_CHECKS)
  DBusError e = DBUS_ERROR_INIT;
  DBusError *ep = NULL;
  void *retval = NULL;

  if (!g_test_undefined ())
    {
      g_test_skip ("Not testing programming errors");
    }
  else if (g_test_subprocess ())
    {
      dbus_setenv ("DBUS_BLOCK_ON_ABORT", NULL);
      _dbus_disable_crash_handling ();
      _DBUS_ASSERT_ERROR_XOR_BOOL (&e, retval != NULL);
    }
  else
    {
      g_test_trap_subprocess (NULL, 0, 0);
      g_test_trap_assert_failed ();
    }

  _DBUS_SET_OOM (&e);
  _DBUS_ASSERT_ERROR_XOR_BOOL (&e, retval != NULL);
  _DBUS_ASSERT_ERROR_XOR_BOOL (ep, retval != NULL);
#else
  g_test_skip ("Assertions or checks disabled");
#endif
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

  g_test_add ("/assertions/assert",
              Fixture, NULL, setup, test_assert, teardown);
  g_test_add ("/assertions/assert_error_is_set",
              Fixture, NULL, setup, test_assert_error_is_set, teardown);
  g_test_add ("/assertions/assert_error_is_clear",
              Fixture, NULL, setup, test_assert_error_is_clear, teardown);
  g_test_add ("/assertions/assert_error_xor_true",
              Fixture, NULL, setup, test_assert_error_xor_true, teardown);
  g_test_add ("/assertions/assert_error_xor_false",
              Fixture, NULL, setup, test_assert_error_xor_false, teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
