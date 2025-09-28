/* Test of creating a file.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

#include <fcntl.h>

#include "signature.h"
SIGNATURE_CHECK (creat, int, (const char *, mode_t));

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "macros.h"

#define BASE "test-creat.t"

int
main (void)
{
  int fd;

  /* Remove anything from prior partial run.  */
  unlink (BASE "file");
  unlink (BASE "e.exe");

  /* Cannot create directory.  */
  errno = 0;
  ASSERT (creat ("nonexist.ent/", 0600) == -1);
  ASSERT (errno == ENOTDIR || errno == EISDIR || errno == ENOENT
          || errno == EINVAL);

  /* Create a regular file.  */
  fd = creat (BASE "file", 0600);
  ASSERT (0 <= fd);
  ASSERT (close (fd) == 0);

  /* Create an executable regular file.  */
  fd = creat (BASE "e.exe", 0700);
  ASSERT (0 <= fd);
  ASSERT (close (fd) == 0);

  /* Cleanup.  */
  ASSERT (unlink (BASE "file") == 0);
  ASSERT (unlink (BASE "e.exe") == 0);

  return 0;
}
