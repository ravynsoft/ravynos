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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "stopwatch.h"

/* endcases - examine some wierd endcases of programming style
 *  test cases for inlined code, macros, #included code, ...
 */
void inc_func (int);
void inc_brace (int);
void inc_body (int);
void inc_entry (int);
void inc_middle (int);
void inc_exit (int);
void macro_code (int);
void ext_macro_code (int);
void xinline_code (int);
static void s_inline_code (int);
void ext_inline_code (int);

#ifndef NO_INLINE
void xinline_code () __attribute__ ((always_inline));
void s_inline_code () __attribute__ ((always_inline));
#endif

#include "inc_inline.h"

int n;
int x1M = 1000000;
int x2M = 2000000;
int x8M = 8000000;

/* define a macro that burns CPU time */
#define burncpu(nn) \
        x = 0; \
        for (j = 0; j < (nn * x8M); j++) { \
                 x = x + 1; \
        }

int
endcases (int n)
{
  hrtime_t start = gethrtime ();
  hrtime_t vstart = gethrvtime ();

  /* Log the event */
  wlog ("start of endcases", NULL);

  if (n == 0)
    n = 4;

  long long count = 0;
  do
    {
      /* test inlines */
      xinline_code (n);
      s_inline_code (n);
      ext_inline_code (n);

      /* test macros */
      macro_code (n);
      ext_macro_code (n);

      /* test various cases of #include'd code */
      inc_func (n);
      inc_brace (n);
      inc_body (n);
      inc_entry (n);
      inc_middle (n);
      inc_exit (n);
      count++;
    }
  while (start + testtime * 1e9 > gethrtime ());

  fprintf (stderr, "   Performed %lld while-loop iterations\n", count);
  whrvlog (gethrtime () - start, gethrvtime () - vstart, "endcases", NULL);
  return 0;
}

/* spend time in a inline locally-defined */
void
xinline_code (int n)
{
  int jmax = n * x8M;
  volatile long x = 0;
  for (int j = 0; j < jmax; j++)
    x = x + 1;
  if (x < 0.0)
    printf ("ERROR: inline_code(): x < 0 (x=%ld)\n", x);
}

/* spend time in a static inline locally-defined */
static void
s_inline_code (int n)
{
  int jmax = n * x8M;
  volatile long x = 0;
  for (int j = 0; j < jmax; j++)
    x = x + 1;
  if (x < 0.0)
    printf ("ERROR: s_inline_code(): x < 0 (x=%ld)\n", x);
}

/* spend time in a macro locally-defined */
void
macro_code (int n)
{
  int j;
  volatile long x = 0;
  burncpu (n);
  if (x < 0.0)
    printf ("ERROR: macro_code(): x < 0 (x=%ld)\n", x);
}

/* spend time in a macro externally-defined */
#include "inc_macro.h"

void
ext_macro_code (int n)
{
  volatile long x = 0;
  int j;
  extburncpu (n);
  if (x < 0.0)
    printf ("ERROR: ext_macro_code(): x < 0 (x=%ld)\n", x);
}

#include "inc_func.h"

void
inc_brace (int n)
#include "inc_brace.h"

void
inc_body (int n) {
#include "inc_body.h"
}

void
inc_entry (int n)
#include "inc_entry.h"
{
  int jmax = n * x8M;
  volatile float x = 0.0;
  for (int j = 0; j < jmax; j++)
    x = x + 1.0;
  if (x < 0.0)
    printf ("ERROR: inc_entry(): x < 0 (x=%f)\n", x);
}
}

void
inc_middle (int n)
{
  {
    int jmax = n * x8M;
    volatile float x = 0.0;
    for (int j = 0; j < jmax; j++)
      x = x + 1.0;
    if (x < 0.0)
      printf ("ERROR: inc_middle(): loop 1: x < 0 (x=%f)\n", x);
  }
#include "inc_body.h"
  {
    int jmax = n * x8M;
    volatile float x = 0.0;
    for (int j = 0; j < jmax; j++)
      x = x + 1.0;
    if (x < 0.0)
      printf ("ERROR: inc_middle(): loop 2: x < 0 (x=%f)\n", x);
  }
}

void
inc_exit (int n)
{
  {
    int jmax = n * x8M;
    volatile float x = 0.0;
    for (int j = 0; j < jmax; j++)
      x = x + 1.0;
    if (x < 0.0)
      printf ("ERROR: inc_exit(): x < 0 (x=%f)\n", x);
  }

#include "inc_exit.h"

