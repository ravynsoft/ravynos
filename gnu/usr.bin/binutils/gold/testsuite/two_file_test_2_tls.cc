// two_file_test_2_tls.cc -- a two file test case for gold, with a TLS symbol

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

// This tests references between files.  This is file 2, and
// two_file_test_1.cc is file 1.  See file 1 for details.

#include "two_file_test.h"

// 1  Code in file 1 calls code in file 2.

__thread int tls1 = 123;

int
t1_2()
{
  return tls1;
}

bool
t1a()
{
  return t1_2() == 123;
}

// 2  Code in file 1 refers to global data in file 2.

int v2 = 456;

// 3 Code in file 1 referes to common symbol in file 2.  This is
// initialized at runtime to 789.

int v3;

// 4  Code in file 1 refers to offset within global data in file 2.

char v4[] = "Hello, world";

// 5 Code in file 1 refers to offset within common symbol in file 2.
// This is initialized at runtime to a copy of v4.

char v5[13];

// 6  Data in file 1 refers to global data in file 2.  This reuses v2.

// 7  Data in file 1 refers to common symbol in file 2.  This reuses v3.

// 8 Data in file 1 refers to offset within global data in file 2.
// This reuses v4.

// 9 Data in file 1 refers to offset within common symbol in file 2.
// This reuses v5.

// 10 Data in file 1 refers to function in file 2.

int
f10()
{
  return 135;
}

// 11 Pass function pointer from file 1 to file 2.

int
f11b(int (*pfn)())
{
  return (*pfn)();
}

// 12 Compare address of function for equality in both files.

bool
(*f12())()
{
  return &t12;
}

// 13 Compare address of inline function for equality in both files.

void
(*f13())()
{
  return &f13i;
}

// 14 Compare string constants in file 1 and file 2.

const char*
f14()
{
  return TEST_STRING_CONSTANT;
}

// 15 Compare wide string constants in file 1 and file 2.

const wchar_t*
f15()
{
  return TEST_WIDE_STRING_CONSTANT;
}

// 17 File 1 checks array of string constants defined in file 2.

const char* t17data[T17_COUNT] =
{
  "a", "b", "c", "d", "e"
};

// 18 File 1 checks string constants referenced directly in file 2.

const char*
f18(int i)
{
  switch (i)
    {
    case 0:
      return "a";
    case 1:
      return "b";
    case 2:
      return "c";
    case 3:
      return "d";
    case 4:
      return "e";
    default:
      return 0;
    }
}
