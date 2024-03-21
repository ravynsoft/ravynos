/* Restart reading the entries of a directory from the beginning.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
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

#include <errno.h>

#if GNULIB_defined_DIR
# include "dirent-private.h"
#endif

/* Don't assume that UNICODE is not defined.  */
#undef FindFirstFile
#define FindFirstFile FindFirstFileA

void
rewinddir (DIR *dirp)
#undef rewinddir
{
#if HAVE_DIRENT_H                       /* equivalent to HAVE_REWINDDIR */
  rewinddir (dirp->real_dirp);
#else
  /* Like in closedir().  */
  if (dirp->current != INVALID_HANDLE_VALUE)
    FindClose (dirp->current);

  /* Like in opendir().  */
  dirp->status = -1;
  dirp->current = FindFirstFile (dirp->dir_name_mask, &dirp->entry);
  if (dirp->current == INVALID_HANDLE_VALUE)
    {
      switch (GetLastError ())
        {
        case ERROR_FILE_NOT_FOUND:
          dirp->status = -2;
          break;
        default:
          /* Save the error code for the next readdir() call.  */
          dirp->status = ENOENT;
          break;
        }
    }
#endif
}
