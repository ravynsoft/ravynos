/* Arithmetic.
   Copyright (C) 2001-2002, 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2001.

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

#include <config.h>

/* This file can also be used to define gcd functions for other unsigned
   types, such as 'unsigned long long' or 'uintmax_t'.  */
#ifndef WORD_T
/* Specification.  */
# include "gcd.h"
# define WORD_T unsigned long
# define GCD gcd
#endif

#include <stdlib.h>

/* Return the greatest common divisor of a > 0 and b > 0.  */
WORD_T
GCD (WORD_T a, WORD_T b)
{
  /* Why no division, as in Euclid's algorithm? Because in Euclid's algorithm
     the division result floor(a/b) or floor(b/a) is very often = 1 or = 2,
     and nearly always < 8.  A sequence of a few subtractions and tests is
     faster than a division.  */
  /* Why not Euclid's algorithm? Because the two integers can be shifted by 1
     bit in a single instruction, and the algorithm uses fewer variables than
     Euclid's algorithm.  */

  WORD_T c = a | b;
  c = c ^ (c - 1);
  /* c = largest power of 2 that divides a and b.  */

  if (a & c)
    {
      if (b & c)
        goto odd_odd;
      else
        goto odd_even;
    }
  else
    {
      if (b & c)
        goto even_odd;
      else
        abort ();
    }

  for (;;)
    {
    odd_odd: /* a/c and b/c both odd */
      if (a == b)
        break;
      if (a > b)
        {
          a = a - b;
        even_odd: /* a/c even, b/c odd */
          do
            a = a >> 1;
          while ((a & c) == 0);
        }
      else
        {
          b = b - a;
        odd_even: /* a/c odd, b/c even */
          do
            b = b >> 1;
          while ((b & c) == 0);
        }
    }

  /* a = b */
  return a;
}
