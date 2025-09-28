/* Create a file.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* If the user's config.h happens to include <fcntl.h>, let it include only
   the system's <fcntl.h> here, so that orig_creat doesn't recurse to
   rpl_creat.  */
#define __need_system_fcntl_h
#include <config.h>

/* Get the original definition of creat.  It might be defined as a macro.  */
#include <fcntl.h>
#include <sys/types.h>
#undef __need_system_fcntl_h

static int
orig_creat (const char *filename, mode_t mode)
{
#if defined _WIN32 && !defined __CYGWIN__
  return _creat (filename, mode);
#else
  return creat (filename, mode);
#endif
}

/* Specification.  */
/* Write "fcntl.h" here, not <fcntl.h>, otherwise OSF/1 5.1 DTK cc eliminates
   this include because of the preliminary #include <fcntl.h> above.  */
#include "fcntl.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>

int
creat (const char *filename, mode_t mode)
{
#if OPEN_TRAILING_SLASH_BUG
  /* Fail if the filename ends in a slash,
     as POSIX says such a filename must name a directory
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>:
       "A pathname that contains at least one non-<slash> character and that
        ends with one or more trailing <slash> characters shall not be resolved
        successfully unless the last pathname component before the trailing
        <slash> characters names an existing directory"
     creat() is defined as being equivalent to open() with flags
     O_CREAT | O_TRUNC | O_WRONLY.  Therefore:
     If the named file already exists as a directory, then creat() must fail
     with errno = EISDIR.
     If the named file does not exist or does not name a directory, then
     creat() must fail since creat() cannot create directories.  */
  {
    size_t len = strlen (filename);
    if (len > 0 && filename[len - 1] == '/')
      {
        errno = EISDIR;
        return -1;
      }
  }
#endif

#if defined _WIN32 && !defined __CYGWIN__
  /* Remap the 'x' bits to the 'r' bits.  */
  mode = (mode & ~0111) | ((mode & 0111) << 2);
#endif

  return orig_creat (filename, mode);
}
