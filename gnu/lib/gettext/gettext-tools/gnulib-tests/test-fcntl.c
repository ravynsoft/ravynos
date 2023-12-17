/* Test of fcntl(2).
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

/* Specification.  */
#include <fcntl.h>

#include "signature.h"
SIGNATURE_CHECK (fcntl, int, (int, int, ...));

/* Helpers.  */
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

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

#include "binary-io.h"
#include "macros.h"

/* Tell GCC not to warn about the specific edge cases tested here.  */
#if __GNUC__ >= 13
# pragma GCC diagnostic ignored "-Wanalyzer-fd-leak"
# pragma GCC diagnostic ignored "-Wanalyzer-va-arg-type-mismatch"
#endif

#if !O_BINARY
# define set_binary_mode my_set_binary_mode
static int
set_binary_mode (_GL_UNUSED int fd, _GL_UNUSED int mode)
{
  return 0;
}
#endif

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
    return false;
  return (flags & HANDLE_FLAG_INHERIT) != 0;
#else
# ifndef F_GETFD
#  error Please port fcntl to your platform
# endif
  int i = fcntl (fd, F_GETFD);
  return 0 <= i && (i & FD_CLOEXEC) == 0;
#endif
}

/* Return non-zero if FD is open in the given MODE, which is either
   O_TEXT or O_BINARY.  */
static bool
is_mode (int fd, int mode)
{
  int value = set_binary_mode (fd, O_BINARY);
  set_binary_mode (fd, value);
  return mode == value;
}

/* Since native fcntl can have more supported operations than our
   replacement is aware of, and since various operations assign
   different types to the vararg argument, a wrapper around fcntl must
   be able to pass a vararg of unknown type on through to the original
   fcntl.  Make sure that this works properly: func1 behaves like the
   original fcntl interpreting the vararg as an int or a pointer to a
   struct, and func2 behaves like rpl_fcntl that doesn't know what
   type to forward.  */
struct dummy_struct
{
  long filler;
  int value;
};
static int
func1 (int a, ...)
{
  va_list arg;
  int i;
  va_start (arg, a);
  if (a < 4)
    i = va_arg (arg, int);
  else
    {
      struct dummy_struct *s = va_arg (arg, struct dummy_struct *);
      i = s->value;
    }
  va_end (arg);
  return i;
}
static int
func2 (int a, ...)
{
  va_list arg;
  void *p;
  va_start (arg, a);
  p = va_arg (arg, void *);
  va_end (arg);
  return func1 (a, p);
}

/* Ensure that all supported fcntl actions are distinct, and
   usable in preprocessor expressions.  */
static void
check_flags (void)
{
  switch (0)
    {
    case F_DUPFD:
#if F_DUPFD
#endif

    case F_DUPFD_CLOEXEC:
#if F_DUPFD_CLOEXEC
#endif

    case F_GETFD:
#if F_GETFD
#endif

#ifdef F_SETFD
    case F_SETFD:
# if F_SETFD
# endif
#endif

#ifdef F_GETFL
    case F_GETFL:
# if F_GETFL
# endif
#endif

#ifdef F_SETFL
    case F_SETFL:
# if F_SETFL
# endif
#endif

#ifdef F_GETOWN
    case F_GETOWN:
# if F_GETOWN
# endif
#endif

#ifdef F_SETOWN
    case F_SETOWN:
# if F_SETOWN
# endif
#endif

#ifdef F_GETLK
    case F_GETLK:
# if F_GETLK
# endif
#endif

#ifdef F_SETLK
    case F_SETLK:
# if F_SETLK
# endif
#endif

#ifdef F_SETLKW
    case F_SETLKW:
# if F_SETLKW
# endif
#endif

    default:
      ;
    }
}

