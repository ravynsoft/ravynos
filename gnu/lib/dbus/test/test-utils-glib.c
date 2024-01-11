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

#include <config.h>
#include "test-utils-glib.h"

#include <errno.h>
#include <string.h>

#ifdef DBUS_WIN
# include <io.h>
# include <windows.h>
#else
# include <netdb.h>
# include <netinet/in.h>
# include <signal.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <pwd.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include <dbus/dbus.h>

#include "dbus/dbus-valgrind-internal.h"

#ifdef G_OS_WIN
# define isatty(x) _isatty(x)
#endif

void
_test_assert_no_error (const DBusError *e,
    const char *file,
    int line)
{
  if (G_UNLIKELY (dbus_error_is_set (e)))
    g_error ("%s:%d: expected success but got error: %s: %s",
        file, line, e->name, e->message);
}

#ifdef DBUS_UNIX
static gboolean
can_become_user_or_skip (uid_t uid)
{
  gchar *message;
  pid_t child_pid;
  pid_t pid;
  int wstatus;

  /* We can't switch to the uid without affecting the whole process,
   * which we don't necessarily want to do, so try it in a child process. */
  child_pid = fork ();

  if (child_pid < 0)
    g_error ("fork: %s", g_strerror (errno));

  if (child_pid == 0)
    {
      /* Child process: try to become uid, exit 0 on success, exit with
       * status = errno on failure */

      if (setuid (uid) != 0)
        {
          /* make sure we report failure even if errno is wrong */
          if (errno == 0)
            errno = ENODATA;

          _exit (errno);
        }

      /* success */
      _exit (0);
    }

  /* Parent process: wait for child and report result */

  pid = waitpid (child_pid, &wstatus, 0);
  g_assert_cmpuint (child_pid, ==, pid);

  if (WIFEXITED (wstatus) && WEXITSTATUS (wstatus) == 0)
    return TRUE;

  if (WIFEXITED (wstatus))
    message = g_strdup_printf ("unable to become uid %lu: %s",
                               (unsigned long) uid,
                               g_strerror (WEXITSTATUS (wstatus)));
  else
    message = g_strdup_printf ("unable to become uid %lu: unknown wait status %d",
                               (unsigned long) uid,
                               wstatus);

  g_test_skip (message);
  g_free (message);
  return FALSE;
}

static void
child_setup (gpointer user_data)
{
  const struct passwd *pwd = user_data;
  uid_t uid = geteuid ();

  if (pwd == NULL || (pwd->pw_uid == uid && getuid () == uid))
    return;

  if (uid != 0)
    g_error ("not currently euid 0: %lu", (unsigned long) uid);

  if (setuid (pwd->pw_uid) != 0)
    g_error ("could not setuid (%lu): %s",
        (unsigned long) pwd->pw_uid, g_strerror (errno));

  uid = getuid ();

  if (uid != pwd->pw_uid)
    g_error ("after successful setuid (%lu) my uid is %ld",
        (unsigned long) pwd->pw_uid, (unsigned long) uid);

  uid = geteuid ();

  if (uid != pwd->pw_uid)
    g_error ("after successful setuid (%lu) my euid is %ld",
        (unsigned long) pwd->pw_uid, (unsigned long) uid);
}
#endif

