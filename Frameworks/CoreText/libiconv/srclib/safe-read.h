/* An interface to read() that retries after interrupts.
   Copyright (C) 2002, 2006, 2009-2026 Free Software Foundation, Inc.

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

/* Some system calls may be interrupted and fail with errno = EINTR in the
   following situations:
     - The process is stopped and restarted (signal SIGSTOP and SIGCONT, user
       types Ctrl-Z) on some platforms: Mac OS X.
     - The process receives a signal for which a signal handler was installed
       with sigaction() with an sa_flags field that does not contain
       SA_RESTART.
     - The process receives a signal for which a signal handler was installed
       with signal() and for which no call to siginterrupt(sig,0) was done,
       on some platforms: AIX, HP-UX, Solaris.

   This module provides a wrapper around read() that handles EINTR.  */

#include <stddef.h>

#include "idx.h"

#ifdef __cplusplus
extern "C" {
#endif


/* This is present for backward compatibility with older versions of this code
   where safe_read returned size_t, so SAFE_READ_ERROR was SIZE_MAX.  */
#define SAFE_READ_ERROR ((ptrdiff_t) -1)

/* Read up to COUNT bytes at BUF from descriptor FD, retrying if interrupted.
   Return the number of bytes read, zero for EOF, or -1 upon error.  */
extern ptrdiff_t safe_read (int fd, void *buf, idx_t count);


#ifdef __cplusplus
}
#endif
