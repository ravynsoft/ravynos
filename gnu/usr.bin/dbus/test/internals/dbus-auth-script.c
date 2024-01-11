/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-auth-script.c Test DBusAuth using a special script file (internal to D-Bus implementation)
 *
 * Copyright (C) 2003 Red Hat, Inc.
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

#ifdef DBUS_ENABLE_EMBEDDED_TESTS

#include "misc-internals.h"

#include "dbus-auth-script.h"

#include <stdio.h>

#include "dbus/dbus-auth.h"
#include "dbus/dbus-credentials.h"
#include "dbus/dbus-hash.h"
#include "dbus/dbus-internals.h"
#include "dbus/dbus-string.h"
#include "dbus/dbus-test-tap.h"

#ifdef DBUS_UNIX
# include "dbus/dbus-userdb.h"
#endif

#include "test/test-utils.h"

/**
 * @defgroup DBusAuthScript code for running unit test scripts for DBusAuth
 * @ingroup  DBusInternals
 * @brief DBusAuth unit test scripting
 *
 * The code in here is used for unit testing, it loads
 * up a script that tests DBusAuth.
 *
 * @{
 */

/* this is slightly different from the other append_quoted_string
 * in dbus-message-builder.c
 */
static dbus_bool_t
append_quoted_string (DBusString       *dest,
                      const DBusString *quoted)
{
  dbus_bool_t in_quotes = FALSE;
  dbus_bool_t in_backslash = FALSE;
  int i;

  i = 0;
  while (i < _dbus_string_get_length (quoted))
    {
      unsigned char b;

      b = _dbus_string_get_byte (quoted, i);

      if (in_backslash)
        {
          unsigned char a;

          if (b == 'r')
            a = '\r';
          else if (b == 'n')
            a = '\n';
          else if (b == '\\')
            a = '\\';
          else
            {
              _dbus_warn ("bad backslashed byte %c", b);
              return FALSE;
            }

          if (!_dbus_string_append_byte (dest, a))
            return FALSE;

          in_backslash = FALSE;
        }
      else if (b == '\\')
        {
          in_backslash = TRUE;
        }
      else if (in_quotes)
        {
          if (b == '\'')
            in_quotes = FALSE;
          else
            {
              if (!_dbus_string_append_byte (dest, b))
                return FALSE;
            }
        }
      else
        {
          if (b == '\'')
            in_quotes = TRUE;
          else if (b == ' ' || b == '\n' || b == '\t')
            break; /* end on whitespace if not quoted */
          else
            {
              if (!_dbus_string_append_byte (dest, b))
                return FALSE;
            }
        }

      ++i;
    }

  return TRUE;
}

static dbus_bool_t
same_first_word (const DBusString *a,
                 const DBusString *b)
{
  int first_a_blank, first_b_blank;

  _dbus_string_find_blank (a, 0, &first_a_blank);
  _dbus_string_find_blank (b, 0, &first_b_blank);

  if (first_a_blank != first_b_blank)
    return FALSE;

  return _dbus_string_equal_len (a, b, first_a_blank);
}

static DBusAuthState
auth_state_from_string (const DBusString *str)
{
  if (_dbus_string_starts_with_c_str (str, "WAITING_FOR_INPUT"))
    return DBUS_AUTH_STATE_WAITING_FOR_INPUT;
  else if (_dbus_string_starts_with_c_str (str, "WAITING_FOR_MEMORY"))
    return DBUS_AUTH_STATE_WAITING_FOR_MEMORY;
  else if (_dbus_string_starts_with_c_str (str, "HAVE_BYTES_TO_SEND"))
    return DBUS_AUTH_STATE_HAVE_BYTES_TO_SEND;
  else if (_dbus_string_starts_with_c_str (str, "NEED_DISCONNECT"))
    return DBUS_AUTH_STATE_NEED_DISCONNECT;
  else if (_dbus_string_starts_with_c_str (str, "AUTHENTICATED"))
    return DBUS_AUTH_STATE_AUTHENTICATED;
  else
    return DBUS_AUTH_STATE_INVALID;
}

