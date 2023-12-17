/* Test that openat works.
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

#include <fcntl.h>

#include "signature.h"
SIGNATURE_CHECK (openat, int, (int, char const *, int, ...));

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "macros.h"

#define BASE "test-openat.t"

#include "test-open.h"

static int dfd = AT_FDCWD;

/* Wrapper around openat to test open behavior.  */
static int
do_open (char const *name, int flags, ...)
{
  if (flags & O_CREAT)
    {
      mode_t mode = 0;
      va_list arg;
      va_start (arg, flags);

      /* We have to use PROMOTED_MODE_T instead of mode_t, otherwise GCC 4
         creates crashing code when 'mode_t' is smaller than 'int'.  */
      mode = va_arg (arg, PROMOTED_MODE_T);

      va_end (arg);
      return openat (dfd, name, flags, mode);
    }
  return openat (dfd, name, flags);
}

int
main ()
{
  int result;

  /* Test behaviour for invalid file descriptors.  */
  {
    errno = 0;
    ASSERT (openat (-1, "foo", O_RDONLY) == -1);
    ASSERT (errno == EBADF);
  }
  {
    close (99);
    errno = 0;
    ASSERT (openat (99, "foo", O_RDONLY) == -1);
    ASSERT (errno == EBADF);
  }

  /* Basic checks.  */
  result = test_open (do_open, false);
  dfd = open (".", O_RDONLY);
  ASSERT (0 <= dfd);
  ASSERT (test_open (do_open, false) == result);
  ASSERT (close (dfd) == 0);

  /* Check that even when *-safer modules are in use, plain openat can
     land in fd 0.  Do this test last, since it is destructive to
     stdin.  */
  ASSERT (close (STDIN_FILENO) == 0);
  ASSERT (openat (AT_FDCWD, ".", O_RDONLY) == STDIN_FILENO);
  {
    dfd = open (".", O_RDONLY);
    ASSERT (STDIN_FILENO < dfd);
    ASSERT (chdir ("..") == 0);
    ASSERT (close (STDIN_FILENO) == 0);
    ASSERT (openat (dfd, ".", O_RDONLY) == STDIN_FILENO);
    ASSERT (close (dfd) == 0);
  }
  return result;
}
