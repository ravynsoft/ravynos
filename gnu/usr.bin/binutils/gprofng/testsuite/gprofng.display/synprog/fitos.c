/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdio.h>
#include <stdlib.h>
#include "stopwatch.h"

/* The random number generator below is adapted from Kernighan and Ritchie,
 *  "C Programming Language", Second Edition, p. 46.
 */

#define IA 1103515245u
#define IC 12345u
#define IM 2147483648u

static unsigned int current_random = 1;

static int
my_irand (int imax)
{
  /* Create a random integer between 0 and imax, inclusive.  i.e. [0..imax] */

  /* Use overflow to wrap */
  current_random = current_random * IA + IC;
  int ival = current_random & (IM - 1); /* Modulus */
  ival = (int) ((float) ival * (float) (imax + 0.999) / (float) IM);
  return ival;
}

#define N 6000000

int
fitos (int n)
{
  int i, k, retv;
  hrtime_t start = gethrtime ();
  hrtime_t vstart = gethrvtime ();

  /* Log the event */
  wlog ("start of fitos", NULL);

  if (n <= 0)
    n = 1;
  int max = N * n;

  long long count = 0;
  do
    {
      for (current_random = 1, i = retv = k = 0; i < max; i++)
        {
          retv += my_irand (100);
          k += (current_random & (IM - 1)) >= (1 << 22);
        }
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());
  fprintf (stderr, "\t\t%d out of a total of %d >= 2^22 (%d)\n", k, max, retv);
  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog ((gethrtime () - start), (gethrvtime () - vstart), "fitos", NULL);
  return 0;
}
