/* Open a stream to a file.
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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

/* If the user's config.h happens to include <stdio.h>, let it include only
   the system's <stdio.h> here, so that orig_fopen doesn't recurse to
   rpl_fopen.  */
#define _GL_ALREADY_INCLUDING_STDIO_H
#include <config.h>

/* Get the original definition of fopen.  It might be defined as a macro.  */
#include <stdio.h>
#undef _GL_ALREADY_INCLUDING_STDIO_H

static FILE *
orig_fopen (const char *filename, const char *mode)
{
  return fopen (filename, mode);
}

/* Specification.  */
/* Write "stdio.h" here, not <stdio.h>, otherwise OSF/1 5.1 DTK cc eliminates
   this include because of the preliminary #include <stdio.h> above.  */
#include "stdio.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

FILE *
rpl_fopen (const char *filename, const char *mode)
{
  int open_direction;
  int open_flags;
#if GNULIB_FOPEN_GNU
  bool open_flags_gnu;
# define BUF_SIZE 80
  char fdopen_mode_buf[BUF_SIZE + 1];
#endif

#if defined _WIN32 && ! defined __CYGWIN__
  if (strcmp (filename, "/dev/null") == 0)
    filename = "NUL";
#endif

  /* Parse the mode.  */
  open_direction = 0;
  open_flags = 0;
#if GNULIB_FOPEN_GNU
  open_flags_gnu = false;
#endif
  {
    const char *p = mode;
#if GNULIB_FOPEN_GNU
    char *q = fdopen_mode_buf;
#endif

    for (; *p != '\0'; p++)
      {
        switch (*p)
          {
          case 'r':
            open_direction = O_RDONLY;
#if GNULIB_FOPEN_GNU
            if (q < fdopen_mode_buf + BUF_SIZE)
              *q++ = *p;
#endif
            continue;
          case 'w':
            open_direction = O_WRONLY;
            open_flags |= O_CREAT | O_TRUNC;
#if GNULIB_FOPEN_GNU
            if (q < fdopen_mode_buf + BUF_SIZE)
              *q++ = *p;
#endif
            continue;
          case 'a':
            open_direction = O_WRONLY;
            open_flags |= O_CREAT | O_APPEND;
#if GNULIB_FOPEN_GNU
            if (q < fdopen_mode_buf + BUF_SIZE)
              *q++ = *p;
#endif
            continue;
          case 'b':
            /* While it is non-standard, O_BINARY is guaranteed by
               gnulib <fcntl.h>.  We can also assume that orig_fopen
               supports the 'b' flag.  */
            open_flags |= O_BINARY;
#if GNULIB_FOPEN_GNU
            if (q < fdopen_mode_buf + BUF_SIZE)
              *q++ = *p;
#endif
            continue;
          case '+':
            open_direction = O_RDWR;
#if GNULIB_FOPEN_GNU
            if (q < fdopen_mode_buf + BUF_SIZE)
              *q++ = *p;
#endif
            continue;
#if GNULIB_FOPEN_GNU
          case 'x':
            open_flags |= O_EXCL;
            open_flags_gnu = true;
            continue;
          case 'e':
            open_flags |= O_CLOEXEC;
            open_flags_gnu = true;
            continue;
#endif
          default:
            break;
          }
#if GNULIB_FOPEN_GNU
        /* The rest of the mode string can be a platform-dependent extension.
           Copy it unmodified.  */
        {
          size_t len = strlen (p);
          if (len > fdopen_mode_buf + BUF_SIZE - q)
            len = fdopen_mode_buf + BUF_SIZE - q;
          memcpy (q, p, len);
          q += len;
        }
#endif
        break;
      }
#if GNULIB_FOPEN_GNU
    *q = '\0';
#endif
  }

#if FOPEN_TRAILING_SLASH_BUG
  /* Fail if the mode requires write access and the filename ends in a slash,
     as POSIX says such a filename must name a directory
     <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_13>:
       "A pathname that contains at least one non-<slash> character and that
        ends with one or more trailing <slash> characters shall not be resolved
        successfully unless the last pathname component before the trailing
        <slash> characters names an existing directory"
     If the named file already exists as a directory, then if a mode that
     requires write access is specified, fopen() must fail because POSIX
     <https://pubs.opengroup.org/onlinepubs/9699919799/functions/fopen.html>
     says that it fails with errno = EISDIR in this case.
     If the named file does not exist or does not name a directory, then
     fopen() must fail since the file does not contain a '.' directory.  */
  {
    size_t len = strlen (filename);
    if (len > 0 && filename[len - 1] == '/')
      {
        int fd;
        struct stat statbuf;
        FILE *fp;

        if (open_direction != O_RDONLY)
          {
            errno = EISDIR;
            return NULL;
          }

        fd = open (filename, open_direction | open_flags,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd < 0)
          return NULL;

        if (fstat (fd, &statbuf) >= 0 && !S_ISDIR (statbuf.st_mode))
          {
            close (fd);
            errno = ENOTDIR;
            return NULL;
          }

# if GNULIB_FOPEN_GNU
        fp = fdopen (fd, fdopen_mode_buf);
# else
        fp = fdopen (fd, mode);
# endif
        if (fp == NULL)
          {
            int saved_errno = errno;
            close (fd);
            errno = saved_errno;
          }
        return fp;
      }
  }
#endif

#if GNULIB_FOPEN_GNU
  if (open_flags_gnu)
    {
      int fd;
      FILE *fp;

      fd = open (filename, open_direction | open_flags,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
      if (fd < 0)
        return NULL;

      fp = fdopen (fd, fdopen_mode_buf);
      if (fp == NULL)
        {
          int saved_errno = errno;
          close (fd);
          errno = saved_errno;
        }
      return fp;
    }
#endif

  /* open_direction is sometimes used, sometimes unused.
     Silence gcc's warning about this situation.  */
  (void) open_direction;

  return orig_fopen (filename, mode);
}
