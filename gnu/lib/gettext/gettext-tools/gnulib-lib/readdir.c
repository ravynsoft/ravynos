/* Read the next entry of a directory.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#include <errno.h>
#include <stddef.h>

#if GNULIB_defined_DIR
# include "dirent-private.h"
#endif

/* Don't assume that UNICODE is not defined.  */
#undef FindNextFile
#define FindNextFile FindNextFileA

struct dirent *
readdir (DIR *dirp)
#undef readdir
{
#if HAVE_DIRENT_H                       /* equivalent to HAVE_READDIR */
  return readdir (dirp->real_dirp);
#else
  char type;
  struct dirent *result;

  /* There is no need to add code to produce entries for "." and "..".
     According to the POSIX:2008 section "4.12 Pathname Resolution"
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html>
     "." and ".." are syntactic entities.
     POSIX also says:
       "If entries for dot or dot-dot exist, one entry shall be returned
        for dot and one entry shall be returned for dot-dot; otherwise,
        they shall not be returned."  */

  switch (dirp->status)
    {
    case -2:
      /* End of directory already reached.  */
      return NULL;
    case -1:
      break;
    case 0:
      if (!FindNextFile (dirp->current, &dirp->entry))
        {
          switch (GetLastError ())
            {
            case ERROR_NO_MORE_FILES:
              dirp->status = -2;
              return NULL;
            default:
              errno = EIO;
              return NULL;
            }
        }
      break;
    default:
      errno = dirp->status;
      return NULL;
    }

  dirp->status = 0;

  if (dirp->entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    type = DT_DIR;
  else if (dirp->entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    type = DT_LNK;
  else if ((dirp->entry.dwFileAttributes
            & ~(FILE_ATTRIBUTE_READONLY
                | FILE_ATTRIBUTE_HIDDEN
                | FILE_ATTRIBUTE_SYSTEM
                | FILE_ATTRIBUTE_ARCHIVE
                | FILE_ATTRIBUTE_NORMAL
                | FILE_ATTRIBUTE_TEMPORARY
                | FILE_ATTRIBUTE_SPARSE_FILE
                | FILE_ATTRIBUTE_COMPRESSED
                | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
                | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
    /* Devices like COM1, LPT1, NUL would also have the attributes 0x20 but
       they cannot occur here.  */
    type = DT_REG;
  else
    type = DT_UNKNOWN;

  /* Reuse the memory of dirp->entry for the result.  */
  result =
    (struct dirent *)
    ((char *) dirp->entry.cFileName - offsetof (struct dirent, d_name[0]));
  result->d_type = type;

  return result;
#endif
}