static const char*
auth_state_to_string (DBusAuthState state)
{
  switch (state)
    {
    case DBUS_AUTH_STATE_WAITING_FOR_INPUT:
      return "WAITING_FOR_INPUT";
    case DBUS_AUTH_STATE_WAITING_FOR_MEMORY:
      return "WAITING_FOR_MEMORY";
    case DBUS_AUTH_STATE_HAVE_BYTES_TO_SEND:
      return "HAVE_BYTES_TO_SEND";
    case DBUS_AUTH_STATE_NEED_DISCONNECT:
      return "NEED_DISCONNECT";
    case DBUS_AUTH_STATE_AUTHENTICATED:
      return "AUTHENTICATED";
    case DBUS_AUTH_STATE_INVALID:
      return "INVALID";
    default:
      break;
    }

  return "unknown";
}

static char **
split_string (DBusString *str)
{
  int i, j, k, count, end;
  char **array;

  end = _dbus_string_get_length (str);

  i = 0;
  _dbus_string_skip_blank (str, i, &i);
  for (count = 0; i < end; count++)
    {
      _dbus_string_find_blank (str, i, &i);
      _dbus_string_skip_blank (str, i, &i);
    }

  array = dbus_new0 (char *, count + 1);
  if (array == NULL)
    return NULL;

  i = 0;
  _dbus_string_skip_blank (str, i, &i);
  for (k = 0; k < count; k++)
    {
      _dbus_string_find_blank (str, i, &j);

      array[k] = dbus_malloc (j - i + 1);
      if (array[k] == NULL)
        {
          dbus_free_string_array (array);
          return NULL;
        }
      memcpy (array[k],
              _dbus_string_get_const_data_len (str, i, j - i), j - i);
      array[k][j - i] = '\0';

      _dbus_string_skip_blank (str, j, &i);
    }
  array[k] = NULL;

  return array;
}

static void
auth_set_unix_credentials(DBusAuth  *auth,
                          dbus_uid_t uid,
                          dbus_pid_t pid)
{
  DBusCredentials *credentials;

  credentials = _dbus_credentials_new ();
  if (credentials == NULL)
    _dbus_test_fatal ("no memory");

  if (uid != DBUS_UID_UNSET)
    {
      if (!_dbus_credentials_add_unix_uid (credentials, uid))
        _dbus_test_fatal ("no memory");
    }
  if (pid != DBUS_PID_UNSET)
    {
      if (!_dbus_credentials_add_pid (credentials, pid))
        _dbus_test_fatal ("no memory");
    }
  _dbus_auth_set_credentials (auth, credentials);

  _dbus_credentials_unref (credentials);
}

/**
 * Runs an "auth script" which is a script for testing the
 * authentication protocol. Scripts send and receive data, and then
 * include assertions about the state of both ends of the connection
 * after processing the data. A script succeeds if these assertions
 * hold.
 *
 * @param filename the file containing the script to run
 * @returns #TRUE if the script succeeds, #FALSE otherwise
 */
