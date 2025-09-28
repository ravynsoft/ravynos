// dwp_test_1.cc -- a test case for dwp

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

// Adapted from two_file_test_1.cc.

#include "dwp_test.h"

// 1  Code in file 1 calls code in file 2.

bool
C1::testcase1()
{
  return t1_2() == 123;
}

// 2  Code in file 1 refers to global data in file 2.

bool
C1::testcase2()
{
  return v2 == 456;
}

// 3  Code in file 1 referes to common symbol in file 2.

bool
C1::testcase3()
{
  return v3 == 789;
}

// 4  Code in file 1 refers to offset within global data in file 2.

bool
C1::testcase4()
{
  return v4[5] == ',';
}

// 5  Code in file 1 refers to offset within common symbol in file 2.

bool
C2::testcase1()
{
  return v5[7] == 'w';
}

// 6  Data in file 1 refers to global data in file 2.

int* p6 = &v2;

bool
C2::testcase2()
{
  return *p6 == 456;
}

// 7  Data in file 1 refers to common symbol in file 2.

int* p7 = &v3;

bool
C2::testcase3()
{
  return *p7 == 789;
}

// 8  Data in file 1 refers to offset within global data in file 2.

char* p8 = &v4[6];

bool
C2::testcase4()
{
  return *p8 == ' ';
}

// 9  Data in file 1 refers to offset within common symbol in file 2.

char* p9 = &v5[8];

bool
C3::testcase1()
{
  return *p9 == 'o';
}

// 10 Data in file 1 refers to function in file 2.

int (*pfn)() = &f10;

bool
C3::testcase2()
{
  return (*pfn)() == 135;
}

// 11 Pass function pointer from file 1 to file 2.

int
f11a()
{
  return 246;
}

bool
C3::testcase3()
{
  return f11b(&f11a) == 246;
}

// 12 Compare address of function for equality in both files.

bool
t12()
{
  return &t12 == c3.f4();
}

// 13 Compare address of inline function for equality in both files.

bool
t13()
{
  return &f13i == f13();
}

// 14 Compare string constants in file 1 and file 2.

bool
t14()
{
  const char* s1 = TEST_STRING_CONSTANT;
  const char* s2 = f14();
  while (*s1 != '\0')
    if (*s1++ != *s2++)
      return false;
  return *s2 == '\0';
}

// 15 Compare wide string constants in file 1 and file 2.

bool
t15()
{
  const wchar_t* s1 = TEST_WIDE_STRING_CONSTANT;
  const wchar_t* s2 = f15();
  while (*s1 != '\0')
    if (*s1++ != *s2++)
      return false;
  return *s2 == '\0';
}

// 16 Call a function directly after its address has been taken.

bool
t16()
{
  return f10() == 135;
}

// 17 File 1 checks array of string constants defined in file 2.

bool
t17()
{
  char c = 'a';
  for (int i = 0; i < T17_COUNT; ++i)
    {
      if (t17data[i][0] != c || t17data[i][1] != '\0')
	return false;
      ++c;
    }
  return true;
}

// 18 File 1 checks string constants referenced in code in file 2.

bool
t18()
{
  char c = 'a';
  for (int i = 0; i < T17_COUNT; ++i)
    {
      const char* s = f18(i);
      if (s[0] != c || s[1] != '\0')
        return false;
      ++c;
    }
  return true;
}
