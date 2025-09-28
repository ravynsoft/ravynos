// ver_test_4.cc -- a test case for gold

// Copyright (C) 2007-2023 Free Software Foundation, Inc.
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

#include "ver_test.h"

__asm__(".symver t1_2_orig,t1_2@");

extern "C"
int
t1_2_orig()
{
  TRACE
  return 12;
}

__asm__(".symver t1_2_a,t1_2@@VER2");

extern "C"
int
t1_2_a()
{
  TRACE
  return 12;
}

__asm__(".symver t2_2_a,t2_2@VER1");

extern "C"
int
t2_2_a()
{
  TRACE
  return 21;
}

__asm__(".symver t2_2_b,t2_2@@VER2");

extern "C"
int
t2_2_b()
{
  TRACE
  return 22;
}


// This function is given a version by the version script, and should
// be overridden by the main program.

int
t4_2a()
{
  TRACE
  return -42;
}