dbus_bool_t
_dbus_auth_script_run (const DBusString *filename)
{
  DBusString file;
  DBusError error = DBUS_ERROR_INIT;
  DBusString line;
  dbus_bool_t retval;
  int line_no;
  DBusAuth *auth;
  DBusString from_auth;
  DBusAuthState state;
  DBusString context;
  DBusString guid;

  retval = FALSE;
  auth = NULL;

  _dbus_string_init_const (&guid, "5fa01f4202cd837709a3274ca0df9d00");
  _dbus_string_init_const (&context, "org_freedesktop_test");

  if (!_dbus_string_init (&file))
    return FALSE;

  if (!_dbus_string_init (&line))
    {
      _dbus_string_free (&file);
      return FALSE;
    }

  if (!_dbus_string_init (&from_auth))
    {
      _dbus_string_free (&file);
      _dbus_string_free (&line);
      return FALSE;
    }

  if (!_dbus_file_get_contents (&file, filename, &error))    {
      _dbus_warn ("Getting contents of %s failed: %s",
                  _dbus_string_get_const_data (filename), error.message);
      dbus_error_free (&error);
      goto out;
    }

  state = DBUS_AUTH_STATE_NEED_DISCONNECT;
  line_no = 0;

 next_iteration:
  while (_dbus_string_pop_line (&file, &line))
    {
      line_no += 1;

      /* _dbus_warn ("%s", _dbus_string_get_const_data (&line)); */

      _dbus_string_delete_leading_blanks (&line);

      if (auth != NULL)
        {
          while ((state = _dbus_auth_do_work (auth)) ==
                 DBUS_AUTH_STATE_HAVE_BYTES_TO_SEND)
            {
              const DBusString *tmp;
              if (_dbus_auth_get_bytes_to_send (auth, &tmp))
                {
                  int count = _dbus_string_get_length (tmp);

                  if (_dbus_string_copy (tmp, 0, &from_auth,
                                         _dbus_string_get_length (&from_auth)))
                    _dbus_auth_bytes_sent (auth, count);
                }
            }
        }

      if (_dbus_string_get_length (&line) == 0)
        {
          /* empty line */
          goto next_iteration;
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "#"))
        {
          /* Ignore this comment */
          goto next_iteration;
        }
#ifdef DBUS_WIN
      else if (_dbus_string_starts_with_c_str (&line,
                                               "WIN_ONLY"))
        {
          /* Ignore this line */
          goto next_iteration;
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "UNIX_ONLY"))
        {
          /* skip this file */
          _dbus_test_diag ("skipping unix only auth script");
          retval = TRUE;
          goto out;
        }
#endif
#ifdef DBUS_UNIX
      else if (_dbus_string_starts_with_c_str (&line,
                                               "UNIX_ONLY"))
        {
          /* Ignore this line */
          goto next_iteration;
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "WIN_ONLY"))
        {
          /* skip this file */
          _dbus_test_diag ("skipping windows only auth script");
          retval = TRUE;
          goto out;
        }
