/* common_test_1_v2.c -- test common symbol sorting

   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>

   This file is part of gold.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.

   This is a test of a common symbol in the main program and a
   versioned symbol in a shared library.  The common symbol in the
   main program should override the shared library symbol.  */

#include <assert.h>

/* Common symbols should be sorted by size, largest first, and then by
   alignment, largest first.  We mix up the names, because gas seems
   to sort common symbols roughly by name.  */

int c9[90];
int c8[80];
int c7[70];
int c6[60];
int c5[10];
int c4[20];
int c3[30];
int c2[40];
int c1[50];

int a1 __attribute__ ((aligned (1 << 9)));
int a2 __attribute__ ((aligned (1 << 8)));
int a3 __attribute__ ((aligned (1 << 7)));
int a4 __attribute__ ((aligned (1 << 6)));
int a5 __attribute__ ((aligned (1 << 1)));
int a6 __attribute__ ((aligned (1 << 2)));
int a7 __attribute__ ((aligned (1 << 3)));
int a8 __attribute__ ((aligned (1 << 4)));
int a9 __attribute__ ((aligned (1 << 5)));

int
main (int argc __attribute__ ((unused)), char** argv __attribute__ ((unused)))
{
  // After an incremental update, all guarantees about ordering
  // are null.
  assert (&c5[0] != &c4[0]);
  assert (&c4[0] != &c3[0]);
  assert (&c3[0] != &c2[0]);
  assert (&c2[0] != &c1[0]);
  assert (&c1[0] != &c6[0]);
  assert (&c6[0] != &c7[0]);
  assert (&c7[0] != &c8[0]);
  assert (&c8[0] != &c9[0]);

  assert (&a1 != &a2);
  assert (&a2 != &a3);
  assert (&a3 != &a4);
  assert (&a4 != &a9);
  assert (&a9 != &a8);
  assert (&a8 != &a7);
  assert (&a7 != &a6);
  assert (&a6 != &a5);

  return 0;
}
