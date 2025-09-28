/* Creation of subprocesses, communicating via pipes.
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


/* Tell clang not to warn about the 'child' variable, below.  */
#if defined __clang__
# pragma clang diagnostic ignored "-Wconditional-uninitialized"
#endif

#include <config.h>

/* Specification.  */
#include "spawn-pipe.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "canonicalize.h"
#include "error.h"
#include "fatal-signal.h"
#include "filename.h"
#include "findprog.h"
#include "unistd-safer.h"
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
#ifndef SPAWN_PIPE_IMPL_AVOID_POSIX_SPAWN
# define SPAWN_PIPE_IMPL_AVOID_POSIX_SPAWN 0
#endif


#if (defined _WIN32 && !defined __CYGWIN__) && SPAWN_PIPE_IMPL_AVOID_POSIX_SPAWN

/* Native Windows API.  */
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif
# include <process.h>
# include "windows-spawn.h"

#elif defined __KLIBC__

/* OS/2 kLIBC API.  */
# include <process.h>
# include "os2-spawn.h"

#else

/* Unix API.  */
# include <spawn.h>

#endif


#ifdef EINTR

/* EINTR handling for close().
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

#if (defined _WIN32 && !defined __CYGWIN__) && SPAWN_PIPE_IMPL_AVOID_POSIX_SPAWN
static int
nonintr_open (const char *pathname, int oflag, mode_t mode)
{
  int retval;

  do
    retval = open (pathname, oflag, mode);
  while (retval < 0 && errno == EINTR);

  return retval;
}
# undef open /* avoid warning on VMS */
# define open nonintr_open
#endif

#endif


/* Open a pipe connected to a child process.
 *
 *           write       system                read
 *    parent  ->   fd[1]   ->   STDIN_FILENO    ->   child       if pipe_stdin
 *    parent  <-   fd[0]   <-   STDOUT_FILENO   <-   child       if pipe_stdout
 *           read        system                write
 *
 * At least one of pipe_stdin, pipe_stdout must be true.
 * pipe_stdin and prog_stdin together determine the child's standard input.
 * pipe_stdout and prog_stdout together determine the child's standard output.
 * If pipe_stdin is true, prog_stdin is ignored.
 * If pipe_stdout is true, prog_stdout is ignored.
 */
static pid_t
create_pipe (const char *progname,
             const char *prog_path,
             const char * const *prog_argv,
             const char *directory,
             bool pipe_stdin, bool pipe_stdout,
             const char *prog_stdin, const char *prog_stdout,
             bool null_stderr,
             bool slave_process, bool exit_on_error,
             int fd[2])
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
                canonicalize_filename_mode (prog_path, CAN_MISSING | CAN_NOLINKS);
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

  int ifd[2];
  int ofd[2];

  /* It is important to create the file descriptors with the close-on-exec bit
     set.
     * In the child process that we are about to create here, the file
       descriptors ofd[0] -> STDIN_FILENO and ifd[1] -> STDOUT_FILENO will be
       preserved across exec, because each dup2 call scheduled by
       posix_spawn_file_actions_adddup2 creates a file descriptor with the
       close-on-exec bit clear.  Similarly on native Windows, where we use
       explicit DuplicateHandle calls, and on kLIBC, where we use explicit dup2
       calls.
     * In the parent process, we close ofd[0] and ifd[1]; so, ofd[1] and ofd[0]
       are still open. But if the parent process spawns another child process
       later, if ofd[1] and ofd[0] were inherited by that child process, the
       "end of input" / "end of output" detection would not work any more.  The
       parent or the child process would block, as long as that other child
       process is running.  */
  if (pipe_stdout)
    if (pipe2_safer (ifd, O_BINARY | O_CLOEXEC) < 0)
      error (EXIT_FAILURE, errno, _("cannot create pipe"));
  if (pipe_stdin)
    if (pipe2_safer (ofd, O_BINARY | O_CLOEXEC) < 0)
      error (EXIT_FAILURE, errno, _("cannot create pipe"));
/* Data flow diagram:
 *
 *           write        system         read
 *    parent  ->   ofd[1]   ->   ofd[0]   ->   child       if pipe_stdin
 *    parent  <-   ifd[0]   <-   ifd[1]   <-   child       if pipe_stdout
 *           read         system         write
 *
 */

#if ((defined _WIN32 && !defined __CYGWIN__) && SPAWN_PIPE_IMPL_AVOID_POSIX_SPAWN) || defined __KLIBC__

  /* Native Windows API.
     This uses _pipe(), dup2(), and _spawnv().  It could also be implemented
     using the low-level functions CreatePipe(), DuplicateHandle(),
     CreateProcess() and _open_osfhandle(); see the GNU make and GNU clisp
     and cvs source code.  */
  char *argv_mem_to_free;
  int child;
  int nulloutfd;
  int stdinfd;
  int stdoutfd;

  const char **argv = prepare_spawn (prog_argv, &argv_mem_to_free);
  if (argv == NULL)
    xalloc_die ();

  child = -1;

