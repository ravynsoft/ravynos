/* Test inttostr functions, and incidentally, INT_BUFSIZE_BOUND
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

/* Written by Jim Meyering.  */

#include <config.h>

#include "inttostr.h"
#include "intprops.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

#define STREQ(a, b) (strcmp (a, b) == 0)
#define IS_TIGHT(T) (_GL_SIGNED_TYPE_OR_EXPR (T) == TYPE_SIGNED (T))
#define ISDIGIT(c) ((unsigned int) (c) - '0' <= 9)

/* Verify that an inttostr function works as advertised.
   Convert maximum and minimum (per-type, T) values using both snprintf --
   with a cast to intmax_t or uintmax_t -- and FN, and compare the
   resulting strings.  Use malloc for the inttostr buffer, so that if
   we ever exceed the usually-tight INT_BUFSIZE_BOUND, tools like
   valgrind will detect the failure. */
#define CK(T, Fn)                                                       \
  do                                                                    \
    {                                                                   \
      char ref[100];                                                    \
      char *buf = malloc (INT_BUFSIZE_BOUND (T));                       \
      char const *p;                                                    \
      ASSERT (buf);                                                     \
      *buf = '\0';                                                      \
      ASSERT                                                            \
        ((TYPE_SIGNED (T)                                               \
          ? snprintf (ref, sizeof ref, "%jd", (intmax_t) TYPE_MINIMUM (T)) \
          : snprintf (ref, sizeof ref, "%ju", (uintmax_t) TYPE_MINIMUM (T))) \
         < sizeof ref);                                                 \
      ASSERT (STREQ ((p = Fn (TYPE_MINIMUM (T), buf)), ref));           \
      /* Ensure that INT_BUFSIZE_BOUND is tight for signed types.  */   \
      ASSERT (! TYPE_SIGNED (T) || (p == buf && *p == '-'));            \
      ASSERT                                                            \
        ((TYPE_SIGNED (T)                                               \
          ? snprintf (ref, sizeof ref, "%jd", (intmax_t) TYPE_MAXIMUM (T)) \
          : snprintf (ref, sizeof ref, "%ju", (uintmax_t) TYPE_MAXIMUM (T))) \
         < sizeof ref);                                                 \
      ASSERT (STREQ ((p = Fn (TYPE_MAXIMUM (T), buf)), ref));           \
      /* For unsigned types, the bound is not always tight.  */         \
      ASSERT (! IS_TIGHT (T) || TYPE_SIGNED (T)                         \
              || (p == buf && ISDIGIT (*p)));                           \
      free (buf);                                                       \
    }                                                                   \
  while (0)

int
main (void)
{
  size_t b_size = 2;
  char *b = malloc (b_size);
  ASSERT (b);

  /* Ideally we would rely on the snprintf-posix module, in which case
     this guard would not be required, but due to limitations in gnulib's
     implementation (see modules/snprintf-posix), we cannot.  */
  if (snprintf (b, b_size, "%ju", (uintmax_t) 3) == 1
      && b[0] == '3' && b[1] == '\0')
    {
      CK (int,          inttostr);
      CK (unsigned int, uinttostr);
      CK (off_t,        offtostr);
      CK (uintmax_t,    umaxtostr);
      CK (intmax_t,     imaxtostr);
      free (b);
      return 0;
    }

  /* snprintf doesn't accept %ju; skip this test.  */
  free (b);
  return 77;
}
