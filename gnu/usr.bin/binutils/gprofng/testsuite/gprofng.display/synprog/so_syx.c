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

static void sx_burncpu ();

int
sx_cputime ()
{
  wlog ("start of sx_cputime", NULL);

  /* put a memory leak in here */
  (void) malloc (13);

  fprintf (stderr, "sx_burncpu @ %p\n", sx_burncpu);
  sx_burncpu ();
  wlog ("end of sx_cputime", NULL);
  return 13;
}

void sx_init () __attribute__ ((constructor));

void
sx_init ()
{
  fprintf (stderr, "sx_init executed\n");
}

/* sx_burncpu - loop to use a bunch of user time */
void
sx_burncpu ()
{
  hrtime_t start = gethrtime ();
  hrtime_t vstart = gethrvtime ();
  volatile float x = 0.;    /* temp variable for f.p. calculation */
  long long count = 0;
  do
    {
      x = 0.0;
      for (int j = 0; j < 100000; j++)
        x = x + 1.0;
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, gethrvtime () - vstart, "sx_burncpu", NULL);
}
