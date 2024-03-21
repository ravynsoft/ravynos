/*
 * Copyright 2002-2009 Red Hat Inc.
 * Copyright 2011-2017 Collabora Ltd.
 * Copyright 2017 Endless Mobile, Inc.
 *
 * SPDX-License-Identifier: MIT
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
#include "test-utils.h"

#include <stdlib.h>
#include <string.h>

#if HAVE_LOCALE_H
#include <locale.h>
#endif

#include <dbus/dbus-sysdeps.h>

#ifdef DBUS_UNIX
# include <sys/types.h>
# include <unistd.h>

# include <dbus/dbus-sysdeps-unix.h>
#else
# include <dbus/dbus-sysdeps-win.h>
#endif

#ifdef __linux__
/* Necessary for the Linux-specific fd leak checking code only */
#include <dirent.h>
#include <errno.h>
#endif

#include "dbus/dbus-message-internal.h"

/*
 * Like strdup(), but crash on out-of-memory, and pass through NULL
 * unchanged (the "0" in the name is meant to be a mnemonic for this,
 * similar to g_strcmp0()).
 */
static char *
strdup0_or_die (const char *str)
{
  char *ret;

  if (str == NULL)
    return NULL;  /* not an error */

  ret = strdup (str);

  if (ret == NULL)
    _dbus_test_fatal ("Out of memory");

  return ret;
}

typedef struct
{
  DBusLoop *loop;
  DBusConnection *connection;

} CData;

static dbus_bool_t
add_watch (DBusWatch *watch,
	   void      *data)
{
  CData *cd = data;

  return _dbus_loop_add_watch (cd->loop, watch);
}

static void
remove_watch (DBusWatch *watch,
	      void      *data)
{
  CData *cd = data;
  
  _dbus_loop_remove_watch (cd->loop, watch);
}

static void
toggle_watch (DBusWatch  *watch,
              void       *data)
{
  CData *cd = data;

  _dbus_loop_toggle_watch (cd->loop, watch);
}

static dbus_bool_t
add_timeout (DBusTimeout *timeout,
	     void        *data)
{
  CData *cd = data;

  return _dbus_loop_add_timeout (cd->loop, timeout);
}

static void
remove_timeout (DBusTimeout *timeout,
		void        *data)
{
  CData *cd = data;

  _dbus_loop_remove_timeout (cd->loop, timeout);
}

static void
dispatch_status_function (DBusConnection    *connection,
                          DBusDispatchStatus new_status,
                          void              *data)
{
  DBusLoop *loop = data;
  
  if (new_status != DBUS_DISPATCH_COMPLETE)
    {
      while (!_dbus_loop_queue_dispatch (loop, connection))
        _dbus_wait_for_memory ();
    }
}

static void
cdata_free (void *data)
{
  CData *cd = data;

  dbus_connection_unref (cd->connection);
  _dbus_loop_unref (cd->loop);
  
  dbus_free (cd);
}

static CData*
cdata_new (DBusLoop       *loop,
           DBusConnection *connection)
{
  CData *cd;

  cd = dbus_new0 (CData, 1);
  if (cd == NULL)
    return NULL;

  cd->loop = loop;
  cd->connection = connection;

  dbus_connection_ref (cd->connection);
  _dbus_loop_ref (cd->loop);

  return cd;
}

dbus_bool_t
test_connection_try_setup (TestMainContext *ctx,
                           DBusConnection  *connection)
{
  DBusLoop *loop = ctx;
  CData *cd;

  cd = NULL;
  
  dbus_connection_set_dispatch_status_function (connection, dispatch_status_function,
                                                loop, NULL);
  /* ownership of cd taken */

  cd = cdata_new (loop, connection);
  if (cd == NULL)
    goto nomem;

  if (!dbus_connection_set_watch_functions (connection,
                                            add_watch,
                                            remove_watch,
                                            toggle_watch,
                                            cd, cdata_free))
    goto nomem;


  cd = cdata_new (loop, connection);
  if (cd == NULL)
    goto nomem;
  
  if (!dbus_connection_set_timeout_functions (connection,
                                              add_timeout,
                                              remove_timeout,
                                              NULL,
                                              cd, cdata_free))
    goto nomem;

  /* ownership taken */
  cd = NULL;

  if (dbus_connection_get_dispatch_status (connection) != DBUS_DISPATCH_COMPLETE)
    {
      if (!_dbus_loop_queue_dispatch (loop, connection))
        goto nomem;
    }
  
  return TRUE;
  
 nomem:
  if (cd)
    cdata_free (cd);
  
  dbus_connection_set_dispatch_status_function (connection, NULL, NULL, NULL);
  dbus_connection_set_watch_functions (connection, NULL, NULL, NULL, NULL, NULL);
  dbus_connection_set_timeout_functions (connection, NULL, NULL, NULL, NULL, NULL);
  
  return FALSE;
}

