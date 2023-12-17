/* Tests of readlink.
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

/* This file is designed to test both readlink(a,b,c) and
   readlinkat(AT_FDCWD,a,b,c).  FUNC is the function to test.  Assumes
   that BASE and ASSERT are already defined, and that appropriate
   headers are already included.  If PRINT, warn before skipping
   symlink tests with status 77.  */

static int
test_readlink (ssize_t (*func) (char const *, char *, size_t), bool print)
{
  char buf[80];

  /* Sanity checks of failures.  Mingw lacks symlink, but readlink can
     still distinguish between various errors.  */
  memset (buf, 0xff, sizeof buf);
  errno = 0;
  ASSERT (func ("no_such", buf, sizeof buf) == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func ("no_such/", buf, sizeof buf) == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func ("", buf, sizeof buf) == -1);
  ASSERT (errno == ENOENT || errno == EINVAL);
  errno = 0;
  ASSERT (func (".", buf, sizeof buf) == -1);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (func ("./", buf, sizeof buf) == -1);
  ASSERT (errno == EINVAL);
  ASSERT (close (creat (BASE "file", 0600)) == 0);
  errno = 0;
  ASSERT (func (BASE "file", buf, sizeof buf) == -1);
  ASSERT (errno == EINVAL);
  errno = 0;
  ASSERT (func (BASE "file/", buf, sizeof buf) == -1);
  ASSERT (errno == ENOTDIR || errno == EINVAL); /* AIX yields EINVAL */

  /* Now test actual symlinks.  */
  if (symlink (BASE "dir", BASE "link"))
    {
      ASSERT (unlink (BASE "file") == 0);
      if (print)
        fputs ("skipping test: symlinks not supported on this file system\n",
               stderr);
      return 77;
    }
  ASSERT (mkdir (BASE "dir", 0700) == 0);
  errno = 0;
  ASSERT (func (BASE "link/", buf, sizeof buf) == -1);
  ASSERT (errno == EINVAL);
  ASSERT (symlink (BASE "link", BASE "link2") == 0);
  errno = 0;
  ASSERT (func (BASE "link2/", buf, sizeof buf) == -1);
  ASSERT (errno == EINVAL);
  ASSERT (unlink (BASE "link2") == 0);
  ASSERT (symlink (BASE "file", BASE "link2") == 0);
  errno = 0;
  ASSERT (func (BASE "link2/", buf, sizeof buf) == -1);
  ASSERT (errno == ENOTDIR || errno == EINVAL); /* AIX yields EINVAL */
  ASSERT (unlink (BASE "file") == 0);
  ASSERT (unlink (BASE "link2") == 0);
  {
    /* Up till now, no readlink has been successful, so buf should be
       unchanged.  */
    int i;
    for (i = 0; i < sizeof buf; i++)
      ASSERT (buf[i] == (char) 0xff);
  }
  {
    size_t len = strlen (BASE "dir");
    /* When passing too small of a buffer, expect the truncated
       length, or an ERANGE failure.  However, a size of 0 is not
       portable enough to test.  */
    ssize_t result;
    errno = 0;
    result = readlink (BASE "link", buf, 1);
    if (result == -1)
      {
        ASSERT (errno == ERANGE);
        ASSERT (buf[0] == (char) 0xff);
      }
    else
      {
        ASSERT (result == 1);
        ASSERT (buf[0] == BASE[0]);
      }
    ASSERT (buf[1] == (char) 0xff);
    ASSERT (func (BASE "link", buf, len) == len);
    ASSERT (strncmp (buf, BASE "dir", len) == 0);
    ASSERT (buf[len] == (char) 0xff);
    ASSERT (func (BASE "link", buf, sizeof buf) == len);
    ASSERT (strncmp (buf, BASE "dir", len) == 0);
    /* POSIX says rest of buf is unspecified; but in practice, it is
       either left alone, or NUL-terminated.  */
    ASSERT (buf[len] == '\0' || buf[len] == (char) 0xff);
  }
  ASSERT (rmdir (BASE "dir") == 0);
  ASSERT (unlink (BASE "link") == 0);

  return 0;
}
