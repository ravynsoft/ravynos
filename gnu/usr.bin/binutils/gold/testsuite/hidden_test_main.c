/* hidden_test_main.c -- test hidden and internal symbols

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

extern void lib1 (void);

void main_default (void);
void main_hidden (void);
void main_internal (void);
void main_protected (void);

void __attribute__((visibility ("default")))
main_default (void)
{
}

void __attribute__((visibility ("hidden")))
main_hidden (void)
{
}

void __attribute__((visibility ("internal")))
main_internal (void)
{
}

void __attribute__((visibility ("protected")))
main_protected (void)
{
}

int
main (int argc __attribute__ ((unused)),
      char** argv __attribute__ ((unused)))
{
  lib1 ();
  return 0;
}