static gchar *
spawn_dbus_daemon (const gchar *binary,
    const gchar *configuration,
    const gchar *listen_address,
    TestUser user,
    const gchar *runtime_dir,
    GPid *daemon_pid)
{
  GError *error = NULL;
  GString *address;
  gint address_fd;
  GPtrArray *argv;
  gchar **envp;
#ifdef DBUS_UNIX
  const struct passwd *pwd = NULL;
#endif

  if (user != TEST_USER_ME)
    {
#ifdef DBUS_UNIX
      if (getuid () != 0)
        {
          g_test_skip ("cannot use alternative uid when not uid 0");
          return NULL;
        }

      switch (user)
        {
          case TEST_USER_ROOT:
            break;

          case TEST_USER_ROOT_DROP_TO_MESSAGEBUS:
          case TEST_USER_MESSAGEBUS:
            pwd = getpwnam (DBUS_USER);

            if (pwd == NULL)
              {
                gchar *message = g_strdup_printf ("user '%s' does not exist",
                    DBUS_USER);

                g_test_skip (message);
                g_free (message);
                return NULL;
              }

            if (!can_become_user_or_skip (pwd->pw_uid))
              return NULL;

            if (user == TEST_USER_ROOT_DROP_TO_MESSAGEBUS)
              {
                /* Let the dbus-daemon start as root and drop privileges
                 * itself */
                pwd = NULL;
              }

            break;

          case TEST_USER_OTHER:
            pwd = getpwnam (DBUS_TEST_USER);

            if (pwd == NULL)
              {
                gchar *message = g_strdup_printf ("user '%s' does not exist",
                    DBUS_TEST_USER);

                g_test_skip (message);
                g_free (message);
                return NULL;
              }

            if (!can_become_user_or_skip (pwd->pw_uid))
              return NULL;

            break;

          case TEST_USER_ME:
            /* cannot get here, fall through */
          default:
            g_assert_not_reached ();
        }
#else
      g_test_skip ("cannot use alternative uid on Windows");
      return NULL;
#endif
    }

  envp = g_get_environ ();

  if (runtime_dir != NULL)
    envp = g_environ_setenv (envp, "XDG_RUNTIME_DIR", runtime_dir, TRUE);

  argv = g_ptr_array_new_with_free_func (g_free);
  g_ptr_array_add (argv, g_strdup (binary));
  g_ptr_array_add (argv, g_strdup (configuration));
  g_ptr_array_add (argv, g_strdup ("--nofork"));
  g_ptr_array_add (argv, g_strdup ("--print-address=1")); /* stdout */

  if (listen_address != NULL)
    g_ptr_array_add (argv, g_strdup (listen_address));

#ifdef DBUS_UNIX
  g_ptr_array_add (argv, g_strdup ("--systemd-activation"));
#endif

  g_ptr_array_add (argv, NULL);

  g_spawn_async_with_pipes (NULL, /* working directory */
      (gchar **) argv->pdata,
      envp,
      G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
#ifdef DBUS_UNIX
      child_setup, (gpointer) pwd,
#else
      NULL, NULL,
#endif
      daemon_pid,
      NULL, /* child's stdin = /dev/null */
      &address_fd,
      NULL, /* child's stderr = our stderr */
      &error);

  /* The other uid might not have access to our build directory if we
   * are building in /root or something */
  if (user != TEST_USER_ME &&
      g_getenv ("DBUS_TEST_UNINSTALLED") != NULL &&
      error != NULL &&
      error->domain == G_SPAWN_ERROR &&
      (error->code == G_SPAWN_ERROR_CHDIR ||
       error->code == G_SPAWN_ERROR_ACCES ||
       error->code == G_SPAWN_ERROR_PERM))
    {
      g_prefix_error (&error, "Unable to launch %s as other user: ",
          binary);
      g_test_skip (error->message);
      g_clear_error (&error);
      return NULL;
    }

  g_assert_no_error (error);

  g_ptr_array_free (argv, TRUE);
  g_strfreev (envp);

  address = g_string_new (NULL);

  /* polling until the dbus-daemon writes out its address is a bit stupid,
   * but at least it's simple, unlike dbus-launch... in principle we could
   * use select() here, but life's too short */
  while (1)
    {
      gssize bytes;
      gchar buf[4096];
      gchar *newline;

      bytes = read (address_fd, buf, sizeof (buf));

      if (bytes > 0)
        g_string_append_len (address, buf, bytes);

      newline = strchr (address->str, '\n');

      if (newline != NULL)
        {
          if ((newline > address->str) && ('\r' == newline[-1]))
            newline -= 1;
          g_string_truncate (address, newline - address->str);
          break;
        }

      g_usleep (G_USEC_PER_SEC / 10);
    }

  g_close (address_fd, NULL);

  return g_string_free (address, FALSE);
}

