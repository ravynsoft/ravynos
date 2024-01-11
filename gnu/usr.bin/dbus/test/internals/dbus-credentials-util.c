/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-credentials-util.c Would be in dbus-credentials.c, but only used for tests/bus
 *
 * Copyright (C) 2007 Red Hat Inc.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <config.h>

#include "misc-internals.h"

#include "dbus/dbus-credentials.h"
#include "dbus/dbus-internals.h"
#include "dbus/dbus-test-tap.h"
#include "dbus/dbus-test.h"

/**
 * @addtogroup DBusCredentials
 * @{
 */

/** @} */

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#include <stdio.h>
#include <string.h>

static DBusCredentials*
make_credentials(dbus_uid_t  unix_uid,
                 dbus_pid_t  pid,
                 int group_vector,
                 const char *windows_sid)
{
  DBusCredentials *credentials;
  static const struct
    {
      size_t n;
      const dbus_gid_t gids[4];
    }
  group_vectors[] =
    {
        { 4, { 1000, 42, 123, 5678 } },
        { 2, { 23, 1001 } },
        { 4, { 5678, 123, 42, 1000 } }
    };

  /*
   * group_vector is 0 to not add any groups, or n > 0 to add groups from
   * group_vectors[n-1].
   */
  _dbus_assert (group_vector >= 0);
  _dbus_assert (group_vector <= _DBUS_N_ELEMENTS (group_vectors));

  credentials = _dbus_credentials_new ();

  if (unix_uid != DBUS_UID_UNSET)
    {
      if (!_dbus_credentials_add_unix_uid (credentials, unix_uid))
        {
          _dbus_credentials_unref (credentials);
          return NULL;
        }
    }

  if (pid != DBUS_PID_UNSET)
    {
      if (!_dbus_credentials_add_pid (credentials, pid))
        {
          _dbus_credentials_unref (credentials);
          return NULL;
        }
    }

  if (group_vector)
    {
      dbus_gid_t *copy;

      copy = dbus_new0 (dbus_gid_t, group_vectors[group_vector - 1].n);

      if (copy == NULL)
        {
          _dbus_credentials_unref (credentials);
          return NULL;
        }

      memcpy (copy, group_vectors[group_vector - 1].gids,
              sizeof (dbus_gid_t) * group_vectors[group_vector - 1].n);

      _dbus_credentials_take_unix_gids (credentials, copy,
                                        group_vectors[group_vector - 1].n);
    }

  if (windows_sid != NULL)
    {
      if (!_dbus_credentials_add_windows_sid (credentials, windows_sid))
        {
          _dbus_credentials_unref (credentials);
          return NULL;
        }
    }

  return credentials;
}

#define SAMPLE_SID "whatever a windows sid looks like"
#define OTHER_SAMPLE_SID "whatever else"

