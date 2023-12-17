/* Emulate fsync on platforms that lack it, primarily Windows and
   cross-compilers like MinGW.

   This is derived from sqlite3 sources.
   https://www.sqlite.org/src/finfo?name=src/os_win.c
   https://www.sqlite.org/copyright.html

   Written by Richard W.M. Jones <rjones.at.redhat.com>

   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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
#include <unistd.h>

#if defined _WIN32 && ! defined __CYGWIN__

/* FlushFileBuffers */
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

# include <errno.h>

/* Get _get_osfhandle.  */
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif

int
fsync (int fd)
{
  HANDLE h = (HANDLE) _get_osfhandle (fd);
  DWORD err;

  if (h == INVALID_HANDLE_VALUE)
    {
      errno = EBADF;
      return -1;
    }

  if (!FlushFileBuffers (h))
    {
      /* Translate some Windows errors into rough approximations of Unix
       * errors.  MSDN is useless as usual - in this case it doesn't
       * document the full range of errors.
       */
      err = GetLastError ();
      switch (err)
        {
        case ERROR_ACCESS_DENIED:
          /* For a read-only handle, fsync should succeed, even though we have
             no way to sync the access-time changes.  */
          return 0;

          /* eg. Trying to fsync a tty. */
        case ERROR_INVALID_HANDLE:
          errno = EINVAL;
          break;

        default:
          errno = EIO;
        }
      return -1;
    }

  return 0;
}

#else /* !Windows */

# error "This platform lacks fsync function, and Gnulib doesn't provide a replacement. This is a bug in Gnulib."

#endif /* !Windows */
