/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-run-session.c - run a child process in its own session
 *
 * Copyright © 2003-2006 Red Hat, Inc.
 * Copyright © 2006 Thiago Macieira <thiago@kde.org>
 * Copyright © 2011-2012 Nokia Corporation
 * Copyright © 2018, 2021 Ralf Habacker
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <sys/types.h>
#ifdef DBUS_UNIX
#include <sys/wait.h>
#include <signal.h>
#include <dbus/dbus-sysdeps-unix.h>
#else
#include <dbus/dbus-internals.h>
#include <dbus/dbus-sysdeps-win.h>
#endif
#include "dbus/dbus.h"
#include "dbus/dbus-internals.h"

#include "tool-common.h"

#define MAX_ADDR_LEN 512
#define PIPE_READ_END  0
#define PIPE_WRITE_END 1

/* PROCESSES
 *
 * If you are in a shell and run "dbus-run-session myapp", here is what
 * happens (compare and contrast with dbus-launch):
 *
 * shell
 *   \- dbus-run-session myapp
 *      \- dbus-daemon --nofork --print-address --session
 *      \- myapp
 *
 * All processes are long-running.
 *
 * When myapp exits, dbus-run-session kills dbus-daemon and terminates.
 *
 * If dbus-daemon exits, dbus-run-session warns and continues to run.
 *
 * PIPES
 *
 * dbus-daemon --print-address -> bus_address_pipe -> d-r-s
 */

static const char me[] = "dbus-run-session";

static void usage (int ecode) _DBUS_GNUC_NORETURN;

static void
usage (int ecode)
{
  fprintf (stderr,
      "%s [OPTIONS] [--] PROGRAM [ARGUMENTS]\n"
      "%s --version\n"
      "%s --help\n"
      "\n"
      "Options:\n"
      "--dbus-daemon=BINARY       run BINARY instead of dbus-daemon\n"
      "--config-file=FILENAME     pass to dbus-daemon instead of --session\n"
      "\n",
      me, me, me);
  exit (ecode);
}

static void version (void) _DBUS_GNUC_NORETURN;

static void
version (void)
{
  printf ("%s %s\n"
          "Copyright (C) 2003-2006 Red Hat, Inc.\n"
          "Copyright (C) 2006 Thiago Macieira\n"
          "Copyright © 2011-2012 Nokia Corporation\n"
          "Copyright © 2018, 2021 Ralf Habacker\n"
          "\n"
          "This is free software; see the source for copying conditions.\n"
          "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
          me, VERSION);
  exit (0);
}

#ifndef DBUS_WIN
static void oom (void) _DBUS_GNUC_NORETURN;

static void
oom (void)
{
  fprintf (stderr, "%s: out of memory\n", me);
  exit (1);
}

typedef enum
{
  READ_STATUS_OK,    /**< Read succeeded */
  READ_STATUS_ERROR, /**< Some kind of error */
  READ_STATUS_EOF    /**< EOF returned */
} ReadStatus;

static ReadStatus
read_line (int        fd,
           char      *buf,
           size_t     maxlen)
{
  size_t bytes = 0;
  ReadStatus retval;

  memset (buf, '\0', maxlen);
  maxlen -= 1; /* ensure nul term */

  retval = READ_STATUS_OK;

  while (1)
    {
      ssize_t chunk;
      size_t to_read;

    again:
      to_read = maxlen - bytes;

      if (to_read == 0)
        break;

      chunk = read (fd,
                    buf + bytes,
                    to_read);
      if (chunk < 0 && errno == EINTR)
        goto again;

      if (chunk < 0)
        {
          retval = READ_STATUS_ERROR;
          break;
        }
      else if (chunk == 0)
        {
          retval = READ_STATUS_EOF;
          break; /* EOF */
        }
      else /* chunk > 0 */
        bytes += chunk;
    }

  if (retval == READ_STATUS_EOF &&
      bytes > 0)
    retval = READ_STATUS_OK;

  /* whack newline */
  if (retval != READ_STATUS_ERROR &&
      bytes > 0 &&
      buf[bytes-1] == '\n')
    buf[bytes-1] = '\0';

  return retval;
}

