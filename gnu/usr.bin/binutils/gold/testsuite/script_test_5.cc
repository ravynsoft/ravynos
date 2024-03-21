// script_test_5.cc -- a test case for gold

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

// This program checks that the default renaming of ".text.xxx"
// sections does not take place in the presence of a linker script
// with a SECTIONS clause.

bool
t1() __attribute__ ((section (".text.foo")));

bool
t1()
{
  return 1;
}

// Main function.  Initialize variables and call test functions.

int
main()
{
  if (t1())
    return 0;
  else
    return 1;
}
