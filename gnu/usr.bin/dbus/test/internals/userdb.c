/*
 * Copyright © 2023 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */

#include <config.h>

#include <glib.h>

#include <dbus/dbus.h>
#include "dbus/dbus-sysdeps.h"
#include "test-utils-glib.h"

#ifdef DBUS_UNIX
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "dbus/dbus-sysdeps-unix.h"
#include "dbus/dbus-userdb.h"
#endif

typedef struct
{
  int dummy;
} Fixture;

static void
setup (Fixture *f G_GNUC_UNUSED,
       gconstpointer context G_GNUC_UNUSED)
{
}

static void
test_groups_from_uid (Fixture *f,
                      gconstpointer context G_GNUC_UNUSED)
{
  DBusError error = DBUS_ERROR_INIT;
  dbus_gid_t *gids = NULL;
  int n_gids = -1;
  dbus_bool_t ret;
#ifdef DBUS_UNIX
  int i;
#endif

  /* We assume that uid 0 (root) is available on all Unix systems,
   * so this should succeed */
  ret = _dbus_unix_groups_from_uid (0, &gids, &n_gids, &error);

#ifdef DBUS_UNIX
  test_assert_no_error (&error);
  g_assert_true (ret);
  g_assert_cmpint (n_gids, >=, 0);

  g_test_message ("Groups of uid 0:");

  for (i = 0; i < n_gids; i++)
    {
      g_test_message ("[%d]: %ld", i, (long) gids[i]);
      g_assert_cmpint (gids[i], >=, 0);
    }
#else
  g_assert_cmpstr (error.name, ==, DBUS_ERROR_NOT_SUPPORTED);
  g_assert_false (ret);
  g_test_message ("Getting Unix groups on Windows failed as expected: %s: %s",
                  error.name, error.message);
  g_assert_null (gids);
  g_assert_cmpint (n_gids, <=, 0);
#endif

  dbus_free (gids);
  dbus_error_free (&error);

#ifdef DBUS_UNIX
  /* Assume that the current uid is something sensible */
  ret = _dbus_unix_groups_from_uid (geteuid (), &gids, &n_gids, &error);
  test_assert_no_error (&error);
  g_assert_true (ret);
  g_assert_cmpint (n_gids, >=, 0);

  g_test_message ("Groups of uid %ld:", (long) geteuid ());

  for (i = 0; i < n_gids; i++)
    {
      g_test_message ("[%d]: %ld", i, (long) gids[i]);
      g_assert_cmpint (gids[i], >=, 0);
    }

  g_test_message ("Total: %i groups", n_gids);

  dbus_free (gids);
  dbus_error_free (&error);

  errno = 0;

  /* arbitrarily chosen, probably isn't a valid uid */
  if (getpwuid (31337) == NULL)
    {
      g_test_message ("uid 31337 doesn't exist: %s",
                      errno == 0 ? "(no errno)" : g_strerror (errno));
      ret = _dbus_unix_groups_from_uid (31337, &gids, &n_gids, &error);
      g_assert_nonnull (error.name);
      g_assert_nonnull (error.message);
      g_assert_false (ret);
      g_test_message ("Getting groups from non-uid failed as expected: %s: %s",
                      error.name, error.message);
      /* The Unix implementation always clears gids/n_gids,
       * even on failure, and even if they were uninitialized */
      g_assert_null (gids);
      g_assert_cmpint (n_gids, ==, 0);

      dbus_free (gids);
      dbus_error_free (&error);
    }
  else
    {
      g_test_skip ("against our expectations, uid 31337 exists on this system");
    }
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

  g_test_add ("/userdb/groups_from_uid",
              Fixture, NULL, setup, test_groups_from_uid, teardown);

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
