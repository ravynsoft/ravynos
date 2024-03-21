/* Create /proc/self/fd-related names for subfiles of open directories.

   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert.  */

#include <config.h>

#include "openat-priv.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __KLIBC__ /* OS/2 */
# include <InnoTekLIBC/backend.h>
#endif
#ifdef __MVS__ /* z/OS */
# include <termios.h>
#endif

#include "intprops.h"

/* Set BUF to the name of the subfile of the directory identified by
   FD, where the subfile is named FILE.  If successful, return BUF if
   the result fits in BUF, dynamically allocated memory otherwise.
   Return NULL (setting errno) on error.  */
char *
openat_proc_name (char buf[OPENAT_BUFFER_SIZE], int fd, char const *file)
{
  char *result = buf;
  int dirlen;

  /* Make sure the caller gets ENOENT when appropriate.  */
  if (!*file)
    {
      buf[0] = '\0';
      return buf;
    }

#if !(defined __KLIBC__ || defined __MVS__)
  /* Generic code for Linux, Solaris, and similar platforms.  */
# define PROC_SELF_FD_FORMAT "/proc/self/fd/%d/"
  {
    enum {
      PROC_SELF_FD_DIR_SIZE_BOUND
        = (sizeof PROC_SELF_FD_FORMAT - (sizeof "%d" - 1)
           + INT_STRLEN_BOUND (int))
    };

    static int proc_status = 0;
    if (! proc_status)
      {
        /* Set PROC_STATUS to a positive value if /proc/self/fd is
           reliable, and a negative value otherwise.  Solaris 10
           /proc/self/fd mishandles "..", and any file name might expand
           to ".." after symbolic link expansion, so avoid /proc/self/fd
           if it mishandles "..".  Solaris 10 has openat, but this
           problem is exhibited on code that built on Solaris 8 and
           running on Solaris 10.  */

        int proc_self_fd =
          open ("/proc/self/fd",
                O_SEARCH | O_DIRECTORY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
        if (proc_self_fd < 0)
          proc_status = -1;
        else
          {
            /* Detect whether /proc/self/fd/%i/../fd exists, where %i is the
               number of a file descriptor open on /proc/self/fd.  On Linux,
               that name resolves to /proc/self/fd, which was opened above.
               However, on Solaris, it may resolve to /proc/self/fd/fd, which
               cannot exist, since all names in /proc/self/fd are numeric.  */
            char dotdot_buf[PROC_SELF_FD_DIR_SIZE_BOUND + sizeof "../fd" - 1];
            sprintf (dotdot_buf, PROC_SELF_FD_FORMAT "../fd", proc_self_fd);
            proc_status = access (dotdot_buf, F_OK) ? -1 : 1;
            close (proc_self_fd);
          }
      }

    if (proc_status < 0)
      return NULL;
    else
      {
        size_t bufsize = PROC_SELF_FD_DIR_SIZE_BOUND + strlen (file);
        if (OPENAT_BUFFER_SIZE < bufsize)
          {
            result = malloc (bufsize);
            if (! result)
              return NULL;
          }

        dirlen = sprintf (result, PROC_SELF_FD_FORMAT, fd);
      }
  }
#else /* (defined __KLIBC__ || defined __MVS__), i.e. OS/2 or z/OS */
  /* OS/2 kLIBC provides a function to retrieve a path from a fd.  */
  {
    size_t bufsize;

# ifdef __KLIBC__
    char dir[_MAX_PATH];
    if (__libc_Back_ioFHToPath (fd, dir, sizeof dir))
      return NULL;
# endif
# ifdef __MVS__
    char dir[_XOPEN_PATH_MAX];
    /* Documentation:
       https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-w-ioctl-w-pioctl-control-devices */
    if (w_ioctl (fd, _IOCC_GPN, sizeof dir, dir) < 0)
      return NULL;
    /* Documentation:
       https://www.ibm.com/docs/en/zos/2.2.0?topic=functions-e2a-l-convert-characters-from-ebcdic-ascii */
    dirlen = __e2a_l (dir, strlen (dir));
    if (dirlen < 0 || dirlen >= sizeof dir)
      return NULL;
    dir[dirlen] = '\0';
# endif

    dirlen = strlen (dir);
    bufsize = dirlen + 1 + strlen (file) + 1; /* 1 for '/', 1 for null */
    if (OPENAT_BUFFER_SIZE < bufsize)
      {
        result = malloc (bufsize);
        if (! result)
          return NULL;
      }

    strcpy (result, dir);
    result[dirlen++] = '/';
  }
#endif

  strcpy (result + dirlen, file);
  return result;
}
