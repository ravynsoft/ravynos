/* Test of mkdir() function.
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

/* This file is designed to test both mkdir(a,b) and
   mkdirat(AT_FDCWD,a,b).  FUNC is the function to test.  Assumes that
   BASE and ASSERT are already defined, and that appropriate headers
   are already included.  If PRINT, warn before skipping tests with
   status 77 when symlinks are unsupported.  */

static int
test_mkdir (int (*func) (char const *, mode_t), bool print)
{
  /* Test basic error handling.  */
  ASSERT (close (creat (BASE "file", 0600)) == 0);
  errno = 0;
  ASSERT (func (BASE "file", 0700) == -1);
  ASSERT (errno == EEXIST);
  errno = 0;
  ASSERT (func (BASE "file/", 0700) == -1);
  ASSERT (errno == ENOTDIR || errno == EEXIST);
  errno = 0;
  ASSERT (func (BASE "file/dir", 0700) == -1);
  ASSERT (errno == ENOTDIR || errno == ENOENT || errno == EOPNOTSUPP);
  ASSERT (unlink (BASE "file") == 0);
  errno = 0;
  ASSERT (func ("", 0700) == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func (BASE "dir/sub", 0700) == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func (BASE "dir/.", 0700) == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func (BASE "dir/.//", 0700) == -1);
  ASSERT (errno == ENOENT);

  /* Test trailing slash handling.  */
  ASSERT (func (BASE "dir", 0700) == 0);
  errno = 0;
  ASSERT (func (BASE "dir", 0700) == -1);
  ASSERT (errno == EEXIST);
  ASSERT (rmdir (BASE "dir") == 0);
  ASSERT (func (BASE "dir/", 0700) == 0);
  errno = 0;
  ASSERT (func (BASE "dir/", 0700) == -1);
  ASSERT (errno == EEXIST);
  ASSERT (rmdir (BASE "dir") == 0);

  /* Test symlink behavior.  POSIX requires the creation of
     directories through a dangling symlink with trailing slash, but
     GNU does not yet implement that, so we support either behavior
     for now.  */
  if (symlink (BASE "dir", BASE "link"))
    {
      if (print)
        fputs ("skipping test: symlinks not supported on this file system\n",
               stderr);
      return 77;
    }
  errno = 0;
  ASSERT (func (BASE "link", 0700) == -1);
  ASSERT (errno == EEXIST);
  {
    int result;
    errno = 0;
    result = func (BASE "link/", 0700);
    if (!result)
      ASSERT (rmdir (BASE "dir") == 0);
    else
      {
        ASSERT (result == -1);
        ASSERT (errno == EEXIST);
        errno = 0;
        ASSERT (rmdir (BASE "dir") == -1);
        ASSERT (errno == ENOENT);
      }
  }
  errno = 0;
  ASSERT (func (BASE "link/.", 0700) == -1);
  ASSERT (errno == ENOENT);
  ASSERT (unlink (BASE "link") == 0);

  return 0;
}
