/*
 * dbus-update-activation-environment - update D-Bus, and optionally
 * systemd, activation environment
 *
 * Copyright © 2014-2015 Collabora Ltd.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYSEXITS_H
# include <sysexits.h>
#endif

#include <dbus/dbus.h>

#ifdef DBUS_UNIX
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include "tool-common.h"

#define PROGNAME "dbus-update-activation-environment"

#ifndef EX_USAGE
# define EX_USAGE 64
#endif

#ifndef EX_UNAVAILABLE
# define EX_UNAVAILABLE 69
#endif

#ifndef EX_OSERR
# define EX_OSERR 71
#endif

#ifdef DBUS_WIN
/* The Windows C runtime uses a different name */
#define environ _environ
#elif defined(__APPLE__)
# include <crt_externs.h>
# define environ (*_NSGetEnviron ())
#elif HAVE_DECL_ENVIRON && defined(HAVE_UNISTD_H)
# include <unistd.h>
#else
extern char **environ;
#endif

/* we don't really have anything useful to say about the stage at which we
 * failed */
#define oom() tool_oom ("updating environment")

static dbus_bool_t verbose = FALSE;

static void say (const char *format, ...) _DBUS_GNUC_PRINTF (1, 2);

static void
say (const char *format,
    ...)
{
  va_list ap;

  if (!verbose)
    return;

  fprintf (stderr, "%s: ", PROGNAME);
  va_start (ap, format);
  vfprintf (stderr, format, ap);
  fputc ('\n', stderr);
  va_end (ap);
}

#ifdef __linux__
static dbus_bool_t
systemd_user_running (void)
{
  char *xdg_runtime_dir = getenv ("XDG_RUNTIME_DIR");
  char *path;
  struct stat buf;
  dbus_bool_t ret = FALSE;

  if (xdg_runtime_dir == NULL)
    return FALSE;

  /* Assume that XDG_RUNTIME_DIR/systemd exists if and only if
   * "systemd --user" is running. It's OK to use asprintf() here
   * because we know we're on Linux. */
  if (asprintf (&path, "%s/systemd", xdg_runtime_dir) < 0)
    oom ();

  if (stat (path, &buf) == 0)
    ret = TRUE;

  free (path);
  return ret;
}
#endif

int
main (int argc, char **argv)
{
  DBusConnection *conn;
  DBusMessage *msg;
  DBusMessage *reply;
  DBusError error = DBUS_ERROR_INIT;
  DBusMessageIter msg_iter;
  DBusMessageIter array_iter;
  int i;
  int first_non_option = argc;
  dbus_bool_t all = FALSE;
#ifdef __linux__
  DBusMessage *sd_msg = NULL;
  DBusMessageIter sd_msg_iter;
  DBusMessageIter sd_array_iter;
  dbus_bool_t systemd = FALSE;
#endif

  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] != '-')
        {
          first_non_option = i;
          break;
        }
      else if (strcmp (argv[i], "--") == 0)
        {
          first_non_option = i + 1;
          break;
        }
      else if (strcmp (argv[i], "--all") == 0)
        {
          all = TRUE;
        }
      else if (strcmp (argv[i], "--systemd") == 0)
        {
#ifdef __linux__
          systemd = TRUE;
#else
          say ("not on Linux, ignoring --systemd argument");
#endif
        }
      else if (strcmp (argv[i], "--verbose") == 0)
        {
          verbose = TRUE;
        }
      else
        {
          fprintf (stderr,
              "%1$s: update environment variables that will be set for D-Bus\n"
              "    session services\n"
              "\n"
              "%1$s [options] VAR[=VAL] [VAR2[=VAL2] ...]\n"
              "    Add specified variables to D-Bus activation environment.\n"
              "    If omitted, values are taken from current environment;\n"
              "    variables not found in the environment are ignored.\n"
              "%1$s --all\n"
              "    Add entire current environment to D-Bus activation\n"
              "    environment.\n"
              "\n"
              "Options:\n"
              "\n"
              "--all\n"
              "    Upload all environment variables.\n"
              "--systemd\n"
              "    Also update the 'systemd --user' environment\n"
              "    if possible.\n"
              "--verbose\n"
              "    Talk about it.\n"
              ,
              PROGNAME);
          exit (EX_USAGE);
        }
    }

  if (all && first_non_option < argc)
    {
      fprintf (stderr, "%s: error: --all cannot be used with VAR or "
               "VAR=VAL arguments\n", PROGNAME);
      exit (EX_USAGE);
    }

  conn = dbus_bus_get (DBUS_BUS_SESSION, &error);

  if (conn == NULL)
    {
      fprintf (stderr,
          "%s: error: unable to connect to D-Bus: %s\n", PROGNAME,
          error.message);
      exit (EX_OSERR);
    }

  msg = dbus_message_new_method_call (DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
      DBUS_INTERFACE_DBUS, "UpdateActivationEnvironment");

  if (msg == NULL)
    oom ();

  dbus_message_iter_init_append (msg, &msg_iter);

  if (!dbus_message_iter_open_container (&msg_iter, DBUS_TYPE_ARRAY,
      "{ss}", &array_iter))
    oom ();

