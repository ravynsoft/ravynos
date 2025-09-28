/* Tests of fstatat.
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

#include <sys/stat.h>

#include "signature.h"
SIGNATURE_CHECK (fstatat, int, (int, char const *, struct stat *, int));

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "openat.h"
#include "same-inode.h"
#include "ignore-value.h"
#include "macros.h"

#ifndef BASE
# define BASE "test-fstatat.t"
#endif

#include "test-lstat.h"
#include "test-stat.h"

static int dfd = AT_FDCWD;

/* Wrapper around fstatat to test stat behavior.  */
static int
do_stat (char const *name, struct stat *st)
{
#ifdef TEST_STATAT
  return statat (dfd, name, st);
#else
  return fstatat (dfd, name, st, 0);
#endif
}

/* Wrapper around fstatat to test lstat behavior.  */
static int
do_lstat (char const *name, struct stat *st)
{
#ifdef TEST_STATAT
  return lstatat (dfd, name, st);
#else
  return fstatat (dfd, name, st, AT_SYMLINK_NOFOLLOW);
#endif
}

int
main (_GL_UNUSED int argc, _GL_UNUSED char *argv[])
{
  int result;

  /* Remove any leftovers from a previous partial run.  */
  ignore_value (system ("rm -rf " BASE "*"));

  /* Test behaviour for invalid file descriptors.  */
  {
    struct stat statbuf;

    errno = 0;
    ASSERT (fstatat (-1, "foo", &statbuf, 0) == -1);
    ASSERT (errno == EBADF);
  }
  {
    struct stat statbuf;

    close (99);
    errno = 0;
    ASSERT (fstatat (99, "foo", &statbuf, 0) == -1);
    ASSERT (errno == EBADF);
  }

  result = test_stat_func (do_stat, false);
  ASSERT (test_lstat_func (do_lstat, false) == result);
  dfd = open (".", O_RDONLY);
  ASSERT (0 <= dfd);
  ASSERT (test_stat_func (do_stat, false) == result);
  ASSERT (test_lstat_func (do_lstat, false) == result);
  ASSERT (close (dfd) == 0);

  /* FIXME - add additional tests of dfd not at current directory.  */

  if (result == 77)
    fputs ("skipping test: symlinks not supported on this file system\n",
           stderr);
  return result;
}