gchar *
test_get_dbus_daemon (const gchar *config_file,
                      TestUser     user,
                      const gchar *runtime_dir,
                      GPid        *daemon_pid)
{
  gchar *dbus_daemon;
  gchar *arg;
  const gchar *listen_address = NULL;
  gchar *address;

  /* we often have to override this because on Windows, the default may be
   * autolaunch:, which is globally-scoped and hence unsuitable for
   * regression tests */
  listen_address = "--address=" TEST_LISTEN;

  if (config_file != NULL)
    {

      if (g_getenv ("DBUS_TEST_DATA") == NULL)
        {
          g_test_message ("set DBUS_TEST_DATA to a directory containing %s",
              config_file);
          g_test_skip ("DBUS_TEST_DATA not set");
          return NULL;
        }

      arg = g_strdup_printf (
          "--config-file=%s/%s",
          g_getenv ("DBUS_TEST_DATA"), config_file);

      /* The configuration file is expected to give a suitable address,
       * do not override it */
      listen_address = NULL;
    }
  else if (g_getenv ("DBUS_TEST_DATADIR") != NULL)
    {
      arg = g_strdup_printf ("--config-file=%s/dbus-1/session.conf",
          g_getenv ("DBUS_TEST_DATADIR"));
    }
  else if (g_getenv ("DBUS_TEST_DATA") != NULL)
    {
      arg = g_strdup_printf (
          "--config-file=%s/valid-config-files/session.conf",
          g_getenv ("DBUS_TEST_DATA"));
    }
  else
    {
      arg = g_strdup ("--session");
    }

  dbus_daemon = g_strdup (g_getenv ("DBUS_TEST_DAEMON"));

  if (dbus_daemon == NULL)
    dbus_daemon = g_strdup ("dbus-daemon");

  if (g_getenv ("DBUS_TEST_DAEMON_ADDRESS") != NULL)
    {
      if (config_file != NULL || user != TEST_USER_ME)
        {
          g_test_skip ("cannot use DBUS_TEST_DAEMON_ADDRESS for "
              "unusally-configured dbus-daemon");
          address = NULL;
        }
      else
        {
          address = g_strdup (g_getenv ("DBUS_TEST_DAEMON_ADDRESS"));
        }
    }
  else
    {
      address = spawn_dbus_daemon (dbus_daemon, arg,
          listen_address, user, runtime_dir, daemon_pid);
    }

  g_free (dbus_daemon);
  g_free (arg);
  return address;
}

DBusConnection *
test_connect_to_bus (TestMainContext *ctx,
    const gchar *address)
{
  GError *error = NULL;
  DBusConnection *conn = test_try_connect_to_bus (ctx, address, &error);

  g_assert_no_error (error);
  g_assert (conn != NULL);
  return conn;
}

DBusConnection *
test_try_connect_to_bus (TestMainContext *ctx,
    const gchar *address,
    GError **gerror)
{
  DBusConnection *conn;
  DBusError error = DBUS_ERROR_INIT;

  conn = dbus_connection_open_private (address, &error);

  if (conn == NULL)
    goto fail;

  if (!dbus_bus_register (conn, &error))
    goto fail;

  g_assert (dbus_bus_get_unique_name (conn) != NULL);

  if (ctx != NULL && !test_connection_try_setup (ctx, conn))
    {
      _DBUS_SET_OOM (&error);
      goto fail;
    }

  return conn;

fail:
  if (gerror != NULL)
    *gerror = g_dbus_error_new_for_dbus_error (error.name, error.message);

  if (conn != NULL)
    {
      dbus_connection_close (conn);
      dbus_connection_unref (conn);
    }

  dbus_error_free (&error);
  return FALSE;
}

static gboolean
become_other_user (TestUser user,
                   GError **error)
{
  /* For now we only do tests like this on Linux, because I don't know how
   * safe this use of setresuid() is on other platforms */
#if defined(HAVE_GETRESUID) && defined(HAVE_SETRESUID) && defined(__linux__)
  uid_t ruid, euid, suid;
  const struct passwd *pwd;
  const char *username;

  g_return_val_if_fail (user != TEST_USER_ME, FALSE);

  switch (user)
    {
      case TEST_USER_ROOT:
        username = "root";
        break;

      case TEST_USER_MESSAGEBUS:
        username = DBUS_USER;
        break;

      case TEST_USER_OTHER:
        username = DBUS_TEST_USER;
        break;

      /* TEST_USER_ROOT_DROP_TO_MESSAGEBUS is only meaningful for
       * test_get_dbus_daemon(), not as a client */
      case TEST_USER_ROOT_DROP_TO_MESSAGEBUS:
        g_return_val_if_reached (FALSE);

      case TEST_USER_ME:
      default:
        g_return_val_if_reached (FALSE);
    }

  if (getresuid (&ruid, &euid, &suid) != 0)
    g_error ("getresuid: %s", g_strerror (errno));

  if (ruid != 0 || euid != 0 || suid != 0)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
          "not uid 0 (ruid=%ld euid=%ld suid=%ld)",
          (unsigned long) ruid, (unsigned long) euid, (unsigned long) suid);
      return FALSE;
    }

  pwd = getpwnam (username);

  if (pwd == NULL)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
          "getpwnam(\"%s\"): %s", username, g_strerror (errno));
      return FALSE;
    }

  /* Impersonate the desired user while we connect to the bus.
   * This should work, because we're root; so if it fails, we just crash. */
  if (setresuid (pwd->pw_uid, pwd->pw_uid, 0) != 0)
    g_error ("setresuid(%ld, (same), 0): %s",
        (unsigned long) pwd->pw_uid, g_strerror (errno));

  return TRUE;

