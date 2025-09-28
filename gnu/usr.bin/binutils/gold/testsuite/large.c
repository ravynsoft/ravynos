/* large.c -- a test case for gold

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>.

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
   MA 02110-1301, USA.  */

#include <assert.h>

/* Test large sections in gold.  */

int v1;
int v2 = 1;
int v3[0x10000];
int v4[0x10000] = { 1 };
const int v5[0x10000] = { 2 };
int v6;
int v7 = 1;

int
main (int argc __attribute__ ((unused)), char** argv __attribute ((unused)))
{
  assert (v1 == 0);
  assert (v2 == 1);
  assert (v3[0] == 0 && v3[0xffff] == 0);
  assert (v4[0] == 1 && v4[0xffff] == 0);
  assert (v5[0] == 2 && v5[0xffff] == 0);
  assert (v6 == 0);
  assert (v7 == 1);

  /* The large symbols must follow the small ones.  */
  assert (&v1 < v3 && &v1 < v4 && &v1 < v5);
  assert (&v2 < v3 && &v2 < v4 && &v2 < v5);
  assert (&v6 < v3 && &v6 < v4 && &v6 < v5);
  assert (&v7 < v3 && &v7 < v4 && &v7 < v5);

  /* Large symbols should be BSS followed by read-only followed by
     read-write.  */
  assert (v3 < v4);
  assert (v3 < v5);
  assert (v5 < v4);

  return 0;
}
