/* initpri2.c -- test mixing init_array and ctor priorities.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Copied from the gcc configury, where the test was contributed by
   H.J. Lu <hongjiu.lu@intel.com>.

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

/* This tests that the linker correctly combines .ctor and .init_array
   sections when both have priorities.  */

#include <stdlib.h>

static int count;

static void
init1005 (void)
{
  if (count != 0)
    abort ();
  count = 1005;
}
void (*const init_array1005[]) (void)
  __attribute__ ((section (".init_array.01005"), aligned (sizeof (void *))))
  = { init1005 };
static void
fini1005 (void)
{
  if (count != 1005)
    abort ();
}
void (*const fini_array1005[]) (void)
  __attribute__ ((section (".fini_array.01005"), aligned (sizeof (void *))))
  = { fini1005 };

static void
ctor1007 (void)
{
  if (count != 1005)
    abort ();
  count = 1007;
}
void (*const ctors1007[]) (void)
  __attribute__ ((section (".ctors.64528"), aligned (sizeof (void *))))
  = { ctor1007 };
static void
dtor1007 (void)
{
  if (count != 1007)
    abort ();
  count = 1005;
}
void (*const dtors1007[]) (void)
  __attribute__ ((section (".dtors.64528"), aligned (sizeof (void *))))
  = { dtor1007 };

static void
init65530 (void)
{
  if (count != 1007)
    abort ();
  count = 65530;
}
void (*const init_array65530[]) (void)
  __attribute__ ((section (".init_array.65530"), aligned (sizeof (void *))))
  = { init65530 };
static void
fini65530 (void)
{
  if (count != 65530)
    abort ();
  count = 1007;
}
void (*const fini_array65530[]) (void)
  __attribute__ ((section (".fini_array.65530"), aligned (sizeof (void *))))
  = { fini65530 };

static void
ctor65535 (void)
{
  if (count != 65530)
    abort ();
  count = 65535;
}
void (*const ctors65535[]) (void)
  __attribute__ ((section (".ctors"), aligned (sizeof (void *))))
  = { ctor65535 };
static void
dtor65535 (void)
{
  if (count != 65535)
    abort ();
  count = 65530;
}
void (*const dtors65535[]) (void)
  __attribute__ ((section (".dtors"), aligned (sizeof (void *))))
  = { dtor65535 };

int
main (void)
{
  return 0;
}