#else
  g_return_val_if_fail (user != TEST_USER_ME, FALSE);

  switch (user)
    {
      case TEST_USER_ROOT:
      case TEST_USER_MESSAGEBUS:
      case TEST_USER_OTHER:
        g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
            "setresuid() not available, or unsure about "
            "credentials-passing semantics on this platform");
        return FALSE;

      /* TEST_USER_ROOT_DROP_TO_MESSAGEBUS is only meaningful for
       * test_get_dbus_daemon(), not as a client */
      case TEST_USER_ROOT_DROP_TO_MESSAGEBUS:
        g_return_val_if_reached (FALSE);

      case TEST_USER_ME:
      default:
        g_return_val_if_reached (FALSE);
    }

#endif
}

/* Undo the effect of a successful call to become_other_user() */
static void
back_to_root (void)
{
#if defined(HAVE_GETRESUID) && defined(HAVE_SETRESUID) && defined(__linux__)
  if (setresuid (0, 0, 0) != 0)
    g_error ("setresuid(0, 0, 0): %s", g_strerror (errno));
#else
  g_error ("become_other_user() cannot succeed on this platform");
#endif
}

/*
 * Raise G_IO_ERROR_NOT_SUPPORTED if the requested user is impossible.
 * Do not mark the test as skipped: we might have more to test anyway.
 */
DBusConnection *
test_try_connect_to_bus_as_user (TestMainContext *ctx,
    const char *address,
    TestUser user,
    GError **error)
{
  DBusConnection *conn;

  if (user != TEST_USER_ME && !become_other_user (user, error))
    return NULL;

  conn = test_try_connect_to_bus (ctx, address, error);

  if (user != TEST_USER_ME)
    back_to_root ();

  return conn;
}

/*
 * Raise G_IO_ERROR_NOT_SUPPORTED if the requested user is impossible.
 */
GDBusConnection *
test_try_connect_gdbus_as_user (const char *address,
                                TestUser user,
                                GError **error)
{
  GDBusConnection *conn;

  if (user != TEST_USER_ME && !become_other_user (user, error))
    return NULL;

  conn = g_dbus_connection_new_for_address_sync (address,
      (G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION |
       G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT),
      NULL, NULL, error);

  if (user != TEST_USER_ME)
    back_to_root ();

  return conn;
}

static void
pid_died (GPid pid,
          gint status,
          gpointer user_data)
{
  gboolean *result = user_data;

  g_assert (result != NULL);
  g_assert (!*result);
  *result = TRUE;
}

void
test_kill_pid (GPid pid)
{
  gint died = FALSE;

  g_child_watch_add (pid, pid_died, &died);

#ifdef DBUS_WIN
  if (pid != NULL)
    TerminateProcess (pid, 1);
#else
  if (pid > 0)
    kill (pid, SIGTERM);
#endif

  while (!died)
    g_main_context_iteration (NULL, TRUE);
}

static gboolean
time_out (gpointer data)
{
  puts ("Bail out! Test timed out (GLib main loop timeout callback reached)");
  fflush (stdout);
  abort ();
  return FALSE;
}

#ifdef G_OS_UNIX
static void wrap_abort (int signal) _DBUS_GNUC_NORETURN;

static void
wrap_abort (int signal)
{
  /* We might be halfway through writing out something else, so force this
   * onto its own line */
  const char message [] = "\nBail out! Test timed out (SIGALRM received)\n";

  if (write (STDOUT_FILENO, message, sizeof (message) - 1) <
      (ssize_t) sizeof (message) - 1)
    {
      /* ignore short write - what would we do about it? */
    }

  abort ();
}
#endif

