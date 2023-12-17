/* An interface to read and write that retries (if necessary) until complete.

   Copyright (C) 1993-1994, 1997-2006, 2009-2023 Free Software Foundation, Inc.

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
#ifdef FULL_READ
# include "full-read.h"
#else
# include "full-write.h"
#endif

#include <errno.h>

#ifdef FULL_READ
# include "safe-read.h"
# define safe_rw safe_read
# define full_rw full_read
# undef const
# define const /* empty */
#else
# include "safe-write.h"
# define safe_rw safe_write
# define full_rw full_write
#endif

#ifdef FULL_READ
/* Set errno to zero upon EOF.  */
# define ZERO_BYTE_TRANSFER_ERRNO 0
#else
/* Some buggy drivers return 0 when one tries to write beyond
   a device's end.  (Example: Linux 1.2.13 on /dev/fd0.)
   Set errno to ENOSPC so they get a sensible diagnostic.  */
# define ZERO_BYTE_TRANSFER_ERRNO ENOSPC
#endif

/* Write(read) COUNT bytes at BUF to(from) descriptor FD, retrying if
   interrupted or if a partial write(read) occurs.  Return the number
   of bytes transferred.
   When writing, set errno if fewer than COUNT bytes are written.
   When reading, if fewer than COUNT bytes are read, you must examine
   errno to distinguish failure from EOF (errno == 0).  */
size_t
full_rw (int fd, const void *buf, size_t count)
{
  size_t total = 0;
  const char *ptr = (const char *) buf;

  while (count > 0)
    {
      size_t n_rw = safe_rw (fd, ptr, count);
      if (n_rw == (size_t) -1)
        break;
      if (n_rw == 0)
        {
          errno = ZERO_BYTE_TRANSFER_ERRNO;
          break;
        }
      total += n_rw;
      ptr += n_rw;
      count -= n_rw;
    }

  return total;
}
