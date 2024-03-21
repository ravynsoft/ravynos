/* Child program invoked by test-execute-main.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

/* If the user's config.h happens to include <sys/stat.h>, let it include only
   the system's <sys/stat.h> here.  */
#define __need_system_sys_stat_h
#include <config.h>

/* Get the original definition of fstat.  It might be defined as a macro.
   Also, 'stat' might be defined as a macro.  */
#include <sys/types.h>
#include <sys/stat.h>
#undef __need_system_sys_stat_h

/* Return non-zero if FD is opened to a device.  */
static int
is_device (int fd)
{
#if defined _WIN32 && ! defined __CYGWIN__
  struct _stat st;
  return _fstat (fd, &st) >= 0 && !((st.st_mode & S_IFMT) == S_IFREG);
#else
  struct stat st;
  return fstat (fd, &st) >= 0 && !S_ISREG (st.st_mode);
#endif
}

/* Now include the other header files.  */
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined _WIN32 && ! defined __CYGWIN__
/* Get declarations of the native Windows API functions.  */
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
/* Get _get_osfhandle, _isatty, _chdir, _getcwd.  */
# include <io.h>
#endif

/* In this file, we use only system functions, no overrides from gnulib.  */
#undef atoi
#undef close
#undef fcntl
#undef fflush
#undef fgetc
#undef fprintf
#undef fputs
#undef getcwd
#undef isatty
#undef open
#undef raise
#undef read
#undef sprintf
#undef strcasestr
#undef strcmp
#undef strlen
#undef strstr
#undef write

#include "qemu.h"

#if HAVE_MSVC_INVALID_PARAMETER_HANDLER
static void __cdecl
gl_msvc_invalid_parameter_handler (const wchar_t *expression,
                                   const wchar_t *function,
                                   const wchar_t *file,
                                   unsigned int line,
                                   uintptr_t dummy)
{
}
#endif

/* Return non-zero if FD is open.  */
static int
is_open (int fd)
{
#if defined _WIN32 && ! defined __CYGWIN__
  /* On native Windows, the initial state of unassigned standard file
     descriptors is that they are open but point to an
     INVALID_HANDLE_VALUE, and there is no fcntl.  */
  return (HANDLE) _get_osfhandle (fd) != INVALID_HANDLE_VALUE;
#else
# ifndef F_GETFL
#  error Please port fcntl to your platform
# endif
  return 0 <= fcntl (fd, F_GETFL);
#endif
}

int
main (int argc, char *argv[])
{
  if (argc == 1)
    /* Check an invocation without arguments.  Check the exit code.  */
    return 40;

  int test = atoi (argv[1]);
  switch (test)
    {
    case 2:
      /* Check argument passing.  */
      return !(argc == 12
               && strcmp (argv[2], "abc def") == 0
               && strcmp (argv[3], "abc\"def\"ghi") == 0
               && strcmp (argv[4], "xyz\"") == 0
               && strcmp (argv[5], "abc\\def\\ghi") == 0
               && strcmp (argv[6], "xyz\\") == 0
               && strcmp (argv[7], "???") == 0
               && strcmp (argv[8], "***") == 0
               && strcmp (argv[9], "") == 0
               && strcmp (argv[10], "foo") == 0
               && strcmp (argv[11], "") == 0);
    #if !(defined _WIN32 && !defined __CYGWIN__)
    case 3:
      /* Check SIGPIPE handling with ignore_sigpipe = false.  */
    case 4:
      /* Check SIGPIPE handling with ignore_sigpipe = true.  */
      raise (SIGPIPE);
      return 71;
    #endif
    case 5:
      /* Check other signal.  */
      raise (SIGINT);
      return 71;
    case 6:
      /* Check stdin is inherited.  */
      return !(fgetc (stdin) == 'F' && fgetc (stdin) == 'o');
    case 7:
      /* Check null_stdin = true.  */
      return !(fgetc (stdin) == EOF);
    case 8:
      /* Check stdout is inherited, part 1 (regular file).  */
      return !(fputs ("bar", stdout) != EOF && fflush (stdout) == 0);
    case 9:
      /* Check stdout is inherited, part 2 (device).  */
    case 10:
      /* Check null_stdout = true.  */
      return !is_device (STDOUT_FILENO);
    case 11:
      /* Check stderr is inherited, part 1 (regular file).  */
      return !(fputs ("bar", stderr) != EOF && fflush (stderr) == 0);
    case 12:
      /* Check stderr is inherited, part 2 (device).  */
    case 13:
      /* Check null_stderr = true.  */
      return !is_device (STDERR_FILENO);
    case 14:
    case 15:
      /* Check file descriptors >= 3 can be inherited.  */
    case 16:
      /* Check file descriptors >= 3 with O_CLOEXEC bit are not inherited.  */
      #if HAVE_MSVC_INVALID_PARAMETER_HANDLER
      /* Avoid exceptions from within _get_osfhandle.  */
      _set_invalid_parameter_handler (gl_msvc_invalid_parameter_handler);
      #endif
      {
        /* QEMU 6.1 in user-mode passes an open fd = 3, that references
           /dev/urandom.  We need to ignore this fd.  */
        bool is_qemu = is_running_under_qemu_user ();
        char buf[300];
        buf[0] = '\0';
        char *p = buf;
        int fd;
        for (fd = 0; fd < 20; fd++)
          if (is_open (fd) && !(is_qemu && fd == 3))
            {
              sprintf (p, "%d ", fd);
              p += strlen (p);
            }
        const char *expected = (test < 16 ? "0 1 2 10 " : "0 1 2 ");
        if (strcmp (buf, expected) == 0)
          return 0;
        else
          {
            fprintf (stderr, "Test case %d: %s\n", test, buf); fflush (stderr);
            return 1;
          }
      }
    case 17:
      /* Check that file descriptors >= 3, open for reading, can be inherited,
         including the file position.  */
      {
        char buf[6];
        int n = read (10, buf, sizeof (buf));
        return !(n == 4 && memcmp (buf, "obar", 4) == 0);
      }
    case 18:
      /* Check that file descriptors >= 3, open for writing, can be inherited,
         including the file position.  */
      {
        int n = write (10, "bar", 3);
        return !(n == 3);
      }
    case 19:
      /* Check that file descriptors >= 3, when inherited, preserve their
         isatty() property, part 1 (regular file).  */
    case 20:
      /* Check that file descriptors >= 3, when inherited, preserve their
         isatty() property, part 2 (character devices).  */
      {
        #if defined _WIN32 && ! defined __CYGWIN__
        return 4 + 2 * (_isatty (10) != 0) + (_isatty (11) != 0);
        #else
        return 4 + 2 * (isatty (10) != 0) + (isatty (11) != 0);
        #endif
      }
    case 21:
      /* Check execution in a different directory.  */
      {
        char cwd[1024];
        #if defined _WIN32 && ! defined __CYGWIN__
        if (_chdir ("..") != 0)
          return 1;
        if (_getcwd (cwd, sizeof (cwd)) == NULL)
          return 2;
        #else
        if (chdir ("..") != 0)
          return 1;
        if (getcwd (cwd, sizeof (cwd)) == NULL)
          return 2;
        #endif
        return (argc == 3 && strcmp (argv[2], cwd) == 0 ? 0 : 3);
      }
    default:
      abort ();
    }
}