static void
set_timeout (guint factor)
{
  static guint timeout = 0;
  const gchar *env_factor_str;
  guint64 env_factor = 1;

  /* Prevent tests from hanging forever. This is intended to be long enough
   * that any reasonable regression test on any reasonable hardware would
   * have finished. */
#define TIMEOUT 60

  if (timeout != 0)
    g_source_remove (timeout);

  if (RUNNING_ON_VALGRIND)
    factor = factor * 10;

  env_factor_str = g_getenv ("DBUS_TEST_TIMEOUT_MULTIPLIER");

  if (env_factor_str != NULL)
    {
      env_factor = g_ascii_strtoull (env_factor_str, NULL, 10);

      if (env_factor == 0)
        g_error ("Invalid DBUS_TEST_TIMEOUT_MULTIPLIER %s", env_factor_str);

      factor = factor * env_factor;
    }

  timeout = g_timeout_add_seconds (TIMEOUT * factor, time_out, NULL);
#ifdef G_OS_UNIX
  /* The GLib main loop might not be running (we don't use it in every
   * test). Die with SIGALRM shortly after if necessary. */
  alarm ((TIMEOUT * factor) + 10);

  /* Get a log message and a core dump from the SIGALRM. */
    {
      struct sigaction act = { };

      act.sa_handler = wrap_abort;

      sigaction (SIGALRM, &act, NULL);
    }
#endif
}

void
test_init (int *argcp, char ***argvp)
{
  /* If our argv only contained the executable name, assume we were
   * run by Automake with LOG_COMPILER overridden by
   * VALGRIND_CHECK_RULES from AX_VALGRIND_CHECK, and automatically switch
   * on TAP output. This avoids needing glib-tap-test.sh. We still use
   * glib-tap-test.sh in the common case because it replaces \r\n line
   * endings with \n, which we need if running the tests under Wine. */
  static char tap[] = "--tap";
  static char *substitute_argv[] = { NULL, tap, NULL };

  g_return_if_fail (argcp != NULL);
  g_return_if_fail (*argcp > 0);
  g_return_if_fail (argvp != NULL);
  g_return_if_fail (argvp[0] != NULL);
  g_return_if_fail (argvp[0][0] != NULL);

  if (*argcp == 1)
    {
      substitute_argv[0] = (*argvp)[0];
      *argcp = 2;
      *argvp = substitute_argv;
    }

  g_test_init (argcp, argvp, NULL);

  g_test_bug_base ("https://bugs.freedesktop.org/show_bug.cgi?id=");
  set_timeout (1);
}

static void
report_and_destroy (gpointer p)
{
  GTimer *timer = p;

  g_test_message ("Time since timeout reset %p: %.3f seconds",
      timer, g_timer_elapsed (timer, NULL));
  g_timer_destroy (timer);
}

void
test_timeout_reset (guint factor)
{
  GTimer *timer = g_timer_new ();

  g_test_message ("Resetting test timeout (reference: %p; factor: %u)",
      timer, factor);
  set_timeout (factor);

  g_test_queue_destroy (report_and_destroy, timer);
}

void
test_progress (char symbol)
{
  if (g_test_verbose () && isatty (1))
    g_print ("%c", symbol);
}

/*
 * Delete @path, with a retry loop if the system call is interrupted by
 * an async signal. If @path does not exist, ignore; otherwise, it is
 * required to be a non-directory.
 */
void
test_remove_if_exists (const gchar *path)
{
  while (g_remove (path) != 0)
    {
      int saved_errno = errno;

      if (saved_errno == ENOENT)
        return;

#ifdef G_OS_UNIX
      if (saved_errno == EINTR)
        continue;
#endif

      g_error ("Unable to remove file \"%s\": %s", path,
               g_strerror (saved_errno));
    }
}

/*
 * Delete empty directory @path, with a retry loop if the system call is
 * interrupted by an async signal. @path is required to exist.
 */
void
test_rmdir_must_exist (const gchar *path)
{
  while (g_remove (path) != 0)
    {
      int saved_errno = errno;

#ifdef G_OS_UNIX
      if (saved_errno == EINTR)
        continue;
#endif

      g_error ("Unable to remove directory \"%s\": %s", path,
               g_strerror (saved_errno));
    }
}

/*
 * Delete empty directory @path, with a retry loop if the system call is
 * interrupted by an async signal. If @path does not exist, ignore.
 */
void
test_rmdir_if_exists (const gchar *path)
{
  while (g_remove (path) != 0)
    {
      int saved_errno = errno;

      if (saved_errno == ENOENT)
        return;

#ifdef G_OS_UNIX
      if (saved_errno == EINTR)
        continue;
#endif

      g_error ("Unable to remove directory \"%s\": %s", path,
               g_strerror (saved_errno));
    }
}

/*
 * Create directory @path, with a retry loop if the system call is
 * interrupted by an async signal.
 */