dbus_bool_t
_dbus_credentials_test (const char *test_data_dir)
{
  DBusCredentials *creds;
  DBusCredentials *creds2;
  DBusString str;
  const dbus_gid_t *gids;
  size_t n;

  if (test_data_dir == NULL)
    return TRUE;

  creds = make_credentials (12, 511, 1, SAMPLE_SID);
  if (creds == NULL)
    _dbus_test_fatal ("oom");

  /* test refcounting */
  _dbus_credentials_ref (creds);
  _dbus_credentials_unref (creds);

  _dbus_assert (_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert (_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert (_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_GROUP_IDS));
  _dbus_assert (_dbus_credentials_include (creds, DBUS_CREDENTIAL_WINDOWS_SID));

  _dbus_assert (_dbus_credentials_get_unix_uid (creds) == 12);
  _dbus_assert (_dbus_credentials_get_pid (creds) == 511);
  _dbus_assert (strcmp (_dbus_credentials_get_windows_sid (creds), SAMPLE_SID) == 0);
  _dbus_assert (_dbus_credentials_get_unix_gids (creds, &gids, &n));
  _dbus_assert (n == 4);
  _dbus_assert (gids[0] == 42);
  _dbus_assert (gids[1] == 123);
  _dbus_assert (gids[2] == 1000);
  _dbus_assert (gids[3] == 5678);

  _dbus_assert (!_dbus_credentials_are_empty (creds));
  _dbus_assert (!_dbus_credentials_are_anonymous (creds));

  /* Test copy */
  creds2 = _dbus_credentials_copy (creds);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (_dbus_credentials_include (creds2, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert (_dbus_credentials_include (creds2, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert (_dbus_credentials_include (creds2, DBUS_CREDENTIAL_UNIX_GROUP_IDS));
  _dbus_assert (_dbus_credentials_include (creds2, DBUS_CREDENTIAL_WINDOWS_SID));

  _dbus_assert (_dbus_credentials_get_unix_uid (creds2) == 12);
  _dbus_assert (_dbus_credentials_get_pid (creds2) == 511);
  _dbus_assert (strcmp (_dbus_credentials_get_windows_sid (creds2), SAMPLE_SID) == 0);
  _dbus_assert (_dbus_credentials_get_unix_gids (creds2, &gids, &n));
  _dbus_assert (n == 4);
  _dbus_assert (gids[0] == 42);
  _dbus_assert (gids[1] == 123);
  _dbus_assert (gids[2] == 1000);
  _dbus_assert (gids[3] == 5678);

  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Same user if both unix and windows are the same */
  creds2 = make_credentials (12, DBUS_PID_UNSET, 0, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (_dbus_credentials_same_user (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Not the same user if Windows is missing */
  creds2 = make_credentials (12, DBUS_PID_UNSET, 0, NULL);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Not the same user if Windows is different */
  creds2 = make_credentials (12, DBUS_PID_UNSET, 0, OTHER_SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (!_dbus_credentials_are_superset (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Not the same user if Unix is missing */
  creds2 = make_credentials (DBUS_UID_UNSET, DBUS_PID_UNSET, 0, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Not the same user if Unix is different */
  creds2 = make_credentials (15, DBUS_PID_UNSET, 0, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (!_dbus_credentials_are_superset (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Not the same user if both are missing */
  creds2 = make_credentials (DBUS_UID_UNSET, DBUS_PID_UNSET, 0, NULL);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (!_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Same user, but not a superset, if groups are different */
  creds2 = make_credentials (12, 511, 2, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (!_dbus_credentials_are_superset (creds, creds2));

  _dbus_credentials_unref (creds2);

  /* Groups being in the same order make no difference */
  creds2 = make_credentials (12, 511, 3, SAMPLE_SID);
  if (creds2 == NULL)
    _dbus_test_fatal ("oom");

  _dbus_assert (_dbus_credentials_same_user (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds, creds2));
  _dbus_assert (_dbus_credentials_are_superset (creds2, creds));

  _dbus_credentials_unref (creds2);

  /* Clearing credentials works */
  _dbus_credentials_clear (creds);

  _dbus_assert (!_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_USER_ID));
  _dbus_assert (!_dbus_credentials_include (creds, DBUS_CREDENTIAL_UNIX_PROCESS_ID));
  _dbus_assert (!_dbus_credentials_include (creds, DBUS_CREDENTIAL_WINDOWS_SID));

  _dbus_assert (_dbus_credentials_get_unix_uid (creds) == DBUS_UID_UNSET);
  _dbus_assert (_dbus_credentials_get_pid (creds) == DBUS_PID_UNSET);
  _dbus_assert (_dbus_credentials_get_windows_sid (creds) == NULL);

  _dbus_assert (_dbus_credentials_are_empty (creds));
  _dbus_assert (_dbus_credentials_are_anonymous (creds));

  _dbus_credentials_unref (creds);

  /* Make some more realistic credentials blobs to test stringification */
  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("oom");

  creds = make_credentials (12, DBUS_PID_UNSET, 0, NULL);
  if (creds == NULL)
    _dbus_test_fatal ("oom");

  if (!_dbus_credentials_to_string_append (creds, &str))
    _dbus_test_fatal ("oom");

  _dbus_test_diag ("Unix uid only: %s", _dbus_string_get_const_data (&str));
  _dbus_assert (strcmp (_dbus_string_get_const_data (&str),
                        "uid=12") == 0);

  _dbus_credentials_unref (creds);

  creds = make_credentials (12, 511, 1, NULL);
  if (creds == NULL)
    _dbus_test_fatal ("oom");

  if (!_dbus_string_set_length (&str, 0))
    _dbus_test_fatal ("oom");

  if (!_dbus_credentials_to_string_append (creds, &str))
    _dbus_test_fatal ("oom");

  _dbus_test_diag ("Unix complete set: %s", _dbus_string_get_const_data (&str));
  _dbus_assert (strcmp (_dbus_string_get_const_data (&str),
                        "uid=12 pid=511 gid=42 gid=123 gid=1000 gid=5678") == 0);

  _dbus_credentials_unref (creds);

  creds = make_credentials (DBUS_UID_UNSET, DBUS_PID_UNSET, 0, SAMPLE_SID);
  if (creds == NULL)
    _dbus_test_fatal ("oom");

  if (!_dbus_string_set_length (&str, 0))
    _dbus_test_fatal ("oom");

  if (!_dbus_credentials_to_string_append (creds, &str))
    _dbus_test_fatal ("oom");

  _dbus_test_diag ("Windows sid only: %s", _dbus_string_get_const_data (&str));
  _dbus_assert (strcmp (_dbus_string_get_const_data (&str),
                        "sid=" SAMPLE_SID) == 0);

  _dbus_credentials_unref (creds);

  creds = make_credentials (DBUS_UID_UNSET, 511, 0, SAMPLE_SID);
  if (creds == NULL)
    _dbus_test_fatal ("oom");

  if (!_dbus_string_set_length (&str, 0))
    _dbus_test_fatal ("oom");

  if (!_dbus_credentials_to_string_append (creds, &str))
    _dbus_test_fatal ("oom");

  _dbus_test_diag ("Windows complete set: %s", _dbus_string_get_const_data (&str));
  _dbus_assert (strcmp (_dbus_string_get_const_data (&str),
                        "pid=511 sid=" SAMPLE_SID) == 0);

  _dbus_credentials_unref (creds);

  _dbus_string_free (&str);

  return TRUE;
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