int
main (int argc, _GL_UNUSED char *argv[])
{
  if (argc > 1)
    /* child process */
    return (is_open (10) ? 42 : 0);

  const char *file = "test-fcntl.tmp";
  int fd;
  int bad_fd = getdtablesize ();

  /* Sanity check that rpl_fcntl is likely to work.  */
  ASSERT (func2 (1, 2) == 2);
  ASSERT (func2 (2, -2) == -2);
  ASSERT (func2 (3, 0x80000000) == 0x80000000);
  {
    struct dummy_struct s = { 0L, 4 };
    ASSERT (func2 (4, &s) == 4);
  }
  check_flags ();

  /* Assume std descriptors were provided by invoker, and ignore fds
     that might have been inherited.  */
  fd = creat (file, 0600);
  ASSERT (STDERR_FILENO < fd);
  close (fd + 1);
  close (fd + 2);

  /* For F_DUPFD*, the source must be valid.  */
  errno = 0;
  ASSERT (fcntl (-1, F_DUPFD, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_DUPFD, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_DUPFD, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (-1, F_DUPFD_CLOEXEC, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_DUPFD_CLOEXEC, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_DUPFD_CLOEXEC, 0) == -1);
  ASSERT (errno == EBADF);

  /* For F_DUPFD*, the destination must be valid.  */
  errno = 0;
  ASSERT (fcntl (fd, F_DUPFD, -1) == -1);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (fcntl (fd, F_DUPFD, bad_fd) == -1);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (fcntl (fd, F_DUPFD_CLOEXEC, -1) == -1);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (fcntl (fd, F_DUPFD_CLOEXEC, bad_fd) == -1);
  ASSERT (errno == EINVAL
          || errno == EMFILE /* WSL */);

  /* For F_DUPFD*, check for correct inheritance, as well as
     preservation of text vs. binary.  */
  set_binary_mode (fd, O_BINARY);
  ASSERT (is_open (fd));
  ASSERT (!is_open (fd + 1));
  ASSERT (!is_open (fd + 2));
  ASSERT (is_inheritable (fd));
  ASSERT (is_mode (fd, O_BINARY));

  ASSERT (fcntl (fd, F_DUPFD, fd) == fd + 1);
  ASSERT (is_open (fd));
  ASSERT (is_open (fd + 1));
  ASSERT (!is_open (fd + 2));
  ASSERT (is_inheritable (fd + 1));
  ASSERT (is_mode (fd, O_BINARY));
  ASSERT (is_mode (fd + 1, O_BINARY));
  ASSERT (close (fd + 1) == 0);

  ASSERT (fcntl (fd, F_DUPFD_CLOEXEC, fd + 2) == fd + 2);
  ASSERT (is_open (fd));
  ASSERT (!is_open (fd + 1));
  ASSERT (is_open (fd + 2));
  ASSERT (is_inheritable (fd));
  ASSERT (!is_inheritable (fd + 2));
  ASSERT (is_mode (fd, O_BINARY));
  ASSERT (is_mode (fd + 2, O_BINARY));
  ASSERT (close (fd) == 0);

  set_binary_mode (fd + 2, O_TEXT);
  ASSERT (fcntl (fd + 2, F_DUPFD, fd + 1) == fd + 1);
  ASSERT (!is_open (fd));
  ASSERT (is_open (fd + 1));
  ASSERT (is_open (fd + 2));
  ASSERT (is_inheritable (fd + 1));
  ASSERT (!is_inheritable (fd + 2));
  ASSERT (is_mode (fd + 1, O_TEXT));
  ASSERT (is_mode (fd + 2, O_TEXT));
  ASSERT (close (fd + 1) == 0);

  ASSERT (fcntl (fd + 2, F_DUPFD_CLOEXEC, 0) == fd);
  ASSERT (is_open (fd));
  ASSERT (!is_open (fd + 1));
  ASSERT (is_open (fd + 2));
  ASSERT (!is_inheritable (fd));
  ASSERT (!is_inheritable (fd + 2));
  ASSERT (is_mode (fd, O_TEXT));
  ASSERT (is_mode (fd + 2, O_TEXT));
  ASSERT (close (fd + 2) == 0);

  /* Test F_GETFD on invalid file descriptors.  */
  errno = 0;
  ASSERT (fcntl (-1, F_GETFD) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_GETFD) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_GETFD) == -1);
  ASSERT (errno == EBADF);

  /* Test F_GETFD, the FD_CLOEXEC bit.  */
  {
    int result = fcntl (fd, F_GETFD);
    ASSERT (0 <= result);
    ASSERT ((result & FD_CLOEXEC) == FD_CLOEXEC);
    ASSERT (dup (fd) == fd + 1);
    result = fcntl (fd + 1, F_GETFD);
    ASSERT (0 <= result);
    ASSERT ((result & FD_CLOEXEC) == 0);
    ASSERT (close (fd + 1) == 0);
  }

#ifdef F_SETFD
  /* Test F_SETFD on invalid file descriptors.  */
  errno = 0;
  ASSERT (fcntl (-1, F_SETFD, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_SETFD, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_SETFD, 0) == -1);
  ASSERT (errno == EBADF);
#endif

#ifdef F_GETFL
  /* Test F_GETFL on invalid file descriptors.  */
  errno = 0;
  ASSERT (fcntl (-1, F_GETFL) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_GETFL) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_GETFL) == -1);
  ASSERT (errno == EBADF);
#endif

#ifdef F_SETFL
  /* Test F_SETFL on invalid file descriptors.  */
  errno = 0;
  ASSERT (fcntl (-1, F_SETFL, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_SETFL, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_SETFL, 0) == -1);
  ASSERT (errno == EBADF);
#endif

#ifdef F_GETOWN
  /* Test F_GETOWN on invalid file descriptors.  */
  errno = 0;
  ASSERT (fcntl (-1, F_GETOWN) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_GETOWN) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_GETOWN) == -1);
  ASSERT (errno == EBADF);
#endif

#ifdef F_SETOWN
  /* Test F_SETFL on invalid file descriptors.  */
  errno = 0;
  ASSERT (fcntl (-1, F_SETOWN, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (fd + 1, F_SETOWN, 0) == -1);
  ASSERT (errno == EBADF);
  errno = 0;
  ASSERT (fcntl (bad_fd, F_SETOWN, 0) == -1);
  ASSERT (errno == EBADF);
#endif

  /* Cleanup.  */
  ASSERT (close (fd) == 0);
  ASSERT (unlink (file) == 0);

  /* Close file descriptors that may have been inherited from the parent
     process and that would cause failures below.
     Such file descriptors have been seen:
       - with GNU make, when invoked as 'make -j N' with j > 1,
       - in some versions of the KDE desktop environment,
       - on NetBSD,
       - in MacPorts with the "trace mode" enabled.
   */
  (void) close (10);

  /* Test whether F_DUPFD_CLOEXEC is effective.  */
  ASSERT (fcntl (1, F_DUPFD_CLOEXEC, 10) >= 0);
#if defined _WIN32 && !defined __CYGWIN__
  return _execl ("./test-fcntl", "./test-fcntl", "child", NULL);
#else
  return execl ("./test-fcntl", "./test-fcntl", "child", NULL);
#endif
}