static void die (const char *message) _DBUS_GNUC_NORETURN;

static void
die (const char *message)
{
  printf ("Bail out! %s\n", message);
  fflush (stdout);
  exit (1);
}

void
test_connection_setup (TestMainContext *ctx,
                       DBusConnection  *connection)
{
  if (!test_connection_try_setup (ctx, connection))
    die ("Not enough memory to set up connection");
}

void
test_connection_shutdown (TestMainContext *ctx,
                          DBusConnection *connection)
{
  if (!dbus_connection_set_watch_functions (connection,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL, NULL))
    die ("setting watch functions to NULL failed");
  
  if (!dbus_connection_set_timeout_functions (connection,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL, NULL))
    die ("setting timeout functions to NULL failed");

  dbus_connection_set_dispatch_status_function (connection, NULL, NULL, NULL);
}

typedef struct
{
  DBusLoop *loop;
  DBusServer *server;
} ServerData;

static void
serverdata_free (void *data)
{
  ServerData *sd = data;

  dbus_server_unref (sd->server);
  _dbus_loop_unref (sd->loop);
  
  dbus_free (sd);
}

static ServerData*
serverdata_new (DBusLoop       *loop,
                DBusServer     *server)
{
  ServerData *sd;

  sd = dbus_new0 (ServerData, 1);
  if (sd == NULL)
    return NULL;

  sd->loop = loop;
  sd->server = server;

  dbus_server_ref (sd->server);
  _dbus_loop_ref (sd->loop);

  return sd;
}

static dbus_bool_t
add_server_watch (DBusWatch  *watch,
                  void       *data)
{
  ServerData *context = data;

  return _dbus_loop_add_watch (context->loop, watch);
}

static void
toggle_server_watch (DBusWatch  *watch,
                     void       *data)
{
  ServerData *context = data;

  _dbus_loop_toggle_watch (context->loop, watch);
}

static void
remove_server_watch (DBusWatch  *watch,
                     void       *data)
{
  ServerData *context = data;
  
  _dbus_loop_remove_watch (context->loop, watch);
}

static dbus_bool_t
add_server_timeout (DBusTimeout *timeout,
                    void        *data)
{
  ServerData *context = data;

  return _dbus_loop_add_timeout (context->loop, timeout);
}

static void
remove_server_timeout (DBusTimeout *timeout,
                       void        *data)
{
  ServerData *context = data;
  
  _dbus_loop_remove_timeout (context->loop, timeout);
}

dbus_bool_t
test_server_try_setup (TestMainContext *ctx,
                       DBusServer      *server)
{
  DBusLoop *loop = ctx;
  ServerData *sd;

  sd = serverdata_new (loop, server);
  if (sd == NULL)
    goto nomem;

  if (!dbus_server_set_watch_functions (server,
                                        add_server_watch,
                                        remove_server_watch,
                                        toggle_server_watch,
                                        sd,
                                        serverdata_free))
    {
      goto nomem;
    }

  sd = serverdata_new (loop, server);
  if (sd == NULL)
    goto nomem;

  if (!dbus_server_set_timeout_functions (server,
                                          add_server_timeout,
                                          remove_server_timeout,
                                          NULL,
                                          sd, serverdata_free))
    {
      goto nomem;
    }   
  return TRUE;

 nomem:
  if (sd)
    serverdata_free (sd);
  
  test_server_shutdown (loop, server);
  
  return FALSE;
}

void
test_server_setup (TestMainContext *ctx,
                   DBusServer      *server)
{
  if (!test_server_try_setup (ctx, server))
    die ("Not enough memory to set up server");
}