# if (defined _WIN32 && !defined __CYGWIN__) && SPAWN_PIPE_IMPL_AVOID_POSIX_SPAWN
  bool must_close_ifd1 = pipe_stdout;
  bool must_close_ofd0 = pipe_stdin;

  /* Create standard file handles of child process.  */
  HANDLE stdin_handle = INVALID_HANDLE_VALUE;
  HANDLE stdout_handle = INVALID_HANDLE_VALUE;
  nulloutfd = -1;
  stdinfd = -1;
  stdoutfd = -1;
  if ((!null_stderr
       || (nulloutfd = open ("NUL", O_RDWR, 0)) >= 0)
      && (pipe_stdin
          || prog_stdin == NULL
          || (stdinfd = open (prog_stdin, O_RDONLY, 0)) >= 0)
      && (pipe_stdout
          || prog_stdout == NULL
          || (stdoutfd = open (prog_stdout, O_WRONLY, 0)) >= 0))
    /* The child process doesn't inherit ifd[0], ifd[1], ofd[0], ofd[1],
       but it inherits the three STD*_FILENO for which we pass the handles.  */
    /* Pass the environment explicitly.  This is needed if the program has
       modified the environment using putenv() or [un]setenv().  On Windows,
       processes have two environments, one in the "environment block" of the
       process and managed through SetEnvironmentVariable(), and one inside the
       process, in the location retrieved by the 'environ' macro.  If we were
       to pass NULL, the child process would inherit a copy of the environment
       block - ignoring the effects of putenv() and [un]setenv().  */
    {
      stdin_handle =
        (HANDLE) _get_osfhandle (pipe_stdin ? ofd[0] :
                                 prog_stdin == NULL ? STDIN_FILENO : stdinfd);
      if (pipe_stdin)
        {
          HANDLE curr_process = GetCurrentProcess ();
          HANDLE duplicate;
          if (!DuplicateHandle (curr_process, stdin_handle,
                                curr_process, &duplicate,
                                0, TRUE, DUPLICATE_SAME_ACCESS))
            {
              errno = EBADF; /* arbitrary */
              goto failed;
            }
          must_close_ofd0 = false;
          close (ofd[0]); /* implies CloseHandle (stdin_handle); */
          stdin_handle = duplicate;
        }
      stdout_handle =
        (HANDLE) _get_osfhandle (pipe_stdout ? ifd[1] :
                                 prog_stdout == NULL ? STDOUT_FILENO : stdoutfd);
      if (pipe_stdout)
        {
          HANDLE curr_process = GetCurrentProcess ();
          HANDLE duplicate;
          if (!DuplicateHandle (curr_process, stdout_handle,
                                curr_process, &duplicate,
                                0, TRUE, DUPLICATE_SAME_ACCESS))
            {
              errno = EBADF; /* arbitrary */
              goto failed;
            }
          must_close_ifd1 = false;
          close (ifd[1]); /* implies CloseHandle (stdout_handle); */
          stdout_handle = duplicate;
        }
      HANDLE stderr_handle =
        (HANDLE) _get_osfhandle (null_stderr ? nulloutfd : STDERR_FILENO);

      child = spawnpvech (P_NOWAIT, prog_path, argv + 1,
                          (const char * const *) environ, directory,
                          stdin_handle, stdout_handle, stderr_handle);
#  if 0 /* Executing arbitrary files as shell scripts is unsecure.  */
      if (child == -1 && errno == ENOEXEC)
        {
          /* prog is not a native executable.  Try to execute it as a
             shell script.  Note that prepare_spawn() has already prepended
             a hidden element "sh.exe" to argv.  */
          argv[1] = prog_path;
          child = spawnpvech (P_NOWAIT, argv[0], argv,
                              (const char * const *) environ, directory,
                              stdin_handle, stdout_handle, stderr_handle);
        }
#  endif
    }
 failed:
  if (child == -1)
    saved_errno = errno;
  if (stdinfd >= 0)
    close (stdinfd);
  if (stdoutfd >= 0)
    close (stdoutfd);
  if (nulloutfd >= 0)
    close (nulloutfd);

  if (pipe_stdin)
    {
      if (must_close_ofd0)
        close (ofd[0]);
      else
        if (stdin_handle != INVALID_HANDLE_VALUE)
          CloseHandle (stdin_handle);
    }
  if (pipe_stdout)
    {
      if (must_close_ifd1)
        close (ifd[1]);
      else
        if (stdout_handle != INVALID_HANDLE_VALUE)
          CloseHandle (stdout_handle);
    }