#endif
      else if (_dbus_string_starts_with_c_str (&line,
                                               "CLIENT"))
        {
          DBusCredentials *creds;

          if (auth != NULL)
            {
              _dbus_warn ("already created a DBusAuth (CLIENT or SERVER given twice)");
              goto out;
            }

          auth = _dbus_auth_client_new ();
          if (auth == NULL)
            {
              _dbus_warn ("no memory to create DBusAuth");
              goto out;
            }

          /* test ref/unref */
          _dbus_auth_ref (auth);
          _dbus_auth_unref (auth);

          creds = _dbus_credentials_new_from_current_process ();
          if (creds == NULL)
            {
              _dbus_warn ("no memory for credentials");
              _dbus_auth_unref (auth);
              auth = NULL;
              goto out;
            }

          if (!_dbus_auth_set_credentials (auth, creds))
            {
              _dbus_warn ("no memory for setting credentials");
              _dbus_auth_unref (auth);
              auth = NULL;
              _dbus_credentials_unref (creds);
              goto out;
            }

          _dbus_credentials_unref (creds);
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "SERVER"))
        {
          DBusCredentials *creds;

          if (auth != NULL)
            {
              _dbus_warn ("already created a DBusAuth (CLIENT or SERVER given twice)");
              goto out;
            }

          auth = _dbus_auth_server_new (&guid);
          if (auth == NULL)
            {
              _dbus_warn ("no memory to create DBusAuth");
              goto out;
            }

          /* test ref/unref */
          _dbus_auth_ref (auth);
          _dbus_auth_unref (auth);

          creds = _dbus_credentials_new_from_current_process ();
          if (creds == NULL)
            {
              _dbus_warn ("no memory for credentials");
              _dbus_auth_unref (auth);
              auth = NULL;
              goto out;
            }

          if (!_dbus_auth_set_credentials (auth, creds))
            {
              _dbus_warn ("no memory for setting credentials");
              _dbus_auth_unref (auth);
              auth = NULL;
              _dbus_credentials_unref (creds);
              goto out;
            }

          _dbus_credentials_unref (creds);

          _dbus_auth_set_context (auth, &context);
        }
      else if (auth == NULL)
        {
          _dbus_warn ("must specify CLIENT or SERVER");
          goto out;

        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "NO_CREDENTIALS"))
        {
          auth_set_unix_credentials (auth, DBUS_UID_UNSET, DBUS_PID_UNSET);
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "ROOT_CREDENTIALS"))
        {
          auth_set_unix_credentials (auth, 0, DBUS_PID_UNSET);
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "SILLY_CREDENTIALS"))
        {
          auth_set_unix_credentials (auth, 4312, DBUS_PID_UNSET);
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "ALLOWED_MECHS"))
        {
          char **mechs;

          _dbus_string_delete_first_word (&line);
          mechs = split_string (&line);
          _dbus_auth_set_mechanisms (auth, (const char **) mechs);
          dbus_free_string_array (mechs);
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "SEND"))
        {
          DBusString to_send;

          _dbus_string_delete_first_word (&line);

          if (!_dbus_string_init (&to_send))
            {
              _dbus_warn ("no memory to allocate string");
              goto out;
            }

          if (!append_quoted_string (&to_send, &line))
            {
              _dbus_warn ("failed to append quoted string line %d",
                          line_no);
              _dbus_string_free (&to_send);
              goto out;
            }

          _dbus_verbose ("Sending '%s'\n", _dbus_string_get_const_data (&to_send));

          if (!_dbus_string_append (&to_send, "\r\n"))
            {
              _dbus_warn ("failed to append \\r\\n from line %d",
                          line_no);
              _dbus_string_free (&to_send);
              goto out;
            }

          /* Replace USERID_HEX with our username in hex */
          {
            int where;

            if (_dbus_string_find (&to_send, 0, "WRONG_USERID_HEX", &where))
              {
                /* This must be checked for before USERID_HEX, because
                 * that's a substring. */
                DBusString uid = _DBUS_STRING_INIT_INVALID;

                if (!_dbus_string_init (&uid) ||
                    !_dbus_test_append_different_uid (&uid))
                  {
                    _dbus_warn ("no memory for uid");
                    _dbus_string_free (&to_send);
                    _dbus_string_free (&uid);
                    goto out;
                  }

                _dbus_string_delete (&to_send, where,
                                     (int) strlen ("WRONG_USERID_HEX"));

                if (!_dbus_string_hex_encode (&uid, 0, &to_send, where))
                  {
                    _dbus_warn ("no memory to subst WRONG_USERID_HEX");
                    _dbus_string_free (&to_send);
                    _dbus_string_free (&uid);
                    goto out;
                  }

                _dbus_string_free (&uid);
              }
            else if (_dbus_string_find (&to_send, 0,
                                        "USERID_HEX", &where))
              {
                DBusString username;

                if (!_dbus_string_init (&username))
                  {
                    _dbus_warn ("no memory for userid");
                    _dbus_string_free (&to_send);
                    goto out;
                  }

                if (!_dbus_append_user_from_current_process (&username))
                  {
                    _dbus_warn ("no memory for userid");
                    _dbus_string_free (&username);
                    _dbus_string_free (&to_send);
                    goto out;
                  }

                _dbus_string_delete (&to_send, where, (int) strlen ("USERID_HEX"));

                if (!_dbus_string_hex_encode (&username, 0,
					      &to_send, where))
                  {
                    _dbus_warn ("no memory to subst USERID_HEX");
                    _dbus_string_free (&username);
                    _dbus_string_free (&to_send);
                    goto out;
                  }

                _dbus_string_free (&username);
              }
            else if (_dbus_string_find (&to_send, 0,
                                        "WRONG_USERNAME_HEX", &where))
              {
                /* This must be checked for before USERNAME_HEX, because
                 * that's a substring. */
#ifdef DBUS_UNIX
                DBusString username = _DBUS_STRING_INIT_INVALID;

                if (!_dbus_string_init (&username) ||
                    !_dbus_test_append_different_username (&username))
                  {
                    _dbus_warn ("no memory for username");
                    _dbus_string_free (&to_send);
                    _dbus_string_free (&username);
                    goto out;
                  }

                _dbus_string_delete (&to_send, where,
                                     (int) strlen ("WRONG_USERNAME_HEX"));

                if (!_dbus_string_hex_encode (&username, 0,
                                              &to_send, where))
                  {
                    _dbus_warn ("no memory to subst WRONG_USERNAME_HEX");
                    _dbus_string_free (&to_send);
                    _dbus_string_free (&username);
                    goto out;
                  }

                _dbus_string_free (&username);
#else
                /* No authentication mechanism uses the login name on
                 * Windows, so there's no point in it appearing in an
                 * auth script that is not UNIX_ONLY. */
                _dbus_warn ("WRONG_USERNAME_HEX cannot be used on Windows");
                _dbus_string_free (&to_send);
                goto out;
#endif
              }
            else if (_dbus_string_find (&to_send, 0,
                                        "USERNAME_HEX", &where))
              {
#ifdef DBUS_UNIX
                const DBusString *username;

                if (!_dbus_username_from_current_process (&username))
                  {
                    _dbus_warn ("no memory for username");
                    _dbus_string_free (&to_send);
                    goto out;
                  }

                _dbus_string_delete (&to_send, where, (int) strlen ("USERNAME_HEX"));

                if (!_dbus_string_hex_encode (username, 0,
					      &to_send, where))
                  {
                    _dbus_warn ("no memory to subst USERNAME_HEX");
                    _dbus_string_free (&to_send);
                    goto out;
                  }
#else
                /* No authentication mechanism uses the login name on
                 * Windows, so there's no point in it appearing in an
                 * auth script that is not UNIX_ONLY. */
                _dbus_warn ("USERNAME_HEX cannot be used on Windows");
                _dbus_string_free (&to_send);
                goto out;
#endif
              }
          }

          {
            DBusString *buffer;

            _dbus_auth_get_buffer (auth, &buffer);
            if (!_dbus_string_copy (&to_send, 0,
                                    buffer, _dbus_string_get_length (buffer)))
              {
                _dbus_warn ("not enough memory to call bytes_received, or can't add bytes to auth object already in end state");
                _dbus_string_free (&to_send);
                _dbus_auth_return_buffer (auth, buffer);
                goto out;
              }

            _dbus_auth_return_buffer (auth, buffer);
          }

          _dbus_string_free (&to_send);
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "EXPECT_STATE"))
        {
          DBusAuthState expected;

          _dbus_string_delete_first_word (&line);

          expected = auth_state_from_string (&line);
          if (expected < 0)
            {
              _dbus_warn ("bad auth state given to EXPECT_STATE");
              goto parse_failed;
            }

          if (expected != state)
            {
              _dbus_warn ("expected auth state %s but got %s on line %d",
                          auth_state_to_string (expected),
                          auth_state_to_string (state),
                          line_no);
              goto out;
            }
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "EXPECT_COMMAND"))
        {
          DBusString received;

          _dbus_string_delete_first_word (&line);

          if (!_dbus_string_init (&received))
            {
              _dbus_warn ("no mem to allocate string received");
              goto out;
            }

          if (!_dbus_string_pop_line (&from_auth, &received))
            {
              _dbus_warn ("no line popped from the DBusAuth being tested, expected command %s on line %d",
                          _dbus_string_get_const_data (&line), line_no);
              _dbus_string_free (&received);
              goto out;
            }

          if (!same_first_word (&received, &line))
            {
              _dbus_warn ("line %d expected command '%s' and got '%s'",
                          line_no,
                          _dbus_string_get_const_data (&line),
                          _dbus_string_get_const_data (&received));
              _dbus_string_free (&received);
              goto out;
            }

          _dbus_string_free (&received);
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "EXPECT_UNUSED"))
        {
          DBusString expected;
          const DBusString *unused;

          _dbus_string_delete_first_word (&line);

          if (!_dbus_string_init (&expected))
            {
              _dbus_warn ("no mem to allocate string expected");
              goto out;
            }

          if (!append_quoted_string (&expected, &line))
            {
              _dbus_warn ("failed to append quoted string line %d",
                          line_no);
              _dbus_string_free (&expected);
              goto out;
            }

          _dbus_auth_get_unused_bytes (auth, &unused);

          if (_dbus_string_equal (&expected, unused))
            {
              _dbus_auth_delete_unused_bytes (auth);
              _dbus_string_free (&expected);
            }
          else
            {
              _dbus_warn ("Expected unused bytes '%s' and have '%s'",
                          _dbus_string_get_const_data (&expected),
                          _dbus_string_get_const_data (unused));
              _dbus_string_free (&expected);
              goto out;
            }
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "EXPECT_HAVE_NO_CREDENTIALS"))
        {
          DBusCredentials *authorized_identity;

          authorized_identity = _dbus_auth_get_identity (auth);
          if (!_dbus_credentials_are_anonymous (authorized_identity))
            {
              _dbus_warn ("Expected anonymous login or failed login, but some credentials were authorized");
              goto out;
            }
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "EXPECT_HAVE_SOME_CREDENTIALS"))
        {
          DBusCredentials *authorized_identity;

          authorized_identity = _dbus_auth_get_identity (auth);
          if (_dbus_credentials_are_anonymous (authorized_identity))
            {
              _dbus_warn ("Expected to have some credentials, but we don't");
              goto out;
            }
        }
      else if (_dbus_string_starts_with_c_str (&line,
                                               "EXPECT"))
        {
          DBusString expected;

          _dbus_string_delete_first_word (&line);

          if (!_dbus_string_init (&expected))
            {
              _dbus_warn ("no mem to allocate string expected");
              goto out;
            }

          if (!append_quoted_string (&expected, &line))
            {
              _dbus_warn ("failed to append quoted string line %d",
                          line_no);
              _dbus_string_free (&expected);
              goto out;
            }

          if (_dbus_string_equal_len (&expected, &from_auth,
                                      _dbus_string_get_length (&expected)))
            {
              _dbus_string_delete (&from_auth, 0,
                                   _dbus_string_get_length (&expected));
              _dbus_string_free (&expected);
            }
          else
            {
              _dbus_warn ("Expected exact string '%s' and have '%s'",
                          _dbus_string_get_const_data (&expected),
                          _dbus_string_get_const_data (&from_auth));
              _dbus_string_free (&expected);
              goto out;
            }
        }
      else
        goto parse_failed;

      goto next_iteration; /* skip parse_failed */

    parse_failed:
      {
        _dbus_warn ("couldn't process line %d \"%s\"",
                    line_no, _dbus_string_get_const_data (&line));
        goto out;
      }
    }

  if (auth == NULL)
    {
      _dbus_warn ("Auth script is bogus, did not even have CLIENT or SERVER");
      goto out;
    }
  else if (state == DBUS_AUTH_STATE_AUTHENTICATED)
    {
      const DBusString *unused;

      _dbus_auth_get_unused_bytes (auth, &unused);

      if (_dbus_string_get_length (unused) > 0)
        {
          _dbus_warn ("did not expect unused bytes (scripts must specify explicitly if they are expected)");
          goto out;
        }
    }

  if (_dbus_string_get_length (&from_auth) > 0)
    {
      _dbus_warn ("script did not have EXPECT_ statements for all the data received from the DBusAuth");
      _dbus_warn ("Leftover data: %s", _dbus_string_get_const_data (&from_auth));
      goto out;
    }

  retval = TRUE;

 out:
  if (auth)
    _dbus_auth_unref (auth);

  _dbus_string_free (&file);
  _dbus_string_free (&line);
  _dbus_string_free (&from_auth);

  return retval;
}

/** @} */
#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
