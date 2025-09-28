/* save-cwd.c -- Save and restore current working directory.

   Copyright (C) 1995, 1997-1998, 2003-2006, 2009-2023 Free Software
   Foundation, Inc.

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

/* Written by Jim Meyering.  */

#include <config.h>

#include "save-cwd.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "chdir-long.h"
#include "unistd--.h"

#if GNULIB_FCNTL_SAFER
# include "fcntl--.h"
#else
# define GNULIB_FCNTL_SAFER 0
#endif

/* Record the location of the current working directory in CWD so that
   the program may change to other directories and later use restore_cwd
   to return to the recorded location.  This function may allocate
   space using malloc (via getcwd) or leave a file descriptor open;
   use free_cwd to perform the necessary free or close.  Upon failure,
   no memory is allocated, any locally opened file descriptors are
   closed;  return non-zero -- in that case, free_cwd need not be
   called, but doing so is ok.  Otherwise, return zero.

   The _raison d'etre_ for this interface is that the working directory
   is sometimes inaccessible, and getcwd is not robust or as efficient.
   So, we prefer to use the open/fchdir approach, but fall back on
   getcwd if necessary.  This module works for most cases with just
   the getcwd-lgpl module, but to be truly robust, use the getcwd module.

   Some systems lack fchdir altogether: e.g., OS/2, pre-2001 Cygwin,
   SCO Xenix.  Also, SunOS 4 and Irix 5.3 provide the function, yet it
   doesn't work for partitions on which auditing is enabled.  If
   you're still using an obsolete system with these problems, please
   send email to the maintainer of this code.  */

int
save_cwd (struct saved_cwd *cwd)
{
  cwd->name = NULL;

  cwd->desc = open (".", O_SEARCH | O_CLOEXEC);
  if (!GNULIB_FCNTL_SAFER)
    cwd->desc = fd_safer_flag (cwd->desc, O_CLOEXEC);
  if (cwd->desc < 0)
    {
      cwd->name = getcwd (NULL, 0);
      return cwd->name ? 0 : -1;
    }

  return 0;
}

/* Change to recorded location, CWD, in directory hierarchy.
   Upon failure, return -1 (errno is set by chdir or fchdir).
   Upon success, return zero.  */

int
restore_cwd (const struct saved_cwd *cwd)
{
  if (0 <= cwd->desc)
    return fchdir (cwd->desc);
  else
    return chdir_long (cwd->name);
}

void
free_cwd (struct saved_cwd *cwd)
{
  if (cwd->desc >= 0)
    close (cwd->desc);
  free (cwd->name);
}
