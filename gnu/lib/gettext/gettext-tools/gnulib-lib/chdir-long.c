/* provide a chdir function that tries not to fail due to ENAMETOOLONG
   Copyright (C) 2004-2023 Free Software Foundation, Inc.

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

/* written by Jim Meyering */

#include <config.h>

#include "chdir-long.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "assure.h"

#ifndef PATH_MAX
# error "compile this file only if your system defines PATH_MAX"
#endif

/* The results of openat() in this file are not leaked to any
   single-threaded code that could use stdio.
   FIXME - if the kernel ever adds support for multi-thread safety for
   avoiding standard fds, then we should use openat_safer.  */

struct cd_buf
{
  int fd;
};

static void
cdb_init (struct cd_buf *cdb)
{
  cdb->fd = AT_FDCWD;
}

static int
cdb_fchdir (struct cd_buf const *cdb)
{
  return fchdir (cdb->fd);
}

static void
cdb_free (struct cd_buf const *cdb)
{
  if (0 <= cdb->fd)
    {
      bool close_fail = close (cdb->fd);
      assure (! close_fail);
    }
}

/* Given a file descriptor of an open directory (or AT_FDCWD), CDB->fd,
   try to open the CDB->fd-relative directory, DIR.  If the open succeeds,
   update CDB->fd with the resulting descriptor, close the incoming file
   descriptor, and return zero.  Upon failure, return -1 and set errno.  */
static int
cdb_advance_fd (struct cd_buf *cdb, char const *dir)
{
  int new_fd = openat (cdb->fd, dir,
                       O_SEARCH | O_DIRECTORY | O_NOCTTY | O_NONBLOCK);
  if (new_fd < 0)
    return -1;

  cdb_free (cdb);
  cdb->fd = new_fd;

  return 0;
}

/* Return a pointer to the first non-slash in S.  */
static char * _GL_ATTRIBUTE_PURE
find_non_slash (char const *s)
{
  size_t n_slash = strspn (s, "/");
  return (char *) s + n_slash;
}

/* This is a function much like chdir, but without the PATH_MAX limitation
   on the length of the directory name.  A significant difference is that
   it must be able to modify (albeit only temporarily) the directory
   name.  It handles an arbitrarily long directory name by operating
   on manageable portions of the name.  On systems without the openat
   syscall, this means changing the working directory to more and more
   "distant" points along the long directory name and then restoring
   the working directory.  If any of those attempts to save or restore
   the working directory fails, this function exits nonzero.

   Note that this function may still fail with errno == ENAMETOOLONG, but
   only if the specified directory name contains a component that is long
   enough to provoke such a failure all by itself (e.g. if the component
   has length PATH_MAX or greater on systems that define PATH_MAX).  */

int
chdir_long (char *dir)
{
  int e = chdir (dir);
  if (e == 0 || errno != ENAMETOOLONG)
    return e;

  {
    size_t len = strlen (dir);
    char *dir_end = dir + len;
    struct cd_buf cdb;
    size_t n_leading_slash;

    cdb_init (&cdb);

    /* If DIR is the empty string, then the chdir above
       must have failed and set errno to ENOENT.  */
    assure (0 < len);
    assure (PATH_MAX <= len);

    /* Count leading slashes.  */
    n_leading_slash = strspn (dir, "/");

    /* Handle any leading slashes as well as any name that matches
       the regular expression, m!^//hostname[/]*! .  Handling this
       prefix separately usually results in a single additional
       cdb_advance_fd call, but it's worthwhile, since it makes the
       code in the following loop cleaner.  */
    if (n_leading_slash == 2)
      {
        int err;
        /* Find next slash.
           We already know that dir[2] is neither a slash nor '\0'.  */
        char *slash = memchr (dir + 3, '/', dir_end - (dir + 3));
        if (slash == NULL)
          {
            errno = ENAMETOOLONG;
            return -1;
          }
        *slash = '\0';
        err = cdb_advance_fd (&cdb, dir);
        *slash = '/';
        if (err != 0)
          goto Fail;
        dir = find_non_slash (slash + 1);
      }
    else if (n_leading_slash)
      {
        if (cdb_advance_fd (&cdb, "/") != 0)
          goto Fail;
        dir += n_leading_slash;
      }

    assure (*dir != '/');
    assure (dir <= dir_end);

    while (PATH_MAX <= dir_end - dir)
      {
        int err;
        /* Find a slash that is PATH_MAX or fewer bytes away from dir.
           I.e. see if there is a slash that will give us a name of
           length PATH_MAX-1 or less.  */
        char *slash = memrchr (dir, '/', PATH_MAX);
        if (slash == NULL)
          {
            errno = ENAMETOOLONG;
            return -1;
          }

        *slash = '\0';
        assure (slash - dir < PATH_MAX);
        err = cdb_advance_fd (&cdb, dir);
        *slash = '/';
        if (err != 0)
          goto Fail;

        dir = find_non_slash (slash + 1);
      }

    if (dir < dir_end)
      {
        if (cdb_advance_fd (&cdb, dir) != 0)
          goto Fail;
      }

    if (cdb_fchdir (&cdb) != 0)
      goto Fail;

    cdb_free (&cdb);
    return 0;

   Fail:
    {
      int saved_errno = errno;
      cdb_free (&cdb);
      errno = saved_errno;
      return -1;
    }
  }
}

#if TEST_CHDIR

# include "closeout.h"
# include "error.h"

int
main (int argc, char *argv[])
{
  char *line = NULL;
  size_t n = 0;
  int len;

  atexit (close_stdout);

  len = getline (&line, &n, stdin);
  if (len < 0)
    {
      int saved_errno = errno;
      if (feof (stdin))
        exit (0);

      error (EXIT_FAILURE, saved_errno,
             "reading standard input");
    }
  else if (len == 0)
    exit (0);

  if (line[len-1] == '\n')
    line[len-1] = '\0';

  if (chdir_long (line) != 0)
    error (EXIT_FAILURE, errno,
           "chdir_long failed: %s", line);

  if (argc <= 1)
    {
      /* Using 'pwd' here makes sense only if it is a robust implementation,
         like the one in coreutils after the 2004-04-19 changes.  */
      char const *cmd = "pwd";
      execlp (cmd, (char *) NULL);
      error (EXIT_FAILURE, errno, "%s", cmd);
    }

  fclose (stdin);
  fclose (stderr);

  exit (EXIT_SUCCESS);
}
#endif

/*
Local Variables:
compile-command: "gcc -DTEST_CHDIR=1 -g -O -W -Wall chdir-long.c libcoreutils.a"
End:
*/