void
test_server_shutdown (TestMainContext  *ctx,
                      DBusServer       *server)
{
  dbus_server_disconnect (server);

  if (!dbus_server_set_watch_functions (server,
                                        NULL, NULL, NULL,
                                        NULL,
                                        NULL))
    die ("setting watch functions to NULL failed");
  
  if (!dbus_server_set_timeout_functions (server,
                                          NULL, NULL, NULL,
                                          NULL,
                                          NULL))
    die ("setting timeout functions to NULL failed");
}

TestMainContext *
test_main_context_get (void)
{
  TestMainContext *ret = _dbus_loop_new ();

  if (ret == NULL)
    die ("Out of memory");

  return ret;
}

TestMainContext *
test_main_context_try_get (void)
{
  return _dbus_loop_new ();
}

TestMainContext *
test_main_context_ref (TestMainContext *ctx)
{
  return _dbus_loop_ref (ctx);
}

void test_main_context_unref (TestMainContext *ctx)
{
  _dbus_loop_unref (ctx);
}

void test_main_context_iterate (TestMainContext *ctx,
                                dbus_bool_t      may_block)
{
  _dbus_loop_iterate (ctx, may_block);
}

void
test_pending_call_store_reply (DBusPendingCall *pc,
    void *data)
{
  DBusMessage **message_p = data;

  *message_p = dbus_pending_call_steal_reply (pc);
  _dbus_assert (*message_p != NULL);
}

#ifdef DBUS_UNIX

/*
 * Set uid to a machine-readable authentication identity (numeric Unix
 * uid or ConvertSidToStringSid-style Windows SID) that is likely to exist,
 * and differs from the identity of the current process.
 *
 * @param uid Populated with a machine-readable authentication identity
 *    on success
 * @returns #FALSE if no memory
 */
dbus_bool_t
_dbus_test_append_different_uid (DBusString *uid)
{
  if (geteuid () == 0)
    return _dbus_string_append (uid, "65534");
  else
    return _dbus_string_append (uid, "0");
}

/*
 * Set uid to a human-readable authentication identity (login name)
 * that is likely to exist, and differs from the identity of the current
 * process. This function currently only exists on Unix platforms.
 *
 * @param uid Populated with a machine-readable authentication identity
 *    on success
 * @returns #FALSE if no memory
 */
dbus_bool_t
_dbus_test_append_different_username (DBusString *username)
{
  if (geteuid () == 0)
    return _dbus_string_append (username, "nobody");
  else
    return _dbus_string_append (username, "root");
}

#else /* !defined(DBUS_UNIX) */

#define ANONYMOUS_SID "S-1-5-7"
#define LOCAL_SYSTEM_SID "S-1-5-18"

dbus_bool_t
_dbus_test_append_different_uid (DBusString *uid)
{
  char *sid = NULL;
  dbus_bool_t ret;

  if (!_dbus_getsid (&sid, _dbus_getpid ()))
    return FALSE;

  if (strcmp (sid, ANONYMOUS_SID) == 0)
    ret = _dbus_string_append (uid, LOCAL_SYSTEM_SID);
  else
    ret = _dbus_string_append (uid, ANONYMOUS_SID);

  LocalFree (sid);
  return ret;
}

#endif /* !defined(DBUS_UNIX) */

#ifdef __linux__
struct DBusInitialFDs {
    fd_set set;
};
#endif

DBusInitialFDs *
_dbus_check_fdleaks_enter (void)
{
#ifdef __linux__
  DIR *d;
  DBusInitialFDs *fds;

  /* this is plain malloc so it won't interfere with leak checking */
  fds = malloc (sizeof (DBusInitialFDs));
  _dbus_assert (fds != NULL);

  /* This works on Linux only */

  if ((d = opendir ("/proc/self/fd")))
    {
      struct dirent *de;

      while ((de = readdir(d)))
        {
          long l;
          char *e = NULL;
          int fd;

          if (de->d_name[0] == '.')
            continue;

          errno = 0;
          l = strtol (de->d_name, &e, 10);
          _dbus_assert (errno == 0 && e && !*e);

          fd = (int) l;

          if (fd < 3)
            continue;

          if (fd == dirfd (d))
            continue;

          if (fd >= FD_SETSIZE)
            {
              _dbus_verbose ("FD %d unexpectedly large; cannot track whether "
                             "it is leaked\n", fd);
              continue;
            }

          FD_SET (fd, &fds->set);
        }

      closedir (d);
    }

  return fds;
#else
  return NULL;
#endif
}

