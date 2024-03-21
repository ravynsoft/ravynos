/* Test duplicating file descriptors.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake <ebb9@byu.net>, 2009.  */

#include <config.h>

#include <unistd.h>

#include "signature.h"
SIGNATURE_CHECK (dup2, int, (int, int));

#include <errno.h>
#include <fcntl.h>

#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#include "binary-io.h"

#if GNULIB_TEST_CLOEXEC
# include "cloexec.h"
#endif

#if defined _WIN32 && ! defined __CYGWIN__
/* Get declarations of the native Windows API functions.  */
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
/* Get _get_osfhandle.  */
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif
#endif

#include "macros.h"

/* Tell GCC not to warn about the specific edge cases tested here.  */
#if __GNUC__ >= 13
# pragma GCC diagnostic ignored "-Wanalyzer-fd-leak"
# pragma GCC diagnostic ignored "-Wanalyzer-fd-use-without-check"
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

#if GNULIB_TEST_CLOEXEC
/* Return non-zero if FD is open and inheritable across exec/spawn.  */
static int
is_inheritable (int fd)
{
# if defined _WIN32 && ! defined __CYGWIN__
  /* On native Windows, the initial state of unassigned standard file
     descriptors is that they are open but point to an
     INVALID_HANDLE_VALUE, and there is no fcntl.  */
  HANDLE h = (HANDLE) _get_osfhandle (fd);
  DWORD flags;
  if (h == INVALID_HANDLE_VALUE || GetHandleInformation (h, &flags) == 0)
    return 0;
  return (flags & HANDLE_FLAG_INHERIT) != 0;
# else
#  ifndef F_GETFD
#   error Please port fcntl to your platform
#  endif
  int i = fcntl (fd, F_GETFD);
  return 0 <= i && (i & FD_CLOEXEC) == 0;
# endif
}
#endif /* GNULIB_TEST_CLOEXEC */

#if !O_BINARY
# define set_binary_mode my_set_binary_mode
static int
set_binary_mode (_GL_UNUSED int fd, _GL_UNUSED int mode)
{
  return 0;
}
#endif

/* Return non-zero if FD is open in the given MODE, which is either
   O_TEXT or O_BINARY.  */
static int
is_mode (int fd, int mode)
{
  int value = set_binary_mode (fd, O_BINARY);
  set_binary_mode (fd, value);
  return mode == value;
}

int
main (void)
{
  const char *file = "test-dup2.tmp";
  char buffer[1];
  int bad_fd = getdtablesize ();
  int fd = open (file, O_CREAT | O_TRUNC | O_RDWR, 0600);

  /* Assume std descriptors were provided by invoker.  */
  ASSERT (STDERR_FILENO < fd);
  ASSERT (is_open (fd));
  /* Ignore any other fd's leaked into this process.  */
  close (fd + 1);
  close (fd + 2);
  ASSERT (!is_open (fd + 1));
  ASSERT (!is_open (fd + 2));

  /* Assigning to self must be a no-op.  */
  ASSERT (dup2 (fd, fd) == fd);
  ASSERT (is_open (fd));

  /* The source must be valid.  */
  errno = 0;
  ASSERT (dup2 (-1, fd) == -1);
  ASSERT (errno == EBADF);
  close (99);
  errno = 0;
  ASSERT (dup2 (99, fd) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (dup2 (AT_FDCWD, fd) == -1);
  ASSERT (errno == EBADF);
  ASSERT (is_open (fd));

  /* If the source is not open, then the destination is unaffected.  */
  errno = 0;
  ASSERT (dup2 (fd + 1, fd + 1) == -1);
  ASSERT (errno == EBADF);
  ASSERT (!is_open (fd + 1));
  errno = 0;
  ASSERT (dup2 (fd + 1, fd) == -1);
  ASSERT (errno == EBADF);
  ASSERT (is_open (fd));

  /* The destination must be valid.  */
  errno = 0;
  ASSERT (dup2 (fd, -2) == -1);
  ASSERT (errno == EBADF);
  if (bad_fd > 256)
    {
      ASSERT (dup2 (fd, 255) == 255);
      ASSERT (dup2 (fd, 256) == 256);
      ASSERT (close (255) == 0);
      ASSERT (close (256) == 0);
    }
  ASSERT (dup2 (fd, bad_fd - 1) == bad_fd - 1);
  ASSERT (close (bad_fd - 1) == 0);
  errno = 0;
  ASSERT (dup2 (fd, bad_fd) == -1);
  ASSERT (errno == EBADF);

  /* Using dup2 can skip fds.  */
  ASSERT (dup2 (fd, fd + 2) == fd + 2);
  ASSERT (is_open (fd));
  ASSERT (!is_open (fd + 1));
  ASSERT (is_open (fd + 2));

  /* Verify that dup2 closes the previous occupant of a fd.  */
  ASSERT (open ("/dev/null", O_WRONLY, 0600) == fd + 1);
  ASSERT (dup2 (fd + 1, fd) == fd);
  ASSERT (close (fd + 1) == 0);
  ASSERT (write (fd, "1", 1) == 1);
  ASSERT (dup2 (fd + 2, fd) == fd);
  ASSERT (lseek (fd, 0, SEEK_END) == 0);
  ASSERT (write (fd + 2, "2", 1) == 1);
  ASSERT (lseek (fd, 0, SEEK_SET) == 0);
  ASSERT (read (fd, buffer, 1) == 1);
  ASSERT (*buffer == '2');

#if GNULIB_TEST_CLOEXEC
  /* Any new fd created by dup2 must not be cloexec.  */
  ASSERT (close (fd + 2) == 0);
  ASSERT (dup_cloexec (fd) == fd + 1);
  ASSERT (!is_inheritable (fd + 1));
  ASSERT (dup2 (fd + 1, fd + 1) == fd + 1);
  ASSERT (!is_inheritable (fd + 1));
  ASSERT (dup2 (fd + 1, fd + 2) == fd + 2);
  ASSERT (!is_inheritable (fd + 1));
  ASSERT (is_inheritable (fd + 2));
  errno = 0;
  ASSERT (dup2 (fd + 1, -1) == -1);
  ASSERT (errno == EBADF);
  ASSERT (!is_inheritable (fd + 1));
#endif

  /* On systems that distinguish between text and binary mode, dup2
     reuses the mode of the source.  */
  set_binary_mode (fd, O_BINARY);
  ASSERT (is_mode (fd, O_BINARY));
  ASSERT (dup2 (fd, fd + 1) == fd + 1);
  ASSERT (is_mode (fd + 1, O_BINARY));
  set_binary_mode (fd, O_TEXT);
  ASSERT (is_mode (fd, O_TEXT));
  ASSERT (dup2 (fd, fd + 1) == fd + 1);
  ASSERT (is_mode (fd + 1, O_TEXT));

  /* Clean up.  */
  ASSERT (close (fd + 2) == 0);
  ASSERT (close (fd + 1) == 0);
  ASSERT (close (fd) == 0);
  ASSERT (unlink (file) == 0);

  return 0;
}
