/* xgetaname-impl.c -- common implementation of xgethostname and xgetdomainname

   Copyright (C) 1992, 1996, 2000-2001, 2003-2006, 2009-2023 Free Software
   Foundation, Inc.

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

/* written by Jim Meyering and Paul Eggert */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "xalloc.h"

/* Return the current host or domain name in malloc'd storage.
   If malloc fails, exit.
   Upon any other failure, return NULL and set errno.  */
char *
XGETANAME (void)
{
  char buf[100];
  idx_t size = sizeof buf;
  char *name = buf;
  char *alloc = NULL;

  while (1)
    {
      /* Use SIZE_1 here rather than SIZE to work around the bug in
         SunOS 5.5's gethostname whereby it NUL-terminates HOSTNAME
         even when the name is as long as the supplied buffer.  */
      idx_t size_1 = size - 1;
      name[size_1] = '\0';
      errno = 0;
      if (GETANAME (name, size_1) == 0)
        {
          /* Check whether the name was possibly truncated; POSIX does not
             specify whether a truncated name is null-terminated.  */
          idx_t actual_size = strlen (name) + 1;
          if (actual_size < size_1)
            return alloc ? alloc : ximemdup (name, actual_size);
          errno = 0;
        }
      free (alloc);
      if (errno != 0 && errno != ENAMETOOLONG && errno != EINVAL
          /* macOS/Darwin does this when SIZE_1 is too small.  */
          && errno != ENOMEM)
        return NULL;
      name = alloc = xpalloc (NULL, &size, 1, -1, 1);
    }
}
