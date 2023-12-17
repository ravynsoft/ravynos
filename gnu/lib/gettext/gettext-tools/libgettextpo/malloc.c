/* malloc() function that is glibc compatible.

   Copyright (C) 1997-1998, 2006-2007, 2009-2023 Free Software Foundation, Inc.

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

/* written by Jim Meyering and Bruno Haible */

#define _GL_USE_STDLIB_ALLOC 1
#include <config.h>

#include <stdlib.h>

#include <errno.h>

#include "xalloc-oversized.h"

/* Allocate an N-byte block of memory from the heap, even if N is 0.  */

void *
rpl_malloc (size_t n)
{
  if (n == 0)
    n = 1;

  if (xalloc_oversized (n, 1))
    {
      errno = ENOMEM;
      return NULL;
    }

  void *result = malloc (n);

#if !HAVE_MALLOC_POSIX
  if (result == NULL)
    errno = ENOMEM;
#endif

  return result;
}
