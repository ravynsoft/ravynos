/* Open a stream to a file.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

/* If the user's config.h happens to include <stdio.h>, let it include only
   the system's <stdio.h> here, so that orig_freopen doesn't recurse to
   rpl_freopen.  */
#define _GL_ALREADY_INCLUDING_STDIO_H
#include <config.h>

/* Get the original definition of freopen.  It might be defined as a macro.  */
#include <stdio.h>
#undef _GL_ALREADY_INCLUDING_STDIO_H

#include <errno.h>

static FILE *
orig_freopen (const char *filename, const char *mode, FILE *stream)
{
  return freopen (filename, mode, stream);
}

/* Specification.  */
/* Write "stdio.h" here, not <stdio.h>, otherwise OSF/1 5.1 DTK cc eliminates
   this include because of the preliminary #include <stdio.h> above.  */
#include "stdio.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

FILE *
rpl_freopen (const char *filename, const char *mode, FILE *stream)
{
  FILE *result;
#if defined _WIN32 && ! defined __CYGWIN__
  char const *null_device = "NUL";
  if (filename && strcmp (filename, "/dev/null") == 0)
    filename = null_device;
#else
  char const *null_device = "/dev/null";
#endif

#ifdef __KLIBC__
  errno = 0;
#endif

  result = orig_freopen (filename, mode, stream);

  if (!result)
    {
#ifdef __KLIBC__
      /* On OS/2 kLIBC, freopen returns NULL even if it is successful
         if filename is NULL. */
      if (!filename && !errno)
        result = stream;
#endif
    }
  else if (filename)
    {
      int fd = fileno (result);
      if (dup2 (fd, fd) < 0 && errno == EBADF)
        {
          int nullfd = open (null_device, O_RDONLY | O_CLOEXEC);
          int err = 0;
          if (nullfd != fd)
            {
              if (dup2 (nullfd, fd) < 0)
                err = 1;
              close (nullfd);
            }
          if (!err)
            result = orig_freopen (filename, mode, result);
        }
    }

  return result;
}
