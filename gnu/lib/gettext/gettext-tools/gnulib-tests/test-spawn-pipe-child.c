/* Child program invoked by test-spawn-pipe-main.
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

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined _WIN32 && ! defined __CYGWIN__
/* Get declarations of the native Windows API functions.  */
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

/* Depending on arguments, this test intentionally closes stderr or
   starts life with stderr closed.  So, we arrange to have fd 10
   (outside the range of interesting fd's during the test) set up to
   duplicate the original stderr.  */

#define BACKUP_STDERR_FILENO 10
#define ASSERT_STREAM myerr
#include "macros.h"

static FILE *myerr;

/* In this file, we use only system functions, no overrides from gnulib.  */
#undef atoi
#undef close
#undef fcntl
#undef fdopen
#undef fflush
#undef fprintf
#undef open
#undef read
#undef strcasestr
#undef strstr
#undef write
#if defined _WIN32 && !defined __CYGWIN__
# define fdopen _fdopen
#endif

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
  /* fd 2 might be closed, but fd BACKUP_STDERR_FILENO is the original
     stderr.  */
  myerr = fdopen (BACKUP_STDERR_FILENO, "w");
  if (!myerr)
    return 2;

  ASSERT (argc == 2);

#if HAVE_MSVC_INVALID_PARAMETER_HANDLER
  /* Avoid exceptions from within _get_osfhandle.  */
  _set_invalid_parameter_handler (gl_msvc_invalid_parameter_handler);
#endif

  /* QEMU 6.1 in user-mode passes an open fd, usually = 3, that references
     /dev/urandom.  We need to ignore this fd.  */
  bool is_qemu = is_running_under_qemu_user ();

  /* Read one byte from fd 0, and write its value plus one to fd 1.
     fd 2 should be closed iff the argument is 1.  Check that no other file
     descriptors leaked.  */

  char buffer[2] = { 's', 't' };

  ASSERT (read (STDIN_FILENO, buffer, 2) == 1);

  buffer[0]++;
  ASSERT (write (STDOUT_FILENO, buffer, 1) == 1);

  switch (atoi (argv[1]))
    {
    case 0:
      /* Expect fd 2 is open.  */
      ASSERT (is_open (STDERR_FILENO));
      break;
    case 1:
      /* Expect fd 2 is closed.
         But on HP-UX 11, fd 2 gets automatically re-opened to /dev/null if it
         was closed.  Similarly on Android and on native Windows.  Future POSIX
         will allow this, see <http://austingroupbugs.net/view.php?id=173>.  */
#if !(defined __hpux || defined __ANDROID__ || (defined _WIN32 && ! defined __CYGWIN__))
      if (!is_qemu)
        ASSERT (! is_open (STDERR_FILENO));
#endif
      break;
    default:
      ASSERT (0);
    }

  int fd;
  for (fd = 3; fd < 7; fd++)
    if (!(is_qemu && fd == 3))
      {
        errno = 0;
        ASSERT (close (fd) == -1);
        ASSERT (errno == EBADF);
      }

  return 0;
}