void
test_mkdir (const gchar *path,
            gint mode)
{
  while (g_mkdir (path, mode) != 0)
    {
      int saved_errno = errno;

#ifdef G_OS_UNIX
      if (saved_errno == EINTR)
        continue;
#endif

      g_error ("Unable to create directory \"%s\": %s", path,
               g_strerror (saved_errno));
    }
}

void
test_oom (void)
{
  g_error ("Out of memory");
}

/*
 * Send the given method call and wait for a reply, spinning the main
 * context as necessary.
 */
DBusMessage *
test_main_context_call_and_wait (TestMainContext *ctx,
    DBusConnection *connection,
    DBusMessage *call,
    int timeout)
{
  DBusPendingCall *pc = NULL;
  DBusMessage *reply = NULL;

  if (!dbus_connection_send_with_reply (connection, call, &pc, timeout) ||
      pc == NULL)
    test_oom ();

  if (dbus_pending_call_get_completed (pc))
    test_pending_call_store_reply (pc, &reply);
  else if (!dbus_pending_call_set_notify (pc, test_pending_call_store_reply,
        &reply, NULL))
    test_oom ();

  while (reply == NULL)
    test_main_context_iterate (ctx, TRUE);

  dbus_clear_pending_call (&pc);
  return g_steal_pointer (&reply);
}

gboolean
test_check_tcp_works (void)
{
#ifdef DBUS_UNIX
  /* In pathological container environments, we might not have a
   * working 127.0.0.1 */
  int res;
  struct addrinfo *addrs = NULL;
  struct addrinfo hints;
  int saved_errno;

  _DBUS_ZERO (hints);
#ifdef AI_ADDRCONFIG
  hints.ai_flags |= AI_ADDRCONFIG;
#endif
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  res = getaddrinfo ("127.0.0.1", "0", &hints, &addrs);
  saved_errno = errno;

  if (res != 0)
    {
      const gchar *system_message;
      gchar *skip_message;

#ifdef EAI_SYSTEM
      if (res == EAI_SYSTEM)
        system_message = g_strerror (saved_errno);
      else
#endif
        system_message = gai_strerror (res);

      skip_message = g_strdup_printf ("Name resolution does not work here: "
                                      "getaddrinfo(\"127.0.0.1\", \"0\", "
                                      "{flags=ADDRCONFIG, family=INET,"
                                      "socktype=STREAM, protocol=TCP}): "
                                      "%s",
                                      system_message);
      g_test_skip (skip_message);
      free (skip_message);
    }

  if (addrs != NULL)
    freeaddrinfo (addrs);

  return (res == 0);
#else
  /* Assume that on Windows, TCP always works */
  return TRUE;
#endif
}

/*
 * Store the result of an async operation. @user_data is a pointer to a
 * variable that can store @result, initialized to %NULL.
 */
void
test_store_result_cb (GObject *source_object G_GNUC_UNUSED,
                      GAsyncResult *result,
                      gpointer user_data)
{
  GAsyncResult **result_p = user_data;

  g_assert_nonnull (result_p);
  g_assert_null (*result_p);
  *result_p = g_object_ref (result);
}

/*
 * Report that a test should have failed, but we are tolerating the
 * failure because it represents a known bug or missing feature.
 *
 * This is the same as g_test_incomplete(), but with a workaround for
 * GLib bug 1474 so that we don't fail tests on older GLib.
 */
void
test_incomplete (const gchar *message)
{
  if (glib_check_version (2, 57, 3))
    {
      /* In GLib >= 2.57.3, g_test_incomplete() behaves as intended:
       * the test result is reported as an expected failure and the
       * overall test exits 0 */
      g_test_incomplete (message);
    }
  else
    {
      /* In GLib < 2.57.3, g_test_incomplete() reported the wrong TAP
       * result (an unexpected success) and the overall test exited 1,
       * which would break "make check". g_test_skip() is the next
       * best thing available. */
      g_test_skip (message);
    }
}

/*
 * Return location of @exe test helper executable, or NULL if unknown.
 *
 * @exe must already include %DBUS_EXEEXT if appropriate.
 *
 * Returns: (transfer full) (nullable): an absolute path or NULL.
 */
gchar *
test_get_helper_executable (const gchar *exe)
{
  const char *dbus_test_exec;

  dbus_test_exec = _dbus_getenv ("DBUS_TEST_EXEC");

  if (dbus_test_exec == NULL)
    return NULL;

  return g_build_filename (dbus_test_exec, exe, NULL);
}
