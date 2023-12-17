/* Tests of areadlink and friends.
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

/* This file is designed to test areadlink(a),
   areadlink_with_size(a,b), and areadlinkat(AT_FDCWD,a).  FUNC is the
   function to test; a length is always supplied, but may be ignored.
   Assumes that BASE and ASSERT are already defined, and that
   appropriate headers are already included.  If PRINT, warn before
   skipping symlink tests with status 77.  */

static int
test_areadlink (char * (*func) (char const *, size_t), bool print)
{
  /* Sanity checks of failures.  Mingw lacks symlink, but areadlink can
     still distinguish between various errors.  */
  errno = 0;
  ASSERT (func ("no_such", 1) == NULL);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func ("no_such/", 1) == NULL);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func ("", 1) == NULL);
  ASSERT (errno == ENOENT || errno == EINVAL);
  errno = 0;
  ASSERT (func (".", 1) == NULL);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (func ("./", 1) == NULL);
  ASSERT (errno == EINVAL);
  ASSERT (close (creat (BASE "file", 0600)) == 0);
  errno = 0;
  ASSERT (func (BASE "file", 1) == NULL);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (func (BASE "file/", 1) == NULL);
  ASSERT (errno == ENOTDIR || errno == EINVAL); /* AIX yields EINVAL */
  ASSERT (unlink (BASE "file") == 0);

  /* Now test actual symlinks.  */
  if (symlink (BASE "dir", BASE "link"))
    {
      if (print)
        fputs ("skipping test: symlinks not supported on this file system\n",
               stderr);
      return 77;
    }
  ASSERT (mkdir (BASE "dir", 0700) == 0);
  errno = 0;
  ASSERT (func (BASE "link/", 1) == NULL);
  ASSERT (errno == EINVAL);
  {
    /* Too small a guess is okay.  */
    char *buf = func (BASE "link", 1);
    ASSERT (buf);
    ASSERT (strcmp (buf, BASE "dir") == 0);
    free (buf);
    /* Too large a guess is okay.  */
    buf = func (BASE "link", 10000000);
    ASSERT (buf);
    ASSERT (strcmp (buf, BASE "dir") == 0);
    free (buf);
  }
  ASSERT (rmdir (BASE "dir") == 0);
  ASSERT (unlink (BASE "link") == 0);

  return 0;
}
