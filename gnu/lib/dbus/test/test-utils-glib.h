/* Utility functions for tests that rely on GLib
 *
 * Copyright © 2010-2011 Nokia Corporation
 * Copyright © 2013-2015 Collabora Ltd.
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

#ifndef TEST_UTILS_GLIB_H
#define TEST_UTILS_GLIB_H

#include <dbus/dbus.h>

#include <gio/gio.h>
#include <glib.h>

#include "test-utils.h"

/*
 * Multi-user support for regression tests run with root privileges in
 * a continuous integration system.
 *
 * A developer would normally run the tests as their own uid. Tests run
 * as TEST_USER_ME are run, and the others are skipped.
 *
 * In a CI system that has access to root privileges, most tests should still
 * be run as an arbitrary non-root user, as above.
 *
 * Certain tests can usefully be run again, as root. When this is done,
 * tests using a TestUser other than TEST_USER_ME can exercise situations
 * that only arise when there's more than one uid.
 */
typedef enum {
    /* Whatever user happens to be running the regression test;
     * such tests also work on Windows */
    TEST_USER_ME,
    /* Must be uid 0 on Unix; the test is skipped on Windows */
    TEST_USER_ROOT,
    /* The user who would normally run the system bus. This is the DBUS_USER
     * from configure.ac, usually 'messagebus' but perhaps 'dbus' or
     * '_dbus'. */
    TEST_USER_MESSAGEBUS,
    /* Run as uid 0, expecting to drop privileges to the user who would
     * normally run the system bus (so we must skip the test if that user
     * doesn't exist). Only valid for test_get_dbus_daemon(), not for
     * test_connect_to_bus_as_user(). */
    TEST_USER_ROOT_DROP_TO_MESSAGEBUS,
    /* An unprivileged user who is neither root nor DBUS_USER.
     * This is DBUS_TEST_USER from configure.ac, usually 'nobody'. */
    TEST_USER_OTHER
} TestUser;

#define test_assert_no_error(e) _test_assert_no_error (e, __FILE__, __LINE__)
void _test_assert_no_error (const DBusError *e,
    const char *file,
    int line);

gchar *test_get_dbus_daemon (const gchar *config_file,
    TestUser user,
    const gchar *runtime_dir,
    GPid *daemon_pid);

DBusConnection *test_connect_to_bus (TestMainContext *ctx,
    const gchar *address);
DBusConnection *test_try_connect_to_bus (TestMainContext *ctx,
    const gchar *address,
    GError **error);
DBusConnection *test_try_connect_to_bus_as_user (TestMainContext *ctx,
    const char *address,
    TestUser user,
    GError **error);
GDBusConnection *test_try_connect_gdbus_as_user (const char *address,
    TestUser user,
    GError **error);

void test_kill_pid (GPid pid);

void test_init (int *argcp, char ***argvp);

void test_progress (char symbol);

void test_remove_if_exists (const gchar *path);
void test_rmdir_must_exist (const gchar *path);
void test_rmdir_if_exists (const gchar *path);
void test_mkdir (const gchar *path, gint mode);

void test_timeout_reset (guint factor);

void test_oom (void) _DBUS_GNUC_NORETURN;

DBusMessage *test_main_context_call_and_wait (TestMainContext *ctx,
    DBusConnection *connection,
    DBusMessage *call,
    int timeout);

#if !GLIB_CHECK_VERSION(2, 44, 0)
#define g_steal_pointer(x) backported_g_steal_pointer (x)
/* A simplified version of g_steal_pointer without type-safety. */
static inline gpointer
backported_g_steal_pointer (gpointer pointer_to_pointer)
{
  gpointer *pp = pointer_to_pointer;
  gpointer ret;

  ret = *pp;
  *pp = NULL;
  return ret;
}
#endif

#ifndef g_assert_nonnull
#define g_assert_nonnull(a) g_assert (a != NULL)
#endif

gboolean test_check_tcp_works (void);

void test_store_result_cb (GObject *source_object,
                           GAsyncResult *result,
                           gpointer user_data);

void test_incomplete (const gchar *message);

gchar *test_get_helper_executable (const gchar *exe);

#endif
