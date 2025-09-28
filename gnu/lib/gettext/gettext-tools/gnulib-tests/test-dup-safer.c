/* Test that dup_safer leaves standard fds alone.
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

#include "unistd--.h"

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "binary-io.h"
#include "cloexec.h"

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

#if !O_BINARY
# define set_binary_mode my_set_binary_mode
static int
set_binary_mode (_GL_UNUSED int fd, _GL_UNUSED int mode)
{
  return 0;
}
#endif

/* This test intentionally closes stderr.  So, we arrange to have fd 10
   (outside the range of interesting fd's during the test) set up to
   duplicate the original stderr.  */

#define BACKUP_STDERR_FILENO 10
#define ASSERT_STREAM myerr
#include "macros.h"

static FILE *myerr;

/* Return true if FD is open.  */
static bool
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

/* Return true if FD is open and inheritable across exec/spawn.  */
static bool
is_inheritable (int fd)
{
#if defined _WIN32 && ! defined __CYGWIN__
  /* On native Windows, the initial state of unassigned standard file
     descriptors is that they are open but point to an
     INVALID_HANDLE_VALUE, and there is no fcntl.  */
  HANDLE h = (HANDLE) _get_osfhandle (fd);
  DWORD flags;
  if (h == INVALID_HANDLE_VALUE || GetHandleInformation (h, &flags) == 0)
    return 0;
  return (flags & HANDLE_FLAG_INHERIT) != 0;
#else
# ifndef F_GETFD
#  error Please port fcntl to your platform
# endif
  int i = fcntl (fd, F_GETFD);
  return 0 <= i && (i & FD_CLOEXEC) == 0;
#endif
}

/* Return true if FD is open in the given MODE, which is either
   O_TEXT or O_BINARY.  */
static bool
is_mode (int fd, int mode)
{
  int value = set_binary_mode (fd, O_BINARY);
  set_binary_mode (fd, value);
  return mode == value;
}

#define witness "test-dup-safer.txt"

int
main (void)
{
  int i;
  int fd;
  int bad_fd = getdtablesize ();

  /* We close fd 2 later, so save it in fd 10.  */
  if (dup2 (STDERR_FILENO, BACKUP_STDERR_FILENO) != BACKUP_STDERR_FILENO
      || (myerr = fdopen (BACKUP_STDERR_FILENO, "w")) == NULL)
    return 2;

  /* Create file for later checks.  */
  fd = creat (witness, 0600);
  ASSERT (STDERR_FILENO < fd);

  /* Four iterations, with progressively more standard descriptors
     closed.  */
  for (i = -1; i <= STDERR_FILENO; i++)
    {
      if (0 <= i)
        ASSERT (close (i) == 0);

      /* Detect errors.  */
      errno = 0;
      ASSERT (dup (-1) == -1);
      ASSERT (errno == EBADF);
      errno = 0;
      ASSERT (dup (bad_fd) == -1);
      ASSERT (errno == EBADF);
      close (fd + 1);
      errno = 0;
      ASSERT (dup (fd + 1) == -1);
      ASSERT (errno == EBADF);

      /* Preserve text vs. binary.  */
      set_binary_mode (fd, O_BINARY);
      ASSERT (dup (fd) == fd + 1);
      ASSERT (is_open (fd + 1));
      ASSERT (is_inheritable (fd + 1));
      ASSERT (is_mode (fd + 1, O_BINARY));

      ASSERT (close (fd + 1) == 0);
      set_binary_mode (fd, O_TEXT);
      ASSERT (dup (fd) == fd + 1);
      ASSERT (is_open (fd + 1));
      ASSERT (is_inheritable (fd + 1));
      ASSERT (is_mode (fd + 1, O_TEXT));

      /* Create cloexec copy.  */
      ASSERT (close (fd + 1) == 0);
      ASSERT (fd_safer_flag (dup_cloexec (fd), O_CLOEXEC) == fd + 1);
      ASSERT (set_cloexec_flag (fd + 1, true) == 0);
      ASSERT (is_open (fd + 1));
      ASSERT (!is_inheritable (fd + 1));
      ASSERT (close (fd) == 0);

      /* dup always creates inheritable copies.  Also, check that
         earliest slot past std fds is used.  */
      ASSERT (dup (fd + 1) == fd);
      ASSERT (is_open (fd));
      ASSERT (is_inheritable (fd));
      ASSERT (close (fd + 1) == 0);
    }

  /* Cleanup.  */
  ASSERT (close (fd) == 0);
  ASSERT (unlink (witness) == 0);

  return 0;
}
