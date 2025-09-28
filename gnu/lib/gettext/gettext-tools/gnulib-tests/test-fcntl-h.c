/* Test of <fcntl.h> substitute.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include <fcntl.h>

/* Check that the various O_* macros are defined.  */
int o = (O_DIRECT | O_DIRECTORY | O_DSYNC | O_IGNORE_CTTY | O_NDELAY | O_NOATIME
         | O_NONBLOCK | O_NOCTTY | O_NOFOLLOW | O_NOLINK | O_NOLINKS | O_NOTRANS
         | O_RSYNC | O_SYNC | O_TTY_INIT | O_BINARY | O_TEXT);

/* Check that the various SEEK_* macros are defined.  */
int sk[] = { SEEK_CUR, SEEK_END, SEEK_SET };

/* Check that the FD_* macros are defined.  */
int i = FD_CLOEXEC;

/* Check that the types are all defined.  */
pid_t t1;
off_t t2;
mode_t t3;

int
main (void)
{
  /* Ensure no overlap in SEEK_*. */
  switch (0)
    {
    case SEEK_CUR:
    case SEEK_END:
    case SEEK_SET:
      ;
    }

  /* Ensure no dangerous overlap in non-zero gnulib-defined replacements.  */
  switch (O_RDONLY)
    {
      /* Access modes */
    case O_RDONLY:
    case O_WRONLY:
    case O_RDWR:
#if O_EXEC && O_EXEC != O_RDONLY
    case O_EXEC:
#endif
#if O_SEARCH && O_EXEC != O_SEARCH && O_SEARCH != O_RDONLY
    case O_SEARCH:
#endif
      i = ! (~O_ACCMODE & (O_RDONLY | O_WRONLY | O_RDWR | O_EXEC | O_SEARCH));
      break;

      /* Everyone should have these */
    case O_CREAT:
    case O_EXCL:
    case O_TRUNC:
    case O_APPEND:
      break;

      /* These might be 0 or O_RDONLY, only test non-zero versions.  */
#if O_CLOEXEC
    case O_CLOEXEC:
#endif
#if O_DIRECT
    case O_DIRECT:
#endif
#if O_DIRECTORY
    case O_DIRECTORY:
#endif
#if O_DSYNC
    case O_DSYNC:
#endif
#if O_IGNORE_CTTY
    case O_IGNORE_CTTY:
#endif
#if O_NOATIME
    case O_NOATIME:
#endif
#if O_NONBLOCK
    case O_NONBLOCK:
#endif
#if O_NOCTTY
    case O_NOCTTY:
#endif
#if O_NOFOLLOW
    case O_NOFOLLOW:
#endif
#if O_NOLINK
    case O_NOLINK:
#endif
#if O_NOLINKS
    case O_NOLINKS:
#endif
#if O_NOTRANS
    case O_NOTRANS:
#endif
#if O_RSYNC && O_RSYNC != O_DSYNC
    case O_RSYNC:
#endif
#if O_SYNC && O_SYNC != O_DSYNC && O_SYNC != O_RSYNC
    case O_SYNC:
#endif
#if O_TTY_INIT
    case O_TTY_INIT:
#endif
#if O_BINARY
    case O_BINARY:
#endif
#if O_TEXT
    case O_TEXT:
#endif
      ;
    }

  return !i;
}