# else /* __KLIBC__ */
  if (!(directory == NULL || strcmp (directory, ".") == 0))
    {
      /* A directory argument is not supported in this implementation.  */
      saved_errno = EINVAL;
      goto fail_with_saved_errno;
    }

  int orig_stdin;
  int orig_stdout;
  int orig_stderr;

  /* Save standard file handles of parent process.  */
  if (pipe_stdin || prog_stdin != NULL)
    orig_stdin = dup_safer_noinherit (STDIN_FILENO);
  if (pipe_stdout || prog_stdout != NULL)
    orig_stdout = dup_safer_noinherit (STDOUT_FILENO);
  if (null_stderr)
    orig_stderr = dup_safer_noinherit (STDERR_FILENO);

  /* Create standard file handles of child process.  */
  nulloutfd = -1;
  stdinfd = -1;
  stdoutfd = -1;
  if ((!pipe_stdin || dup2 (ofd[0], STDIN_FILENO) >= 0)
      && (!pipe_stdout || dup2 (ifd[1], STDOUT_FILENO) >= 0)
      && (!null_stderr
          || ((nulloutfd = open ("NUL", O_RDWR, 0)) >= 0
              && (nulloutfd == STDERR_FILENO
                  || (dup2 (nulloutfd, STDERR_FILENO) >= 0
                      && close (nulloutfd) >= 0))))
      && (pipe_stdin
          || prog_stdin == NULL
          || ((stdinfd = open (prog_stdin, O_RDONLY, 0)) >= 0
              && (stdinfd == STDIN_FILENO
                  || (dup2 (stdinfd, STDIN_FILENO) >= 0
                      && close (stdinfd) >= 0))))
      && (pipe_stdout
          || prog_stdout == NULL
          || ((stdoutfd = open (prog_stdout, O_WRONLY, 0)) >= 0
              && (stdoutfd == STDOUT_FILENO
                  || (dup2 (stdoutfd, STDOUT_FILENO) >= 0
                      && close (stdoutfd) >= 0)))))
    /* The child process doesn't inherit ifd[0], ifd[1], ofd[0], ofd[1],
       but it inherits all open()ed or dup2()ed file handles (which is what
       we want in the case of STD*_FILENO).  */
    {
      child = _spawnvpe (P_NOWAIT, prog_path, argv + 1,
                         (const char **) environ);
#  if 0 /* Executing arbitrary files as shell scripts is unsecure.  */
      if (child == -1 && errno == ENOEXEC)
        {
          /* prog is not a native executable.  Try to execute it as a
             shell script.  Note that prepare_spawn() has already prepended
             a hidden element "sh.exe" to argv.  */
          child = _spawnvpe (P_NOWAIT, argv[0], argv,
                             (const char **) environ);
        }
#  endif
    }
  if (child == -1)
    saved_errno = errno;
  if (stdinfd >= 0)
    close (stdinfd);
  if (stdoutfd >= 0)
    close (stdoutfd);
  if (nulloutfd >= 0)
    close (nulloutfd);

  /* Restore standard file handles of parent process.  */
  if (null_stderr)
    undup_safer_noinherit (orig_stderr, STDERR_FILENO);
  if (pipe_stdout || prog_stdout != NULL)
    undup_safer_noinherit (orig_stdout, STDOUT_FILENO);
  if (pipe_stdin || prog_stdin != NULL)
    undup_safer_noinherit (orig_stdin, STDIN_FILENO);

  if (pipe_stdin)
    close (ofd[0]);
  if (pipe_stdout)
    close (ifd[1]);
# endif

  free (argv);
  free (argv_mem_to_free);
  free (prog_path_to_free);

  if (child == -1)
    {
      if (pipe_stdout)
        close (ifd[0]);
      if (pipe_stdin)
        close (ofd[1]);
      goto fail_with_saved_errno;
    }

  if (pipe_stdout)
    fd[0] = ifd[0];
  if (pipe_stdin)
    fd[1] = ofd[1];
  return child;

