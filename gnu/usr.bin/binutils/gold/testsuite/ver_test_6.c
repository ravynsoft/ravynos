/* ver_test_6.c -- test common symbol with shared library version

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

int t3_2;

/* Since we don't use any of the arguments to main, we give it a void
   prototype, so as to quiet gcc -Wstrict-prototypes.  */
int
main(void)
{
  return t3_2;
}