static void
exec_dbus_daemon (const char *dbus_daemon,
                  int         bus_address_pipe[2],
                  const char *config_file)
{
  /* Child process, which execs dbus-daemon or dies trying */
#define MAX_FD_LEN 64
  char write_address_fd_as_string[MAX_FD_LEN];

  close (bus_address_pipe[PIPE_READ_END]);

  /* Set all fds >= 3 close-on-execute, except for the one that can't be.
   * We don't want dbus-daemon to inherit random fds we might have
   * inherited from our caller. (Note that we *do* let the wrapped process
   * inherit them in exec_app(), in an attempt to be as close as possible
   * to being a transparent wrapper.) */
  _dbus_fd_set_all_close_on_exec ();
  _dbus_fd_clear_close_on_exec (bus_address_pipe[PIPE_WRITE_END]);

  sprintf (write_address_fd_as_string, "%d", bus_address_pipe[PIPE_WRITE_END]);

  execlp (dbus_daemon,
          dbus_daemon,
          "--nofork",
          "--print-address", write_address_fd_as_string,
          config_file ? "--config-file" : "--session",
          config_file, /* has to be last in this varargs list */
          NULL);

  fprintf (stderr, "%s: failed to execute message bus daemon '%s': %s\n",
           me, dbus_daemon, strerror (errno));
}

static void exec_app (int prog_arg, char **argv) _DBUS_GNUC_NORETURN;

static void
exec_app (int prog_arg, char **argv)
{
  execvp (argv[prog_arg], argv + prog_arg);

  fprintf (stderr, "%s: failed to exec '%s': %s\n", me, argv[prog_arg],
           strerror (errno));
  exit (1);
}

