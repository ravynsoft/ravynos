/* initpri3.c -- test ctor odering when using init_array.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

/* This tests that the linker correctly orders .ctor entries when
   putting them into .init_array, as is the default.  */

#include <assert.h>

int i = 1;

static void
ctor1 (void)
{
  assert (i == 1);
  i = 2;
}

static void
ctor2 (void)
{
  assert (i == 2);
  i = 3;
}

static void
dtor1 (void)
{
  assert (i == 3);
  i = 2;
}

static void
dtor2 (void)
{
  assert (i == 2);
  i = 1;
}

/* The .ctors section is run in reverse order, the .dtors section in
   run in forward order.  We give these arrays the "aligned" attribute
   because the x86_64 ABI would otherwise give them a 16-byte
   alignment, which may leave a hole in the section.  */

void (*ctors[]) (void)
  __attribute__ ((aligned (4), section (".ctors"))) = {
  ctor2,
  ctor1
};

void (*dtors[]) (void)
  __attribute__ ((aligned (4), section (".dtors"))) = {
  dtor1,
  dtor2
};

int
main (void)
{
  assert (i == 3);
  return 0;
}
