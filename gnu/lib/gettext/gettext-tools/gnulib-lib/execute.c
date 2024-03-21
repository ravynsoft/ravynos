/* Creation of autonomous subprocesses.
   Copyright (C) 2001-2004, 2006-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */


#include <config.h>

/* Specification.  */
#include "execute.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "canonicalize.h"
#include "error.h"
#include "fatal-signal.h"
#include "filename.h"
#include "findprog.h"
#include "wait-process.h"
#include "xalloc.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Choice of implementation for native Windows.
   - Define to 0 to use the posix_spawn facility (modules 'posix_spawn' and
     'posix_spawnp'), that is based on the module 'windows-spawn'.
   - Define to 1 to use the older code, that uses the module 'windows-spawn'
     directly.
   You can set this macro from a Makefile or at configure time, from the
   CPPFLAGS.  */
#ifndef EXECUTE_IMPL_AVOID_POSIX_SPAWN
# define EXECUTE_IMPL_AVOID_POSIX_SPAWN 0
#endif


#if (defined _WIN32 && !defined __CYGWIN__) && EXECUTE_IMPL_AVOID_POSIX_SPAWN

/* Native Windows API.  */
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif
# include <process.h>
# include "windows-spawn.h"

#else

/* Unix API.  */
# include <spawn.h>

#endif


#if defined EINTR && (defined _WIN32 && !defined __CYGWIN__) && EXECUTE_IMPL_AVOID_POSIX_SPAWN

/* EINTR handling for close(), open().
   These functions can return -1/EINTR even though we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

static int
nonintr_close (int fd)
{
  int retval;

  do
    retval = close (fd);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#undef close /* avoid warning related to gnulib module unistd */
#define close nonintr_close

