/* Test truncating a file.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#include <unistd.h>

#include "signature.h"
SIGNATURE_CHECK (ftruncate, int, (int, off_t));

#include <errno.h>
#include <fcntl.h>

#include "macros.h"

int
main (_GL_UNUSED int argc, char *argv[])
{
  const char *filename = argv[1];

  /* Test behaviour for invalid file descriptors.  */
  {
    errno = 0;
    ASSERT (ftruncate (-1, 0) == -1);
    ASSERT (errno == EBADF);
  }
  {
    close (99);
    errno = 0;
    ASSERT (ftruncate (99, 0) == -1);
    ASSERT (errno == EBADF);
  }

  /* Test behaviour for read-only file descriptors.  */
  {
    int fd = open (filename, O_RDONLY);
    ASSERT (fd >= 0);
    errno = 0;
    ASSERT (ftruncate (fd, 0) == -1);
    ASSERT (errno == EBADF || errno == EINVAL
            || errno == EACCES /* seen on mingw */
           );
    close (fd);
  }

  return 0;
}