#ifdef __linux__
  if (systemd)
    {
      if (!systemd_user_running ())
        {
          /* This is only best-effort. */
          say ("systemd --user not found, ignoring --systemd argument");
          systemd = FALSE;
        }
    }

  if (systemd)
    {
      sd_msg = dbus_message_new_method_call ("org.freedesktop.systemd1",
          "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager",
          "SetEnvironment");

      if (sd_msg == NULL)
        oom ();

      dbus_message_iter_init_append (sd_msg, &sd_msg_iter);

      if (!dbus_message_iter_open_container (&sd_msg_iter, DBUS_TYPE_ARRAY,
          "s", &sd_array_iter))
        oom ();
    }
#endif

  for (i = all ? 0 : first_non_option;
      all ? environ[i] != NULL : i < argc;
      i++)
    {
      const char *var;
      char *copy;
      char *eq;
      const char *val;
      DBusMessageIter pair_iter;

      if (all)
        var = environ[i];
      else
        var = argv[i];

      copy = strdup (var);

      if (copy == NULL)
        oom ();

      if (!dbus_validate_utf8 (var, NULL))
        {
          /* var is either of the form VAR or VAR=VAL */
          fprintf (stderr,
              "%s: warning: environment variable not UTF-8: %s\n",
              PROGNAME, var);
          goto next;
        }

      eq = strchr (copy, '=');

      if (eq == NULL)
        {
          if (all)
            {
              /* items in the environment block should be of the form
               * VAR=VAL */
              fprintf (stderr,
                  "%s: warning: environment variable without '=': %s\n",
                  PROGNAME, var);
              goto next;
            }
          else
            {
              /* items on the command-line may be of the form VAR
               * in which case we infer the value from the environment */
              val = getenv (var);

              if (val == NULL)
                {
                  /* nothing to be done here */
                  goto next;
                }

              if (!dbus_validate_utf8 (val, NULL))
                {
                  fprintf (stderr,
                      "%s: warning: environment variable not UTF-8: %s=%s\n",
                      PROGNAME, var, val);
                  goto next;
                }
            }
        }
      else
        {
          /* split VAR=VAL into VAR and VAL */
          *eq = '\0';
          val = eq + 1;
        }

#ifdef __linux__
      if (systemd)
        {
          char *combined;

          /* recombine if necessary */
          if (asprintf (&combined, "%s=%s", copy, val) < 0)
            oom ();

          if (!dbus_message_iter_append_basic (&sd_array_iter,
                DBUS_TYPE_STRING, &combined))
            oom ();

          free (combined);
        }
#endif

      if (!dbus_message_iter_open_container (&array_iter,
              DBUS_TYPE_DICT_ENTRY, NULL, &pair_iter))
        oom ();

      say ("setting %s=%s", copy, val);

      if (!dbus_message_iter_append_basic (&pair_iter, DBUS_TYPE_STRING,
              &copy))
        oom ();

      if (!dbus_message_iter_append_basic (&pair_iter, DBUS_TYPE_STRING,
              &val))
        oom ();

      if (!dbus_message_iter_close_container (&array_iter, &pair_iter))
        oom ();

next:
      free (copy);
    }

  if (!dbus_message_iter_close_container (&msg_iter, &array_iter))
    oom ();

#ifdef __linux__
  if (systemd &&
      !dbus_message_iter_close_container (&sd_msg_iter, &sd_array_iter))
    oom ();
#endif

  reply = dbus_connection_send_with_reply_and_block (conn, msg, -1, &error);

  if (reply == NULL)
    {
      fprintf (stderr,
          "%s: error sending to dbus-daemon: %s: %s\n",
          PROGNAME, error.name, error.message);
      exit (EX_UNAVAILABLE);
    }

  if (dbus_set_error_from_message (&error, msg) ||
      !dbus_message_get_args (msg, &error, DBUS_TYPE_INVALID))
    {
      fprintf (stderr,
          "%s: error from dbus-daemon: %s: %s\n",
          PROGNAME, error.name, error.message);
      exit (EX_UNAVAILABLE);
    }

  dbus_message_unref (reply);

#ifdef __linux__
  if (systemd)
    {
      reply = dbus_connection_send_with_reply_and_block (conn, sd_msg, -1,
          &error);

      /* non-fatal, the main purpose of this thing is to communicate
       * with dbus-daemon */
      if (reply == NULL)
        {
          fprintf (stderr,
              "%s: warning: error sending to systemd: %s: %s\n",
              PROGNAME, error.name, error.message);
        }
      else if (dbus_set_error_from_message (&error, msg) ||
          !dbus_message_get_args (msg, &error, DBUS_TYPE_INVALID))
        {
          fprintf (stderr,
              "%s: warning: error from systemd: %s: %s\n",
              PROGNAME, error.name, error.message);
        }

      if (reply != NULL)
        dbus_message_unref (reply);

      dbus_message_unref (sd_msg);
      dbus_error_free (&error);
    }
#endif

  dbus_message_unref (msg);
  dbus_connection_unref (conn);
  return 0;
}
