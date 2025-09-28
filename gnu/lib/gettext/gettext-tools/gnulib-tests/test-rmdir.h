/* Tests of rmdir.
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

/* This file is designed to test both rmdir(n) and
   unlinkat(AT_FDCWD,n,AT_REMOVEDIR).  FUNC is the function to test.
   Assumes that BASE and ASSERT are already defined, and that
   appropriate headers are already included.  If PRINT, then warn
   before returning status 77 when symlinks are unsupported.  */

static int
test_rmdir_func (int (*func) (char const *name), bool print)
{
  /* Setup.  */
  ASSERT (mkdir (BASE "dir", 0700) == 0);
  ASSERT (close (creat (BASE "dir/file", 0600)) == 0);

  /* Basic error conditions.  */
  errno = 0;
  ASSERT (func ("") == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func (BASE "nosuch") == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func (BASE "nosuch/") == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func (".") == -1);
  ASSERT (errno == EINVAL || errno == EBUSY);
  /* Resulting errno after ".." or "/" is too varied to test; it is
     reasonable to see any of EINVAL, EBUSY, EEXIST, ENOTEMPTY,
     EACCES, EPERM.  */
  ASSERT (func ("..") == -1);
  ASSERT (func ("/") == -1);
  ASSERT (func ("///") == -1);
  errno = 0;
  ASSERT (func (BASE "dir/file/") == -1);
  ASSERT (errno == ENOTDIR);

  /* Non-empty directory.  */
  errno = 0;
  ASSERT (func (BASE "dir") == -1);
  ASSERT (errno == EEXIST || errno == ENOTEMPTY);

  /* Non-directory.  */
  errno = 0;
  ASSERT (func (BASE "dir/file") == -1);
  ASSERT (errno == ENOTDIR);

  /* Empty directory.  */
  ASSERT (unlink (BASE "dir/file") == 0);
  errno = 0;
  ASSERT (func (BASE "dir/.//") == -1);
  ASSERT (errno == EINVAL || errno == EBUSY || errno == EEXIST
          || errno == ENOTEMPTY);
  ASSERT (func (BASE "dir") == 0);

  /* Test symlink behavior.  Specifying trailing slash should remove
     referent directory (POSIX), or cause ENOTDIR failure (Linux), but
     not touch symlink.  We prefer the Linux behavior for its
     intuitiveness (especially compared to rmdir("symlink-to-file/")),
     but not enough to penalize POSIX systems with an rpl_rmdir.  */
  if (symlink (BASE "dir", BASE "link") != 0)
    {
      if (print)
        fputs ("skipping test: symlinks not supported on this file system\n",
               stderr);
      return 77;
    }
  ASSERT (mkdir (BASE "dir", 0700) == 0);
  errno = 0;
  if (func (BASE "link/") == 0)
    {
      struct stat st;
      errno = 0;
      ASSERT (stat (BASE "link", &st) == -1);
      ASSERT (errno == ENOENT);
    }
  else
    {
      ASSERT (errno == ENOTDIR);
      ASSERT (func (BASE "dir") == 0);
    }
  ASSERT (unlink (BASE "link") == 0);

  return 0;
}
