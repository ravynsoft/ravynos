/* Test of opening a file stream.
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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

/* Include <config.h> and a form of <stdio.h> first.  */

#include <errno.h>
#include <unistd.h>

#include "macros.h"

/* Tell GCC not to warn about the specific edge cases tested here.  */
#if __GNUC__ >= 10
# pragma GCC diagnostic ignored "-Wanalyzer-file-leak"
#endif

/* Test fopen.  Assumes BASE is defined.  */

static int
test_fopen (void)
{
  FILE *f;
  /* Remove anything from prior partial run.  */
  unlink (BASE "file");

  /* Read requires existing file.  */
  errno = 0;
  ASSERT (fopen (BASE "file", "r") == NULL);
  ASSERT (errno == ENOENT);

  /* Write can create a file.  */
  f = fopen (BASE "file", "w");
  ASSERT (f);
  ASSERT (fclose (f) == 0);

  /* Trailing slash is invalid on non-directory.  */
  errno = 0;
  ASSERT (fopen (BASE "file/", "r") == NULL);
  ASSERT (errno == ENOTDIR || errno == EISDIR || errno == EINVAL);

  errno = 0;
  ASSERT (fopen (BASE "file/", "r+") == NULL);
  ASSERT (errno == ENOTDIR || errno == EISDIR || errno == EINVAL);

  /* Cannot create a directory.  */
  errno = 0;
  ASSERT (fopen ("nonexist.ent/", "w") == NULL);
  ASSERT (errno == ENOTDIR || errno == EISDIR || errno == ENOENT
          || errno == EINVAL);

  /* Directories cannot be opened for writing.  */
  errno = 0;
  ASSERT (fopen (".", "w") == NULL);
  ASSERT (errno == EISDIR || errno == EINVAL || errno == EACCES);

  errno = 0;
  ASSERT (fopen ("./", "w") == NULL);
  ASSERT (errno == EISDIR || errno == EINVAL || errno == EACCES);

  errno = 0;
  ASSERT (fopen (".", "r+") == NULL);
  ASSERT (errno == EISDIR || errno == EINVAL || errno == EACCES);

  errno = 0;
  ASSERT (fopen ("./", "r+") == NULL);
  ASSERT (errno == EISDIR || errno == EINVAL || errno == EACCES);

  /* /dev/null must exist, and be writable.  */
  f = fopen ("/dev/null", "r");
  ASSERT (f);
  ASSERT (fclose (f) == 0);
  f = fopen ("/dev/null", "w");
  ASSERT (f);
  ASSERT (fclose (f) == 0);

  /* Cleanup.  */
  ASSERT (unlink (BASE "file") == 0);

  return 0;
}
