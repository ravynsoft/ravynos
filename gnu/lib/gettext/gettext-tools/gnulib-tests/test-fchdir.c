/* Test changing to a directory named by a file descriptor.
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
SIGNATURE_CHECK (fchdir, int, (int));

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "cloexec.h"
#include "macros.h"

int
main (void)
{
  char *cwd;
  int fd;
  int i;

  cwd = getcwd (NULL, 0);
  ASSERT (cwd);

  fd = open (".", O_RDONLY);
  ASSERT (0 <= fd);

  /* Test behaviour for invalid file descriptors.  */
  {
    errno = 0;
    ASSERT (fchdir (-1) == -1);
    ASSERT (errno == EBADF);
  }
  {
    close (99);
    errno = 0;
    ASSERT (fchdir (99) == -1);
    ASSERT (errno == EBADF);
  }

  /* Check for other failure cases.  */
  {
    int bad_fd = open ("/dev/null", O_RDONLY);
    ASSERT (0 <= bad_fd);
    errno = 0;
    ASSERT (fchdir (bad_fd) == -1);
    ASSERT (errno == ENOTDIR);
    ASSERT (close (bad_fd) == 0);
  }

  /* Repeat test twice, once in '.' and once in '..'.  */
  for (i = 0; i < 2; i++)
    {
      ASSERT (chdir (&".."[1 - i]) == 0);
      ASSERT (fchdir (fd) == 0);
      {
        size_t len = strlen (cwd) + 1;
        char *new_dir = malloc (len);
        ASSERT (new_dir);
        ASSERT (getcwd (new_dir, len) == new_dir);
        ASSERT (strcmp (cwd, new_dir) == 0);
        free (new_dir);
      }

      /* For second iteration, use a cloned fd, to ensure that dup
         remembers whether an fd was associated with a directory.  */
      if (!i)
        {
          int new_fd = dup (fd);
          ASSERT (0 <= new_fd);
          ASSERT (close (fd) == 0);
          ASSERT (dup2 (new_fd, fd) == fd);
          ASSERT (close (new_fd) == 0);
          ASSERT (dup_cloexec (fd) == new_fd);
          ASSERT (dup2 (new_fd, fd) == fd);
          ASSERT (close (new_fd) == 0);
          ASSERT (fcntl (fd, F_DUPFD_CLOEXEC, new_fd) == new_fd);
          ASSERT (close (fd) == 0);
          ASSERT (fcntl (new_fd, F_DUPFD, fd) == fd);
          ASSERT (close (new_fd) == 0);
#if GNULIB_TEST_DUP3
          ASSERT (dup3 (fd, new_fd, 0) == new_fd);
          ASSERT (dup3 (new_fd, fd, 0) == fd);
          ASSERT (close (new_fd) == 0);
#endif
        }
    }

  free (cwd);
  return 0;
}