static int
run_session (const char *dbus_daemon,
             const char *config_file,
             char       *bus_address,
             char      **argv,
             int         prog_arg)
{
  pid_t bus_pid;
  pid_t app_pid;
  int bus_address_pipe[2] = { 0, 0 };

  if (pipe (bus_address_pipe) < 0)
    {
      fprintf (stderr, "%s: failed to create pipe: %s\n", me, strerror (errno));
      return 127;
    }

  /* Make sure our output buffers aren't redundantly printed by both the
   * parent and the child */
  fflush (stdout);
  fflush (stderr);

  bus_pid = fork ();

  if (bus_pid < 0)
    {
      fprintf (stderr, "%s: failed to fork: %s\n", me, strerror (errno));
      return 127;
    }

  if (bus_pid == 0)
    {
      /* child */
      exec_dbus_daemon (dbus_daemon, bus_address_pipe, config_file);
      /* not reached */
      return 127;
    }

  close (bus_address_pipe[PIPE_WRITE_END]);

  switch (read_line (bus_address_pipe[PIPE_READ_END], bus_address, MAX_ADDR_LEN))
    {
    case READ_STATUS_OK:
      break;

    case READ_STATUS_EOF:
      fprintf (stderr, "%s: EOF reading address from bus daemon\n", me);
      return 127;
      break;

    case READ_STATUS_ERROR:
      fprintf (stderr, "%s: error reading address from bus daemon: %s\n",
               me, strerror (errno));
      return 127;
      break;

    default:
      _dbus_assert_not_reached ("invalid read result");
    }

  close (bus_address_pipe[PIPE_READ_END]);

  if (!dbus_setenv ("DBUS_SESSION_BUS_ADDRESS", bus_address) ||
      !dbus_setenv ("DBUS_SESSION_BUS_PID", NULL) ||
      !dbus_setenv ("DBUS_SESSION_BUS_WINDOWID", NULL) ||
      !dbus_setenv ("DBUS_STARTER_ADDRESS", NULL) ||
      !dbus_setenv ("DBUS_STARTER_BUS_TYPE", NULL))
    oom ();

  fflush (stdout);
  fflush (stderr);

  app_pid = fork ();

  if (app_pid < 0)
    {
      fprintf (stderr, "%s: failed to fork: %s\n", me, strerror (errno));
      return 127;
    }

  if (app_pid == 0)
    {
      /* child */
      exec_app (prog_arg, argv);
      /* not reached */
      return 127;
    }

  while (1)
    {
      int child_status;
      pid_t child_pid = waitpid (-1, &child_status, 0);

      if (child_pid == (pid_t) -1)
        {
          int errsv = errno;

          if (errsv == EINTR)
            continue;

          /* shouldn't happen: the only other documented errors are ECHILD,
           * which shouldn't happen because we terminate when all our children
           * have died, and EINVAL, which would indicate programming error */
          fprintf (stderr, "%s: waitpid() failed: %s\n", me, strerror (errsv));
          return 127;
        }
      else if (child_pid == bus_pid)
        {
          /* no need to kill it, now */
          bus_pid = 0;

          if (WIFEXITED (child_status))
            fprintf (stderr, "%s: dbus-daemon exited with code %d\n",
                me, WEXITSTATUS (child_status));
          else if (WIFSIGNALED (child_status))
            fprintf (stderr, "%s: dbus-daemon terminated by signal %d\n",
                me, WTERMSIG (child_status));
          else
            fprintf (stderr, "%s: dbus-daemon died or something\n", me);
        }
      else if (child_pid == app_pid)
        {
          if (bus_pid != 0)
            kill (bus_pid, SIGTERM);

          if (WIFEXITED (child_status))
            return WEXITSTATUS (child_status);

          /* if it died from a signal, behave like sh(1) */
          if (WIFSIGNALED (child_status))
            return 128 + WTERMSIG (child_status);

          /* I give up (this should never be reached) */
          fprintf (stderr, "%s: child process died or something\n", me);
          return 127;
        }
      else
        {
          fprintf (stderr, "%s: ignoring unknown child process %ld\n", me,
              (long) child_pid);
        }
    }

  return 0;
}
#else
static int
run_session (const char *dbus_daemon,
             const char *config_file,
             char       *bus_address,
             char      **argv,
             int         prog_arg)
{
  char *dbus_daemon_argv[5];
  int ret = 127;
  HANDLE server_handle = NULL;
  HANDLE app_handle = NULL;
  HANDLE ready_event_handle = NULL;
  DWORD exit_code;
  DBusString argv_strings[4];
  DBusString address;
  char **env = NULL;
  DBusHashTable *env_table = NULL;
  long sec,usec;
  dbus_bool_t result = TRUE;
  char *key = NULL;
  char *value = NULL;
  DBusError error;

  if (!_dbus_string_init (&argv_strings[0]))
    result = FALSE;
  if (!_dbus_string_init (&argv_strings[1]))
    result = FALSE;
  if (!_dbus_string_init (&argv_strings[2]))
    result = FALSE;
  if (!_dbus_string_init (&argv_strings[3]))
    result = FALSE;
  if (!_dbus_string_init (&address))
    result = FALSE;
  if (!result)
    goto out;

  /* The handle of this event is used by the dbus daemon
   * to signal that connections are ready. */
  dbus_error_init (&error);
  ready_event_handle = _dbus_win_event_create_inheritable (&error);
  if (ready_event_handle == NULL)
    goto out;

  /* run dbus daemon */
  _dbus_get_real_time (&sec, &usec);
  /* On Windows it's difficult to make use of --print-address to
   * convert a listenable address into a connectable address, so instead
   * we tell the temporary dbus-daemon to use the Windows autolaunch
   * mechanism, with a unique scope that is shared by this dbus-daemon,
   * the app process that defines its lifetime, and any other child
   * processes they might have. */
  _dbus_string_append_printf (&address, "autolaunch:scope=dbus-tmp-session-%ld%ld-" DBUS_PID_FORMAT, sec, usec, _dbus_getpid ());
  _dbus_string_append_printf (&argv_strings[0], "%s", dbus_daemon);
  if (config_file != NULL)
    _dbus_string_append_printf (&argv_strings[1], "--config-file=%s", config_file);
  else
    _dbus_string_append_printf (&argv_strings[1], "--session");
  _dbus_string_append_printf (&argv_strings[2], "--address=%s", _dbus_string_get_const_data (&address));
  _dbus_string_append_printf (&argv_strings[3], "--ready-event-handle=%p", ready_event_handle);
  dbus_daemon_argv[0] = _dbus_string_get_data (&argv_strings[0]);
  dbus_daemon_argv[1] = _dbus_string_get_data (&argv_strings[1]);
  dbus_daemon_argv[2] = _dbus_string_get_data (&argv_strings[2]);
  dbus_daemon_argv[3] = _dbus_string_get_data (&argv_strings[3]);
  dbus_daemon_argv[4] = NULL;

  server_handle = _dbus_spawn_program (dbus_daemon, dbus_daemon_argv, NULL, TRUE, &error);
  if (server_handle == NULL)
    goto out;

  /* wait until dbus-daemon is ready for connections */
  if (ready_event_handle != NULL)
    {
      DWORD status;
      HANDLE events[2];

      _dbus_verbose ("Wait until dbus-daemon is ready for connections (event handle %p)\n", ready_event_handle);

      events[0] = ready_event_handle;
      events[1] = server_handle;
      status = WaitForMultipleObjects (2, events, FALSE, 30000);

      switch (status)
        {
          case WAIT_OBJECT_0:
            /* ready event signalled, everything is okay */
            break;

          case WAIT_OBJECT_0 + 1:
            /* dbus-daemon process has exited */
            dbus_set_error (&error, DBUS_ERROR_SPAWN_CHILD_EXITED, "dbus-daemon exited before signalling ready");
            goto out;

          case WAIT_FAILED:
            _dbus_win_set_error_from_last_error (&error, "Unable to wait for server readiness (handle %p)", ready_event_handle);
            goto out;

          case WAIT_TIMEOUT:
            /* GetLastError() is not set */
            dbus_set_error (&error, DBUS_ERROR_TIMEOUT, "Timed out waiting for server readiness or exit (handle %p)", ready_event_handle);
            goto out;

          default:
            /* GetLastError() is probably not set? */
            dbus_set_error (&error, DBUS_ERROR_FAILED, "Unknown result '%lu' while waiting for server readiness (handle %p)", status, ready_event_handle);
            goto out;
        }
      _dbus_verbose ("Got signal that dbus-daemon is ready for connections\n");
    }

  /* run app */
  env = _dbus_get_environment ();
  env_table = _dbus_hash_table_new (DBUS_HASH_STRING,
                                    dbus_free,
                                    dbus_free);
  if (!_dbus_hash_table_from_array (env_table, env, '='))
    {
      goto out;
    }

  /* replace DBUS_SESSION_BUS_ADDRESS in environment */
  if (!_dbus_string_steal_data (&address, &value))
    goto out;

  key = _dbus_strdup ("DBUS_SESSION_BUS_ADDRESS");

  if (key == NULL)
    goto out;

  if (_dbus_hash_table_insert_string (env_table, key, value))
    {
      /* env_table took ownership, do not free separately */
      key = NULL;
      value = NULL;
    }
  else
    {
      /* we still own key and value, the cleanup code will free them */
      goto out;
    }

  _dbus_hash_table_remove_string (env_table, "DBUS_STARTER_ADDRESS");
  _dbus_hash_table_remove_string (env_table, "DBUS_STARTER_BUS_TYPE");
  _dbus_hash_table_remove_string (env_table, "DBUS_SESSION_BUS_PID");
  _dbus_hash_table_remove_string (env_table, "DBUS_SESSION_BUS_WINDOWID");

  dbus_free_string_array (env);
  env = _dbus_hash_table_to_array (env_table, '=');
  if (!env)
    goto out;

  app_handle = _dbus_spawn_program (argv[prog_arg], argv + prog_arg, env, FALSE, &error);
  if (app_handle == NULL)
    goto out;

  WaitForSingleObject (app_handle, INFINITE);
  if (!GetExitCodeProcess (app_handle, &exit_code))
    {
      _dbus_win_set_error_from_last_error (&error, "Could not fetch exit code");
      goto out;
    }
  ret = exit_code;

out:
  if (dbus_error_is_set (&error))
    tool_stderr_error (me, &error);
  dbus_error_free (&error);
  TerminateProcess (server_handle, 0);
  if (server_handle != NULL)
    CloseHandle (server_handle);
  if (app_handle != NULL)
    CloseHandle (app_handle);
  if (ready_event_handle != NULL)
    _dbus_win_event_free (ready_event_handle, NULL);
  _dbus_string_free (&argv_strings[0]);
  _dbus_string_free (&argv_strings[1]);
  _dbus_string_free (&argv_strings[2]);
  _dbus_string_free (&argv_strings[3]);
  _dbus_string_free (&address);
  dbus_free_string_array (env);
  if (env_table != NULL)
    _dbus_hash_table_unref (env_table);
  dbus_free (key);
  dbus_free (value);
  return ret;
}
#endif

