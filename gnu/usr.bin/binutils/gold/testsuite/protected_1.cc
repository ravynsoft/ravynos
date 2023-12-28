// protected_1.cc -- a test case for gold

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

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

// The function f1 is protected, which means that other callers in the
// same shared library will call this version.

int
f1() __attribute__ ((__visibility__ ("protected")));

int
f1()
{
  return 1;
}

// The function f2 is used to test that the executable can see the
// same function address for a protected function in the executable
// and in the shared library.  We can't use the visibility attribute
// here, becaues that may cause gcc to generate a PC relative reloc;
// we need it to get the value from the GOT.  I'm not sure this is
// really useful, given that it doesn't work with the visibility
// attribute.  This test exists here mainly because the glibc
// testsuite has the same test, and we want to make sure that gold
// passes the glibc testsuite.

extern "C" int f2();
asm(".protected f2");

extern "C" int
f2()
{
  return 2;
}

int
(*get_f2_addr())()
{
  return f2;
}
