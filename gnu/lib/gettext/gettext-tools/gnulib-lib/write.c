/* POSIX compatible write() function.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2008.

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

/* On native Windows platforms, SIGPIPE does not exist.  When write() is
   called on a pipe with no readers, WriteFile() fails with error
   GetLastError() = ERROR_NO_DATA, and write() in consequence fails with
   error EINVAL.  */

#if defined _WIN32 && ! defined __CYGWIN__

# include <errno.h>
# include <signal.h>
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

# undef write

# if HAVE_MSVC_INVALID_PARAMETER_HANDLER
static ssize_t
write_nothrow (int fd, const void *buf, size_t count)
{
  ssize_t result;

  TRY_MSVC_INVAL
    {
      result = _write (fd, buf, count);
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
#  define write_nothrow _write
# endif

ssize_t
rpl_write (int fd, const void *buf, size_t count)
{
  for (;;)
    {
      ssize_t ret = write_nothrow (fd, buf, count);

      if (ret < 0)
        {
# if GNULIB_NONBLOCKING
          if (errno == ENOSPC)
            {
              HANDLE h = (HANDLE) _get_osfhandle (fd);
              if (GetFileType (h) == FILE_TYPE_PIPE)
                {
                  /* h is a pipe or socket.  */
                  DWORD state;
                  if (GetNamedPipeHandleState (h, &state, NULL, NULL, NULL,
                                               NULL, 0)
                      && (state & PIPE_NOWAIT) != 0)
                    {
                      /* h is a pipe in non-blocking mode.
                         We can get here in four situations:
                           1. When the pipe buffer is full.
                           2. When count <= pipe_buf_size and the number of
                              free bytes in the pipe buffer is < count.
                           3. When count > pipe_buf_size and the number of free
                              bytes in the pipe buffer is > 0, < pipe_buf_size.
                           4. When count > pipe_buf_size and the pipe buffer is
                              entirely empty.
                         The cases 1 and 2 are POSIX compliant.  In cases 3 and
                         4 POSIX specifies that write() must split the request
                         and succeed with a partial write.  We fix case 4.
                         We don't fix case 3 because it is not essential for
                         programs.  */
                      DWORD out_size; /* size of the buffer for outgoing data */
                      DWORD in_size;  /* size of the buffer for incoming data */
                      if (GetNamedPipeInfo (h, NULL, &out_size, &in_size, NULL))
                        {
                          size_t reduced_count = count;
                          /* In theory we need only one of out_size, in_size.
                             But I don't know which of the two.  The description
                             is ambiguous.  */
                          if (out_size != 0 && out_size < reduced_count)
                            reduced_count = out_size;
                          if (in_size != 0 && in_size < reduced_count)
                            reduced_count = in_size;
                          if (reduced_count < count)
                            {
                              /* Attempt to write only the first part.  */
                              count = reduced_count;
                              continue;
                            }
                        }
                      /* Change errno from ENOSPC to EAGAIN.  */
                      errno = EAGAIN;
                    }
                }
            }
          else
# endif
            {
# if GNULIB_SIGPIPE
              if (GetLastError () == ERROR_NO_DATA
                  && GetFileType ((HANDLE) _get_osfhandle (fd))
                     == FILE_TYPE_PIPE)
                {
                  /* Try to raise signal SIGPIPE.  */
                  raise (SIGPIPE);
                  /* If it is currently blocked or ignored, change errno from
                     EINVAL to EPIPE.  */
                  errno = EPIPE;
                }
# endif
            }
        }
      return ret;
    }
}

#endif
