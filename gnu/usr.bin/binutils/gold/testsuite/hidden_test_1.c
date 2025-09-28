/* hidden_test_1.c -- test hidden and internal symbols

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
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

   This is a test of symbols of various visibilities in the main program
   and attempts to reference those symbols from a shared library.
   The linker should issue an error message for references to hidden
   and internal symbols.  */

extern void main_default (void);
extern void main_hidden (void);
extern void main_internal (void);
extern void main_protected (void);

int
lib1 (void)
{
  main_default ();
  main_hidden ();
  main_internal ();
  main_protected ();
  return 0;
}
