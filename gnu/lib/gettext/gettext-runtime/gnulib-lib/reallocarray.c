/* reallocarray function that is glibc compatible.

   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

/* written by Darshit Shah */

#include <config.h>

#include <stdckdint.h>
#include <stdlib.h>
#include <errno.h>

void *
reallocarray (void *ptr, size_t nmemb, size_t size)
{
  size_t nbytes;
  if (ckd_mul (&nbytes, nmemb, size))
    {
      errno = ENOMEM;
      return NULL;
    }

  /* Rely on the semantics of GNU realloc.  */
  return realloc (ptr, nbytes);
}
