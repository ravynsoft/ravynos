/* Test of lstat() function.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Simon Josefsson, 2008; and Eric Blake, 2009.  */

/* This file is designed to test both lstat(n,buf) and
   fstatat(AT_FDCWD,n,buf,AT_SYMLINK_NOFOLLOW).  FUNC is the function
   to test.  Assumes that BASE and ASSERT are already defined, and
   that appropriate headers are already included.  If PRINT, warn
   before skipping symlink tests with status 77.  */

static int
test_lstat_func (int (*func) (char const *, struct stat *), bool print)
{
  struct stat st1;
  struct stat st2;

  /* Test for common directories.  */
  ASSERT (func (".", &st1) == 0);
  ASSERT (func ("./", &st2) == 0);
#if !(defined _WIN32 && !defined __CYGWIN__ && !_GL_WINDOWS_STAT_INODES)
  ASSERT (psame_inode (&st1, &st2));
#endif
  ASSERT (S_ISDIR (st1.st_mode));
  ASSERT (S_ISDIR (st2.st_mode));
  ASSERT (func ("/", &st1) == 0);
  ASSERT (func ("///", &st2) == 0);
#if !(defined _WIN32 && !defined __CYGWIN__ && !_GL_WINDOWS_STAT_INODES)
  ASSERT (psame_inode (&st1, &st2));
#endif
  ASSERT (S_ISDIR (st1.st_mode));
  ASSERT (S_ISDIR (st2.st_mode));
  ASSERT (func ("..", &st1) == 0);
  ASSERT (S_ISDIR (st1.st_mode));

  /* Test for error conditions.  */
  errno = 0;
  ASSERT (func ("", &st1) == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func ("nosuch", &st1) == -1);
  ASSERT (errno == ENOENT);
  errno = 0;
  ASSERT (func ("nosuch/", &st1) == -1);
  ASSERT (errno == ENOENT);

  ASSERT (close (creat (BASE "file", 0600)) == 0);
  ASSERT (func (BASE "file", &st1) == 0);
  ASSERT (S_ISREG (st1.st_mode));
  errno = 0;
  ASSERT (func (BASE "file/", &st1) == -1);
  ASSERT (errno == ENOTDIR);

  /* Now for some symlink tests, where supported.  We set up:
     link1 -> directory
     link2 -> file
     link3 -> dangling
     link4 -> loop
     then test behavior both with and without trailing slash.
  */
  if (symlink (".", BASE "link1") != 0)
    {
      ASSERT (unlink (BASE "file") == 0);
      if (print)
        fputs ("skipping test: symlinks not supported on this file system\n",
               stderr);
      return 77;
    }
  ASSERT (symlink (BASE "file", BASE "link2") == 0);
  ASSERT (symlink (BASE "nosuch", BASE "link3") == 0);
  ASSERT (symlink (BASE "link4", BASE "link4") == 0);

  ASSERT (func (BASE "link1", &st1) == 0);
  ASSERT (S_ISLNK (st1.st_mode));
  ASSERT (func (BASE "link1/", &st1) == 0);
  ASSERT (stat (BASE "link1", &st2) == 0);
  ASSERT (S_ISDIR (st1.st_mode));
  ASSERT (S_ISDIR (st2.st_mode));
#if !(defined _WIN32 && !defined __CYGWIN__ && !_GL_WINDOWS_STAT_INODES)
  ASSERT (psame_inode (&st1, &st2));
#endif

  ASSERT (func (BASE "link2", &st1) == 0);
  ASSERT (S_ISLNK (st1.st_mode));
  errno = 0;
  ASSERT (func (BASE "link2/", &st1) == -1);
  ASSERT (errno == ENOTDIR);

  ASSERT (func (BASE "link3", &st1) == 0);
  ASSERT (S_ISLNK (st1.st_mode));
  errno = 0;
  ASSERT (func (BASE "link3/", &st1) == -1);
  ASSERT (errno == ENOENT);

  ASSERT (func (BASE "link4", &st1) == 0);
  ASSERT (S_ISLNK (st1.st_mode));
  errno = 0;
  ASSERT (func (BASE "link4/", &st1) == -1);
  ASSERT (errno == ELOOP);

  /* Cleanup.  */
  ASSERT (unlink (BASE "file") == 0);
  ASSERT (unlink (BASE "link1") == 0);
  ASSERT (unlink (BASE "link2") == 0);
  ASSERT (unlink (BASE "link3") == 0);
  ASSERT (unlink (BASE "link4") == 0);

  return 0;
}
