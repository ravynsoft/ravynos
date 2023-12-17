/* POSIX compatible read() function.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2011.

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

# include <errno.h>
# include <io.h>

# define WIN32_LEAN_AND_MEAN  /* avoid including junk */
# include <windows.h>

# if HAVE_MSVC_INVALID_PARAMETER_HANDLER
#  include "msvc-inval.h"
# endif
# if GNULIB_MSVC_NOTHROW
#  include "msvc-nothrow.h"
# else
#  include <io.h>
# endif

/* Don't assume that UNICODE is not defined.  */
# undef GetNamedPipeHandleState
# define GetNamedPipeHandleState GetNamedPipeHandleStateA

# undef read

# if HAVE_MSVC_INVALID_PARAMETER_HANDLER
static ssize_t
read_nothrow (int fd, void *buf, size_t count)
{
  ssize_t result;

  TRY_MSVC_INVAL
    {
      result = _read (fd, buf, count);
    }
  CATCH_MSVC_INVAL
    {
      result = -1;
      errno = EBADF;
    }
  DONE_MSVC_INVAL;

  return result;
}
# else
#  define read_nothrow _read
# endif

ssize_t
rpl_read (int fd, void *buf, size_t count)
{
  ssize_t ret = read_nothrow (fd, buf, count);

# if GNULIB_NONBLOCKING
  if (ret < 0
      && GetLastError () == ERROR_NO_DATA)
    {
      HANDLE h = (HANDLE) _get_osfhandle (fd);
      if (GetFileType (h) == FILE_TYPE_PIPE)
        {
          /* h is a pipe or socket.  */
          DWORD state;
          if (GetNamedPipeHandleState (h, &state, NULL, NULL, NULL, NULL, 0)
              && (state & PIPE_NOWAIT) != 0)
            /* h is a pipe in non-blocking mode.
               Change errno from EINVAL to EAGAIN.  */
            errno = EAGAIN;
        }
    }
# endif

  return ret;
}

#endif
