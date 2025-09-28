/* Stop reading the entries of a directory.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <dirent.h>

#if REPLACE_FCHDIR
# include <unistd.h>
#endif

#include <stdlib.h>

#if HAVE_CLOSEDIR

/* Override closedir(), to keep track of the open file descriptors.
   Needed because there is a function dirfd().  */

#endif

#if GNULIB_defined_DIR
# include "dirent-private.h"
#endif

int
closedir (DIR *dirp)
#undef closedir
{
#if GNULIB_defined_DIR || REPLACE_FCHDIR
  int fd = dirfd (dirp);
#endif
  int retval;

#if HAVE_DIRENT_H                       /* equivalent to HAVE_CLOSEDIR */

# if GNULIB_defined_DIR
  retval = closedir (dirp->real_dirp);
  if (retval >= 0)
    free (dirp);
# else
  retval = closedir (dirp);
# endif

#else

  if (dirp->current != INVALID_HANDLE_VALUE)
    FindClose (dirp->current);
  free (dirp);

  retval = 0;

#endif

#if GNULIB_defined_DIR
  if (retval >= 0)
    close (fd);
#elif REPLACE_FCHDIR
  if (retval >= 0)
    _gl_unregister_fd (fd);
#endif

  return retval;
}