#else

  /* Unix API.  */
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
          (pipe_stdin
           && (err = posix_spawn_file_actions_adddup2 (&actions,
                                                       ofd[0], STDIN_FILENO))
              != 0)
          || (pipe_stdout
              && (err = posix_spawn_file_actions_adddup2 (&actions,
                                                          ifd[1], STDOUT_FILENO))
                 != 0)
          || (pipe_stdin
              && (err = posix_spawn_file_actions_addclose (&actions, ofd[0]))
                 != 0)
          || (pipe_stdout
              && (err = posix_spawn_file_actions_addclose (&actions, ifd[1]))
                 != 0)
          || (pipe_stdin
              && (err = posix_spawn_file_actions_addclose (&actions, ofd[1]))
                 != 0)
          || (pipe_stdout
              && (err = posix_spawn_file_actions_addclose (&actions, ifd[0]))
                 != 0)
          || (null_stderr
              && (err = posix_spawn_file_actions_addopen (&actions,
                                                          STDERR_FILENO,
                                                          "/dev/null", O_RDWR,
                                                          0))
                 != 0)
          || (!pipe_stdin
              && prog_stdin != NULL
              && (err = posix_spawn_file_actions_addopen (&actions,
                                                          STDIN_FILENO,
                                                          prog_stdin, O_RDONLY,
                                                          0))
                 != 0)
          || (!pipe_stdout
              && prog_stdout != NULL
              && (err = posix_spawn_file_actions_addopen (&actions,
                                                          STDOUT_FILENO,
                                                          prog_stdout, O_WRONLY,
                                                          0))
                 != 0)
          || (directory != NULL
              && (err = posix_spawn_file_actions_addchdir (&actions,
                                                           directory)))
          || (slave_process
              && ((err = posix_spawnattr_init (&attrs)) != 0
                  || (attrs_allocated = true,
# if defined _WIN32 && !defined __CYGWIN__
                      (err = posix_spawnattr_setpgroup (&attrs, 0)) != 0
                      || (err = posix_spawnattr_setflags (&attrs,
                                                         POSIX_SPAWN_SETPGROUP))
                         != 0
# else
                      (err = posix_spawnattr_setsigmask (&attrs,
                                                         &blocked_signals))
                      != 0
                      || (err = posix_spawnattr_setflags (&attrs,
                                                        POSIX_SPAWN_SETSIGMASK))
                         != 0
# endif
             )   )   )
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
      if (pipe_stdout)
        {
          close (ifd[0]);
          close (ifd[1]);
        }
      if (pipe_stdin)
        {
          close (ofd[0]);
          close (ofd[1]);
        }
      free (prog_path_to_free);
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
  if (pipe_stdin)
    close (ofd[0]);
  if (pipe_stdout)
    close (ifd[1]);
  free (prog_path_to_free);

  if (pipe_stdout)
    fd[0] = ifd[0];
  if (pipe_stdin)
    fd[1] = ofd[1];
  return child;

#endif

 fail_with_errno:
  saved_errno = errno;
 fail_with_saved_errno:
  if (exit_on_error || !null_stderr)
    error (exit_on_error ? EXIT_FAILURE : 0, saved_errno,
           _("%s subprocess failed"), progname);
  errno = saved_errno;
  return -1;
}

/* Open a bidirectional pipe.
 *
 *           write       system                read
 *    parent  ->   fd[1]   ->   STDIN_FILENO    ->   child
 *    parent  <-   fd[0]   <-   STDOUT_FILENO   <-   child
 *           read        system                write
 *
 */
pid_t
create_pipe_bidi (const char *progname,
                  const char *prog_path, const char * const *prog_argv,
                  const char *directory,
                  bool null_stderr,
                  bool slave_process, bool exit_on_error,
                  int fd[2])
{
  pid_t result = create_pipe (progname, prog_path, prog_argv, directory,
                              true, true, NULL, NULL,
                              null_stderr, slave_process, exit_on_error,
                              fd);
  return result;
}

/* Open a pipe for input from a child process.
 * The child's stdin comes from a file.
 *
 *           read        system                write
 *    parent  <-   fd[0]   <-   STDOUT_FILENO   <-   child
 *
 */
pid_t
create_pipe_in (const char *progname,
                const char *prog_path, const char * const *prog_argv,
                const char *directory,
                const char *prog_stdin, bool null_stderr,
                bool slave_process, bool exit_on_error,
                int fd[1])
{
  int iofd[2];
  pid_t result = create_pipe (progname, prog_path, prog_argv, directory,
                              false, true, prog_stdin, NULL,
                              null_stderr, slave_process, exit_on_error,
                              iofd);
  if (result != -1)
    fd[0] = iofd[0];
  return result;
}

/* Open a pipe for output to a child process.
 * The child's stdout goes to a file.
 *
 *           write       system                read
 *    parent  ->   fd[0]   ->   STDIN_FILENO    ->   child
 *
 */
pid_t
create_pipe_out (const char *progname,
                 const char *prog_path, const char * const *prog_argv,
                 const char *directory,
                 const char *prog_stdout, bool null_stderr,
                 bool slave_process, bool exit_on_error,
                 int fd[1])
{
  int iofd[2];
  pid_t result = create_pipe (progname, prog_path, prog_argv, directory,
                              true, false, NULL, prog_stdout,
                              null_stderr, slave_process, exit_on_error,
                              iofd);
  if (result != -1)
    fd[0] = iofd[1];
  return result;
}
