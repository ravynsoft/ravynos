/* Test the write() function.
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
SIGNATURE_CHECK (write, ssize_t, (int, const void *, size_t));

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "macros.h"

int
main (void)
{
  const char *filename = "test-write.tmp";
  int fd;

  /* Create a file with a simple contents.  */
  fd = open (filename, O_CREAT | O_WRONLY, 0600);
  ASSERT (fd >= 0);
  ASSERT (write (fd, "Hello World", 11) == 11);
  ASSERT (close (fd) == 0);

  /* Write into the middle of the file.  */
  fd = open (filename, O_WRONLY);
  ASSERT (fd >= 0);
  ASSERT (lseek (fd, 6, SEEK_SET) == 6);
  ASSERT (write (fd, "fascination", 11) == 11);

  /* Verify the contents of the file.  */
  {
    char buf[64];
    int rfd = open (filename, O_RDONLY);
    ASSERT (rfd >= 0);
    ASSERT (read (rfd, buf, sizeof (buf)) == 17);
    ASSERT (close (rfd) == 0);
    ASSERT (memcmp (buf, "Hello fascination", 17) == 0);
  }

  ASSERT (close (fd) == 0);

  /* Test behaviour for invalid file descriptors.  */
  {
    char byte = 'x';
    errno = 0;
    ASSERT (write (-1, &byte, 1) == -1);
    ASSERT (errno == EBADF);
  }
  {
    char byte = 'x';
    close (99);
    errno = 0;
    ASSERT (write (99, &byte, 1) == -1);
    ASSERT (errno == EBADF);
  }

  /* Clean up.  */
  unlink (filename);

  return 0;
}
