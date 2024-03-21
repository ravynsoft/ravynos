/* hash-pjw.c -- compute a hash value from a NUL-terminated string.

   Copyright (C) 2001, 2003, 2006, 2009-2023 Free Software Foundation, Inc.

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

#include "hash-pjw.h"

#include <limits.h>

#define SIZE_BITS (sizeof (size_t) * CHAR_BIT)

/* A hash function for NUL-terminated char* strings using
   the method described by Bruno Haible.
   See https://www.haible.de/bruno/hashfunc.html.  */

size_t
hash_pjw (const void *x, size_t tablesize)
{
  const char *s;
  size_t h = 0;

  for (s = x; *s; s++)
    h = *s + ((h << 9) | (h >> (SIZE_BITS - 9)));

  return h % tablesize;
}