static int
nonintr_open (const char *pathname, int oflag, mode_t mode)
{
  int retval;

  do
    retval = open (pathname, oflag, mode);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#undef open /* avoid warning on VMS */
#define open nonintr_open

#endif


int
execute (const char *progname,
         const char *prog_path, const char * const *prog_argv,
         const char *directory,
         bool ignore_sigpipe,
         bool null_stdin, bool null_stdout, bool null_stderr,
         bool slave_process, bool exit_on_error,
         int *termsigp)
{
  int saved_errno;
  char *prog_path_to_free = NULL;

  if (directory != NULL)
    {
      /* If a change of directory is requested, make sure PROG_PATH is absolute
         before we do so.  This is needed because
           - posix_spawn and posix_spawnp are required to resolve a relative
             PROG_PATH *after* changing the directory.  See
             <https://www.austingroupbugs.net/view.php?id=1208>:
               "if this pathname does not start with a <slash> it shall be
                interpreted relative to the working directory of the child
                process _after_ all file_actions have been performed."
             But this would be a surprising application behaviour, possibly
             even security relevant.
           - For the Windows CreateProcess() function, it is unspecified whether
             a relative file name is interpreted to the parent's current
             directory or to the specified directory.  See
             <https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa>  */
      if (! IS_ABSOLUTE_FILE_NAME (prog_path))
        {
          const char *resolved_prog =
            find_in_given_path (prog_path, getenv ("PATH"), NULL, false);
          if (resolved_prog == NULL)
            goto fail_with_errno;
          if (resolved_prog != prog_path)
            prog_path_to_free = (char *) resolved_prog;
          prog_path = resolved_prog;

          if (! IS_ABSOLUTE_FILE_NAME (prog_path))
            {
              char *absolute_prog =
                canonicalize_filename_mode (prog_path,
                                            CAN_MISSING | CAN_NOLINKS);
              if (absolute_prog == NULL)
                {
                  free (prog_path_to_free);
                  goto fail_with_errno;
                }
              free (prog_path_to_free);
              prog_path_to_free = absolute_prog;
              prog_path = absolute_prog;

              if (! IS_ABSOLUTE_FILE_NAME (prog_path))
                abort ();
            }
        }
    }

#if (defined _WIN32 && !defined __CYGWIN__) && EXECUTE_IMPL_AVOID_POSIX_SPAWN

  /* Native Windows API.  */

  char *argv_mem_to_free;

  const char **argv = prepare_spawn (prog_argv, &argv_mem_to_free);
  if (argv == NULL)
    xalloc_die ();

  int exitcode = -1;

  /* Create standard file handles of child process.  */
  int nullinfd = -1;
  int nulloutfd = -1;
  if ((!null_stdin
       || (nullinfd = open ("NUL", O_RDONLY, 0)) >= 0)
      && (!(null_stdout || null_stderr)
          || (nulloutfd = open ("NUL", O_RDWR, 0)) >= 0))
    /* Pass the environment explicitly.  This is needed if the program has
       modified the environment using putenv() or [un]setenv().  On Windows,
       processes have two environments, one in the "environment block" of the
       process and managed through SetEnvironmentVariable(), and one inside the
       process, in the location retrieved by the 'environ' macro.  If we were
       to pass NULL, the child process would inherit a copy of the environment
       block - ignoring the effects of putenv() and [un]setenv().  */
    {
      HANDLE stdin_handle =
        (HANDLE) _get_osfhandle (null_stdin ? nullinfd : STDIN_FILENO);
      HANDLE stdout_handle =
        (HANDLE) _get_osfhandle (null_stdout ? nulloutfd : STDOUT_FILENO);
      HANDLE stderr_handle =
        (HANDLE) _get_osfhandle (null_stderr ? nulloutfd : STDERR_FILENO);

      exitcode = spawnpvech (P_WAIT, prog_path, argv + 1,
                             (const char * const *) environ, directory,
                             stdin_handle, stdout_handle, stderr_handle);
# if 0 /* Executing arbitrary files as shell scripts is unsecure.  */
      if (exitcode == -1 && errno == ENOEXEC)
        {
          /* prog is not a native executable.  Try to execute it as a
             shell script.  Note that prepare_spawn() has already prepended
             a hidden element "sh.exe" to argv.  */
          argv[1] = prog_path;
          exitcode = spawnpvech (P_WAIT, argv[0], argv,
                                 (const char * const *) environ, directory,
                                 stdin_handle, stdout_handle, stderr_handle);
        }
# endif
    }
  if (exitcode == -1)
    saved_errno = errno;
  if (nulloutfd >= 0)
    close (nulloutfd);
  if (nullinfd >= 0)
    close (nullinfd);
  free (argv);
  free (argv_mem_to_free);
  free (prog_path_to_free);

  /* Treat failure and signalled child processes like wait_subprocess()
     does.  */
  if (termsigp != NULL)
    *termsigp = 0;

  if (exitcode == -1)
    goto fail_with_saved_errno;

  if (WIFSIGNALED (exitcode))
    {
      if (termsigp != NULL)
        *termsigp = WTERMSIG (exitcode);
      saved_errno = 0;
      goto fail_with_saved_errno;
    }

  return exitcode;

#else

  /* Unix API.  */
  /* Note about 127: Some errors during posix_spawnp() cause the function
     posix_spawnp() to return an error code; some other errors cause the
     subprocess to exit with return code 127.  It is implementation
     dependent which error is reported which way.  We treat both cases as
     equivalent.  */
  sigset_t blocked_signals;
  posix_spawn_file_actions_t actions;
  bool actions_allocated;
  posix_spawnattr_t attrs;
  bool attrs_allocated;
  int err;
  pid_t child;

  if (slave_process)
    {
      sigprocmask (SIG_SETMASK, NULL, &blocked_signals);
      block_fatal_signals ();
    }
  actions_allocated = false;
  attrs_allocated = false;
  if ((err = posix_spawn_file_actions_init (&actions)) != 0
      || (actions_allocated = true,
          (null_stdin
            && (err = posix_spawn_file_actions_addopen (&actions,
                                                        STDIN_FILENO,
                                                        "/dev/null", O_RDONLY,
                                                        0))
               != 0)
          || (null_stdout
              && (err = posix_spawn_file_actions_addopen (&actions,
                                                          STDOUT_FILENO,
                                                          "/dev/null", O_RDWR,
                                                          0))
                 != 0)
          || (null_stderr
              && (err = posix_spawn_file_actions_addopen (&actions,
                                                          STDERR_FILENO,
                                                          "/dev/null", O_RDWR,
                                                          0))
                 != 0)
          || (directory != NULL
              && (err = posix_spawn_file_actions_addchdir (&actions,
                                                           directory)))
# if !(defined _WIN32 && !defined __CYGWIN__)
          || (slave_process
              && ((err = posix_spawnattr_init (&attrs)) != 0
                  || (attrs_allocated = true,
                      (err = posix_spawnattr_setsigmask (&attrs,
                                                         &blocked_signals))
                      != 0
                      || (err = posix_spawnattr_setflags (&attrs,
                                                        POSIX_SPAWN_SETSIGMASK))
                         != 0)))
# endif
          || (err = (directory != NULL
                     ? posix_spawn (&child, prog_path, &actions,
                                    attrs_allocated ? &attrs : NULL,
                                    (char * const *) prog_argv, environ)
                     : posix_spawnp (&child, prog_path, &actions,
                                     attrs_allocated ? &attrs : NULL,
                                     (char * const *) prog_argv, environ)))
             != 0))
    {
      if (actions_allocated)
        posix_spawn_file_actions_destroy (&actions);
      if (attrs_allocated)
        posix_spawnattr_destroy (&attrs);
      if (slave_process)
        unblock_fatal_signals ();
      free (prog_path_to_free);
      if (termsigp != NULL)
        *termsigp = 0;
      saved_errno = err;
      goto fail_with_saved_errno;
    }
  posix_spawn_file_actions_destroy (&actions);
  if (attrs_allocated)
    posix_spawnattr_destroy (&attrs);
  if (slave_process)
    {
      register_slave_subprocess (child);
      unblock_fatal_signals ();
    }
  free (prog_path_to_free);

  return wait_subprocess (child, progname, ignore_sigpipe, null_stderr,
                          slave_process, exit_on_error, termsigp);

#endif

 fail_with_errno:
  saved_errno = errno;
 fail_with_saved_errno:
  if (exit_on_error || !null_stderr)
    error (exit_on_error ? EXIT_FAILURE : 0, saved_errno,
           _("%s subprocess failed"), progname);
  return 127;
}
