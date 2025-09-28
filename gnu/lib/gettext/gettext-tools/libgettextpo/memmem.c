/* Copyright (C) 1991-1994, 1996-1998, 2000, 2004, 2007-2023 Free Software
   Foundation, Inc.
   This file is part of the GNU C Library.

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

/* This particular implementation was written by Eric Blake, 2008.  */

#ifndef _LIBC
# include <config.h>
#endif

/* Specification of memmem.  */
#include <string.h>

#define RETURN_TYPE void *
#define AVAILABLE(h, h_l, j, n_l) ((j) <= (h_l) - (n_l))
#include "str-two-way.h"

/* Return the first occurrence of NEEDLE in HAYSTACK.  Return HAYSTACK
   if NEEDLE_LEN is 0, otherwise NULL if NEEDLE is not found in
   HAYSTACK.  */
void *
memmem (const void *haystack_start, size_t haystack_len,
        const void *needle_start, size_t needle_len)
{
  /* Abstract memory is considered to be an array of 'unsigned char' values,
     not an array of 'char' values.  See ISO C 99 section 6.2.6.1.  */
  const unsigned char *haystack = (const unsigned char *) haystack_start;
  const unsigned char *needle = (const unsigned char *) needle_start;

  if (needle_len == 0)
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string.  */
    return (void *) haystack;

  /* Sanity check, otherwise the loop might search through the whole
     memory.  */
  if (__builtin_expect (haystack_len < needle_len, 0))
    return NULL;

  /* Use optimizations in memchr when possible, to reduce the search
     size of haystack using a linear algorithm with a smaller
     coefficient.  However, avoid memchr for long needles, since we
     can often achieve sublinear performance.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    {
      haystack = memchr (haystack, *needle, haystack_len);
      if (!haystack || __builtin_expect (needle_len == 1, 0))
        return (void *) haystack;
      haystack_len -= haystack - (const unsigned char *) haystack_start;
      if (haystack_len < needle_len)
        return NULL;
      return two_way_short_needle (haystack, haystack_len, needle, needle_len);
    }
  else
    return two_way_long_needle (haystack, haystack_len, needle, needle_len);
}

#undef LONG_NEEDLE_THRESHOLD
