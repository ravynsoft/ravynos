/* plugin_common_test_1.c -- test common symbol handling in plugins

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Cary Coutant <ccoutant@google.com>

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

   This is a test of common symbols in plugin objects.  C1-C5 test
   various combinations of tentative definitions, extern declarations,
   and definitions.  */

#include <assert.h>

int c1;
int c2;
extern int c3;
int c4;
int c5 = 50;

extern void foo (void);

int
main (int argc __attribute__ ((unused)), char** argv __attribute__ ((unused)))
{
  foo();

  assert (c1 == 10);
  assert (c2 == 20);
  assert (c3 == 30);
  assert (c4 == 40);

  return 0;
}
