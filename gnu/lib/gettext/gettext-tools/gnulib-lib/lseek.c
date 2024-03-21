/* An lseek() function that detects pipes.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

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
#include <unistd.h>

#if defined _WIN32 && ! defined __CYGWIN__
/* Windows platforms.  */
/* Get GetFileType.  */
# include <windows.h>
/* Get _get_osfhandle.  */
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif
#else
# include <sys/stat.h>
#endif
#include <errno.h>

#undef lseek

off_t
rpl_lseek (int fd, off_t offset, int whence)
{
#if defined _WIN32 && ! defined __CYGWIN__
  /* mingw lseek mistakenly succeeds on pipes, sockets, and terminals.  */
  HANDLE h = (HANDLE) _get_osfhandle (fd);
  if (h == INVALID_HANDLE_VALUE)
    {
      errno = EBADF;
      return -1;
    }
  if (GetFileType (h) != FILE_TYPE_DISK)
    {
      errno = ESPIPE;
      return -1;
    }
#elif defined __APPLE__ && defined __MACH__ && defined SEEK_DATA
  if (whence == SEEK_DATA)
    {
      /* If OFFSET points to data, macOS lseek+SEEK_DATA returns the
         start S of the first data region that begins *after* OFFSET,
         where the region from OFFSET to S consists of possibly-empty
         data followed by a possibly-empty hole.  To work around this
         portability glitch, check whether OFFSET is within data by
         using lseek+SEEK_HOLE, and if so return to OFFSET by using
         lseek+SEEK_SET.  Also, contrary to the macOS documentation,
         lseek+SEEK_HOLE can fail with ENXIO if there are no holes on
         or after OFFSET.  What a mess!  */
      off_t next_hole = lseek (fd, offset, SEEK_HOLE);
      if (next_hole < 0)
        return errno == ENXIO ? offset : next_hole;
      if (next_hole != offset)
        whence = SEEK_SET;
    }
#else
  /* BeOS lseek mistakenly succeeds on pipes...  */
  struct stat statbuf;
  if (fstat (fd, &statbuf) < 0)
    return -1;
  if (!S_ISREG (statbuf.st_mode))
    {
      errno = ESPIPE;
      return -1;
    }
#endif
#if _GL_WINDOWS_64_BIT_OFF_T || (defined __MINGW32__ && defined _FILE_OFFSET_BITS && (_FILE_OFFSET_BITS == 64))
  return _lseeki64 (fd, offset, whence);
#else
  return lseek (fd, offset, whence);
#endif
}