int
main (int argc, char **argv)
{
  int prog_arg = 0;
  const char *config_file = NULL;
  const char *dbus_daemon = NULL;
  char bus_address[MAX_ADDR_LEN] = { 0 };
  const char *prev_arg = NULL;
  int i = 1;
  int requires_arg = 0;

  while (i < argc)
    {
      const char *arg = argv[i];

      if (requires_arg)
        {
          const char **arg_dest;

          assert (prev_arg != NULL);

          if (strcmp (prev_arg, "--config-file") == 0)
            {
              arg_dest = &config_file;
            }
          else if (strcmp (prev_arg, "--dbus-daemon") == 0)
            {
              arg_dest = &dbus_daemon;
            }
          else
            {
              /* shouldn't happen */
              fprintf (stderr, "%s: internal error: %s not fully implemented\n",
                       me, prev_arg);
              return 127;
            }

          if (*arg_dest != NULL)
            {
              fprintf (stderr, "%s: %s given twice\n", me, prev_arg);
              return 127;
            }

          *arg_dest = arg;
          requires_arg = 0;
          prev_arg = arg;
          ++i;
          continue;
        }

      if (strcmp (arg, "--help") == 0 ||
          strcmp (arg, "-h") == 0 ||
          strcmp (arg, "-?") == 0)
        {
          usage (0);
        }
      else if (strcmp (arg, "--version") == 0)
        {
          version ();
        }
      else if (strstr (arg, "--config-file=") == arg)
        {
          const char *file;

          if (config_file != NULL)
            {
              fprintf (stderr, "%s: --config-file given twice\n", me);
              return 127;
            }

          file = strchr (arg, '=');
          ++file;

          config_file = file;
        }
      else if (strstr (arg, "--dbus-daemon=") == arg)
        {
          const char *file;

          if (dbus_daemon != NULL)
            {
              fprintf (stderr, "%s: --dbus-daemon given twice\n", me);
              return 127;
            }

          file = strchr (arg, '=');
          ++file;

          dbus_daemon = file;
        }
      else if (strcmp (arg, "--config-file") == 0 ||
               strcmp (arg, "--dbus-daemon") == 0)
        {
          requires_arg = 1;
        }
      else if (arg[0] == '-')
        {
          if (strcmp (arg, "--") != 0)
            {
              fprintf (stderr, "%s: option '%s' is unknown\n", me, arg);
              return 127;
            }
          else
            {
              prog_arg = i + 1;
              break;
            }
        }
      else
        {
          prog_arg = i;
          break;
        }

      prev_arg = arg;
      ++i;
    }

  /* "dbus-run-session" and "dbus-run-session ... --" are not allowed:
   * there must be something to run */
  if (prog_arg < 1 || prog_arg >= argc)
    {
      fprintf (stderr, "%s: a non-option argument is required\n", me);
      return 127;
    }

  if (requires_arg)
    {
      fprintf (stderr, "%s: option '%s' requires an argument\n", me, prev_arg);
      return 127;
    }

  if (dbus_daemon == NULL)
    dbus_daemon = "dbus-daemon";

  return run_session (dbus_daemon, config_file, bus_address, argv, prog_arg);
}