void
_dbus_check_fdleaks_leave (DBusInitialFDs *fds,
                           const char     *context)
{
#ifdef __linux__
  DIR *d;

  /* This works on Linux only */

  if ((d = opendir ("/proc/self/fd")))
    {
      struct dirent *de;

      while ((de = readdir(d)))
        {
          long l;
          char *e = NULL;
          int fd;

          if (de->d_name[0] == '.')
            continue;

          errno = 0;
          l = strtol (de->d_name, &e, 10);
          _dbus_assert (errno == 0 && e && !*e);

          fd = (int) l;

          if (fd < 3)
            continue;

          if (fd == dirfd (d))
            continue;

          if (fd >= FD_SETSIZE)
            {
              _dbus_verbose ("FD %d unexpectedly large; cannot track whether "
                             "it is leaked\n", fd);
              continue;
            }

          if (FD_ISSET (fd, &fds->set))
            continue;

          _dbus_test_fatal ("file descriptor %i leaked in %s.", fd, context);
        }

      closedir (d);
    }

  free (fds);
#else
  _dbus_assert (fds == NULL);
#endif
}

static void
_dbus_test_help_page (const char *appname)
{
  fprintf(stdout, "%s [<options>] [<test-data-dir>] [<specific-test>]\n", appname);
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "    --help         this page\n");
  fprintf(stdout, "    --list-tests   show available tests\n");
  fprintf(stdout, "    --tap          expect test data dir to be set by environment variable DBUS_TEST_DATA\n");
  fprintf(stdout, "Environment variables:\n");
  fprintf(stdout, "    DBUS_TEST_ONLY=<specific-test>    set specific test to run\n");
  fprintf(stdout, "    DBUS_TEST_DATA=<test-data-dir>    set test data dir (required when using --tap)\n");
}

static void
_dbus_test_show_available_tests (size_t n_tests,
                                 const DBusTestCase *tests)
{
  size_t i;

  for (i = 0; i < n_tests; i++)
    {
      if (tests[i].name == NULL)
        break;

      fprintf(stdout, "%s\n", tests[i].name);
    }
}

static const DBusTestCase *
_dbus_test_find_test (size_t n_tests,
                      const DBusTestCase *tests,
                      const char *specific_test)
{
  size_t i;

  for (i = 0; i < n_tests; i++)
    {
      if (tests[i].name == NULL)
        break;

      if (strcmp (specific_test, tests[i].name) == 0)
        return &tests[i];
    }
  return NULL;
}


/*
 * _dbus_test_main:
 * @argc: number of command-line arguments
 * @argv: array of @argc arguments
 * @n_tests: length of @tests
 * @tests: array of @n_tests tests
 * @flags: flags affecting all tests
 * @test_pre_hook: if not %NULL, called before each test
 * @test_post_hook: if not %NULL, called after each test
 *
 * Wrapper for dbus tests that do not use GLib. Processing of @tests
 * can be terminated early by an entry with @name = NULL, which is a
 * convenient way to put a trailing comma on every "real" test entry
 * without breaking compilation on pedantic C compilers.
 */
