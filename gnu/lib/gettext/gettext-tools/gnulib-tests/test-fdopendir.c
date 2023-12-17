/* Test opening a directory stream from a file descriptor.
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

#include <dirent.h>

#include "signature.h"
SIGNATURE_CHECK (fdopendir, DIR *, (int));

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "macros.h"

int
main ()
{
  DIR *d;
  int fd;

  /* A non-directory cannot be turned into a directory stream.  */
  fd = open ("test-fdopendir.tmp", O_RDONLY | O_CREAT, 0600);
  ASSERT (0 <= fd);
  errno = 0;
  ASSERT (fdopendir (fd) == NULL);
  ASSERT (errno == ENOTDIR);
  ASSERT (close (fd) == 0);
  ASSERT (unlink ("test-fdopendir.tmp") == 0);

  /* A bad fd cannot be turned into a stream.  */
  {
    errno = 0;
    ASSERT (fdopendir (-1) == NULL);
    ASSERT (errno == EBADF);
  }
  {
    close (99);
    errno = 0;
    ASSERT (fdopendir (99) == NULL);
    ASSERT (errno == EBADF);
  }

  /* This should work.  */
  fd = open (".", O_RDONLY);
  ASSERT (0 <= fd);
  d = fdopendir (fd);
  ASSERT (d);
  /* fdopendir should not close fd.  */
  ASSERT (dup2 (fd, fd) == fd);

  /* Don't test dirfd here.  dirfd (d) must return fd on current POSIX
     platforms, but on pre-2008 platforms or on non-POSIX platforms
     dirfd (fd) might return some other descriptor, or -1, and gnulib
     does not work around this porting problem.  */

  ASSERT (closedir (d) == 0);
  /* Now we can guarantee that fd must be closed.  */
  errno = 0;
  ASSERT (dup2 (fd, fd) == -1);
  ASSERT (errno == EBADF);

  return 0;
}
