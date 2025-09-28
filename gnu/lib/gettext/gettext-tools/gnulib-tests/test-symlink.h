/* Tests of symlink.
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

/* This file is designed to test both symlink(a,b) and
   symlinkat(a,AT_FDCWD,b).  FUNC is the function to test.  Assumes
   that BASE and ASSERT are already defined, and that appropriate
   headers are already included.  If PRINT, warn before skipping
   symlink tests with status 77.  */

static int
test_symlink (int (*func) (char const *, char const *), bool print)
{
  if (func ("nowhere", BASE "link1"))
    {
      if (print)
        fputs ("skipping test: symlinks not supported on this file system\n",
               stderr);
      return 77;
    }

  /* Some systems allow the creation of 0-length symlinks as a synonym
     for "."; but most reject it.  */
  {
    int status;
    errno = 0;
    status = func ("", BASE "link2");
    if (status == -1)
      ASSERT (errno == ENOENT || errno == EINVAL);
    else
      {
        ASSERT (status == 0);
        ASSERT (unlink (BASE "link2") == 0);
      }
  }

  /* Sanity checks of failures.  */
  errno = 0;
  ASSERT (func ("nowhere", "") == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func ("nowhere", ".") == -1);
  ASSERT (errno == EEXIST || errno == EINVAL);
  errno = 0;
  ASSERT (func ("somewhere", BASE "link1") == -1);
  ASSERT (errno == EEXIST);
  errno = 0;
  ASSERT (func ("nowhere", BASE "link2/") == -1);
  ASSERT (errno == ENOTDIR || errno == ENOENT);
  ASSERT (mkdir (BASE "dir", 0700) == 0);
  errno = 0;
  ASSERT (func ("nowhere", BASE "dir") == -1);
  ASSERT (errno == EEXIST);
  errno = 0;
  ASSERT (func ("nowhere", BASE "dir/") == -1);
  ASSERT (errno == EEXIST || errno == EINVAL
          || errno == ENOENT /* Lustre FS on Linux */);
  ASSERT (close (creat (BASE "file", 0600)) == 0);
  errno = 0;
  ASSERT (func ("nowhere", BASE "file") == -1);
  ASSERT (errno == EEXIST);
  errno = 0;
  ASSERT (func ("nowhere", BASE "file/") == -1);
  ASSERT (errno == EEXIST || errno == ENOTDIR || errno == ENOENT);

  /* Trailing slash must always be rejected.  */
  ASSERT (unlink (BASE "link1") == 0);
  ASSERT (func (BASE "link2", BASE "link1") == 0);
  errno = 0;
  ASSERT (func (BASE "nowhere", BASE "link1/") == -1);
  ASSERT (errno == EEXIST || errno == ENOTDIR || errno == ENOENT);
  errno = 0;
  ASSERT (unlink (BASE "link2") == -1);
  ASSERT (errno == ENOENT);

  /* Cleanup.  */
  ASSERT (rmdir (BASE "dir") == 0);
  ASSERT (unlink (BASE "file") == 0);
  ASSERT (unlink (BASE "link1") == 0);

  return 0;
}
