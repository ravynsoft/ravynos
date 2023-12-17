/* c-strcasestr.c -- case insensitive substring search in C locale
   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2005.

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
#include "c-strcasestr.h"

#include <string.h>

#include "c-ctype.h"
#include "c-strcase.h"

/* Two-Way algorithm.  */
#define RETURN_TYPE char *
#define AVAILABLE(h, h_l, j, n_l)                       \
  (!memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))     \
   && ((h_l) = (j) + (n_l)))
#define CANON_ELEMENT c_tolower
#define CMP_FUNC(p1, p2, l)                             \
  c_strncasecmp ((const char *) (p1), (const char *) (p2), l)
#include "str-two-way.h"

/* Find the first occurrence of NEEDLE in HAYSTACK, using case-insensitive
   comparison from the C locale, regardless of the current locale.  */
char *
c_strcasestr (const char *haystack_start, const char *needle_start)
{
  const char *haystack = haystack_start;
  const char *needle = needle_start;
  size_t needle_len; /* Length of NEEDLE.  */
  size_t haystack_len; /* Known minimum length of HAYSTACK.  */
  bool ok = true; /* True if NEEDLE is prefix of HAYSTACK.  */

  /* Determine length of NEEDLE, and in the process, make sure
     HAYSTACK is at least as long (no point processing all of a long
     NEEDLE if HAYSTACK is too short).  */
  while (*haystack && *needle)
    ok &= (c_tolower ((unsigned char) *haystack++)
           == c_tolower ((unsigned char) *needle++));
  if (*needle)
    return NULL;
  if (ok)
    return (char *) haystack_start;
  needle_len = needle - needle_start;
  haystack = haystack_start + 1;
  haystack_len = needle_len - 1;

  /* Perform the search.  Abstract memory is considered to be an array
     of 'unsigned char' values, not an array of 'char' values.  See
     ISO C 99 section 6.2.6.1.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    return two_way_short_needle ((const unsigned char *) haystack,
                                 haystack_len,
                                 (const unsigned char *) needle_start,
                                 needle_len);
  return two_way_long_needle ((const unsigned char *) haystack, haystack_len,
                              (const unsigned char *) needle_start,
                              needle_len);
}

#undef LONG_NEEDLE_THRESHOLD
