/* Tests of unlink.
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

/* This file is designed to test both unlink(n) and
   unlinkat(AT_FDCWD,n,0).  FUNC is the function to test.  Assumes
   that BASE and ASSERT are already defined, and that appropriate
   headers are already included.  If PRINT, then warn before returning
   status 77 when symlinks are unsupported.  */

static int
test_unlink_func (int (*func) (char const *name), bool print)
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
  /* Resulting errno after directories is rather varied across
     implementations (EPERM, EINVAL, EACCES, EBUSY, EISDIR, ENOTSUP);
     however, we must be careful to not attempt unlink on a directory
     unless we know it must fail.  */
  if (cannot_unlink_dir ())
    {
      ASSERT (func (".") == -1);
      ASSERT (func ("..") == -1);
      ASSERT (func ("/") == -1);
      ASSERT (func (BASE "dir") == -1);
      ASSERT (mkdir (BASE "dir1", 0700) == 0);
      ASSERT (func (BASE "dir1") == -1);
      ASSERT (rmdir (BASE "dir1") == 0);
    }
  errno = 0;
  ASSERT (func (BASE "dir/file/") == -1);
  ASSERT (errno == ENOTDIR);

  /* Test symlink behavior.  Specifying trailing slash will attempt
     unlink of a directory, so only attempt it if we know it must
     fail.  */
  if (symlink (BASE "dir", BASE "link") != 0)
    {
      ASSERT (func (BASE "dir/file") == 0);
      ASSERT (rmdir (BASE "dir") == 0);
      if (print)
        fputs ("skipping test: symlinks not supported on this file system\n",
               stderr);
      return 77;
    }
  if (cannot_unlink_dir ())
    ASSERT (func (BASE "link/") == -1);
  ASSERT (func (BASE "link") == 0);
  ASSERT (symlink (BASE "dir/file", BASE "link") == 0);
  errno = 0;
  ASSERT (func (BASE "link/") == -1);
  ASSERT (errno == ENOTDIR);
  /* Order here proves unlink of a symlink does not follow through to
     the file.  */
  ASSERT (func (BASE "link") == 0);
  ASSERT (func (BASE "dir/file") == 0);
  ASSERT (rmdir (BASE "dir") == 0);

  return 0;
}