int
_dbus_test_main (int                  argc,
                 char               **argv,
                 size_t               n_tests,
                 const DBusTestCase  *tests,
                 DBusTestFlags        flags,
                 void               (*test_pre_hook) (void),
                 void               (*test_post_hook) (void))
{
  char *test_data_dir;
  char *specific_test;
  size_t i;

#ifdef DBUS_UNIX
  /* close any inherited fds so dbus-spawn's check for close-on-exec works */
  _dbus_close_all ();
#endif

#if HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif

  if (argc > 1 && strcmp (argv[1], "--help") == 0)
    {
      _dbus_test_help_page (argv[0]);
      exit(0);
    }

  else if (argc > 1 && strcmp (argv[1], "--list-tests") == 0)
    {
      _dbus_test_show_available_tests (n_tests, tests);
      exit (0);
    }

  /* We can't assume that strings from _dbus_getenv() will remain valid
   * forever, because some tests call setenv(), which is allowed to
   * reallocate the entire environment block, and in Wine it seems that it
   * genuinely does; so we copy them.
   *
   * We can't use _dbus_strdup() here because the test might be checking
   * for memory leaks, so we don't want any libdbus allocations still
   * alive at the end; so we use strdup(), which is not in Standard C but
   * is available in both POSIX and Windows. */
  if (argc > 1 && strcmp (argv[1], "--tap") != 0)
    test_data_dir = strdup0_or_die (argv[1]);
  else
    test_data_dir = strdup0_or_die (_dbus_getenv ("DBUS_TEST_DATA"));

  if (test_data_dir != NULL)
    _dbus_test_diag ("Test data in %s", test_data_dir);
  else if (flags & DBUS_TEST_FLAGS_REQUIRE_DATA)
    {
      _dbus_test_help_page (argv[0]);
      _dbus_test_fatal ("Must specify test data directory as argv[1] or "
                        "in DBUS_TEST_DATA environment variable");
    }
  else
    _dbus_test_diag ("No test data!");

  if (argc > 2)
    specific_test = strdup0_or_die (argv[2]);
  else
    specific_test = strdup0_or_die (_dbus_getenv ("DBUS_TEST_ONLY"));

  /* check that test is present */
  if (specific_test)
    {
      if (_dbus_test_find_test (n_tests, tests, specific_test) == NULL)
        {
          _dbus_test_fatal ("Invalid test name '%s' specified", specific_test);
        }
    }

  /* Some NSS modules like those for sssd and LDAP might allocate fds
   * on a one-per-process basis. Make sure those have already been
   * allocated before we enter the code under test, so that they don't
   * show up as having been "leaked" by the first module of code under
   * test that happens to do a NSS lookup. */
    {
      DBusString username;
      dbus_uid_t ignored_uid = DBUS_UID_UNSET;

      _dbus_string_init_const (&username, "dbus-no-user-with-this-name");
      /* We use a username that almost certainly doesn't exist, because
       * if we used something like root it might get handled early in the
       * NSS search order, before we get as far as asking sssd or LDAP. */
      _dbus_parse_unix_user_from_config (&username, &ignored_uid);
      _dbus_test_check_memleaks ("initial nss query");
    }

  for (i = 0; i < n_tests; i++)
    {
      long before, after;
      DBusInitialFDs *initial_fds = NULL;

      if (tests[i].name == NULL)
        break;

      if (n_tests > 1 &&
          specific_test != NULL &&
          strcmp (specific_test, tests[i].name) != 0)
        {
          _dbus_test_skip ("%s - Only intending to run %s",
                           tests[i].name, specific_test);
          continue;
        }

      _dbus_test_diag ("Running test: %s", tests[i].name);
      _dbus_get_monotonic_time (&before, NULL);

      if (test_pre_hook)
        test_pre_hook ();

      if (flags & DBUS_TEST_FLAGS_CHECK_FD_LEAKS)
        initial_fds = _dbus_check_fdleaks_enter ();

      if (tests[i].func (test_data_dir))
        _dbus_test_ok ("%s", tests[i].name);
      else
        _dbus_test_not_ok ("%s", tests[i].name);

      _dbus_get_monotonic_time (&after, NULL);

      _dbus_test_diag ("%s test took %ld seconds",
                       tests[i].name, after - before);

      if (test_post_hook)
        test_post_hook ();

      if (flags & DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS)
        _dbus_test_check_memleaks (tests[i].name);

      if (flags & DBUS_TEST_FLAGS_CHECK_FD_LEAKS)
        _dbus_check_fdleaks_leave (initial_fds, tests[i].name);
    }

  free (test_data_dir);
  free (specific_test);

  return _dbus_test_done_testing ();
}

/* If embedded tests are enabled, the TAP helpers have to be in the
 * shared library because some of the embedded tests call them. If not,
 * implement them here. We #include the file here instead of adding it
 * to SOURCES because Automake versions older than 1.16 can't cope with
 * expanding directory variables in SOURCES when using subdir-objects. */
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus/dbus-test-tap.c"
#endif
