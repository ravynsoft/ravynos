// two_file_test_1.cc -- a two file test case for gold, file 1 of 2

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

// For incremental linking tests, this file needs to be compiled with
// -fno-exceptions -fno-asynchronous-unwind-tables.

// This tests references between files.  This is file 1, and
// two_file_test_2.cc is file 2.  We test in several different ways:

// Files 1 and 2 linked together in executable.
// File 1 in executable, file 2 in shared library.
// File 1 in shared library, file 2 in executable.
// Files 1 and 2 linked together in shared library.
// Files 1 and 2 in different shared libraries.

// We test the following cases.

// 1  Code in file 1 calls code in file 2.
// 2  Code in file 1 refers to global data in file 2.
// 3  Code in file 1 referes to common symbol in file 2.
// 4  Code in file 1 refers to offset within global data in file 2.
// 5  Code in file 1 refers to offset within common symbol in file 2.
// 6  Data in file 1 refers to global data in file 2.
// 7  Data in file 1 refers to common symbol in file 2.
// 8  Data in file 1 refers to offset within global data in file 2.
// 9  Data in file 1 refers to offset within common symbol in file 2.
// 10 Data in file 1 refers to function in file 2.
// 11 Pass function pointer from file 1 to file 2.
// 12 Compare address of function for equality in both files.
// 13 Compare address of inline function for equality in both files.
// 14 Compare string constants in file 1 and file 2.
// 15 Compare wide string constants in file 1 and file 2.
// 16 Call a function directly after its address has been taken.
// 17 File 1 checks array of string constants defined in file 2.
// 18 File 1 checks string constants referenced in code in file 2.

#include "two_file_test.h"

// 1  Code in file 1 calls code in file 2.

bool
t1()
{
  return t1_2() == 123;
}

// 2  Code in file 1 refers to global data in file 2.

bool
t2()
{
  return v2 == 456;
}

// 3  Code in file 1 referes to common symbol in file 2.

bool
t3()
{
  return v3 == 789;
}

// 4  Code in file 1 refers to offset within global data in file 2.

bool
t4()
{
  return v4[5] == ',';
}

// 5  Code in file 1 refers to offset within common symbol in file 2.

bool
t5()
{
  return v5[7] == 'w';
}

// 6  Data in file 1 refers to global data in file 2.

int* p6 = &v2;

bool
t6()
{
  return *p6 == 456;
}

// 7  Data in file 1 refers to common symbol in file 2.

int* p7 = &v3;

bool
t7()
{
  return *p7 == 789;
}

// 8  Data in file 1 refers to offset within global data in file 2.

char* p8 = &v4[6];

bool
t8()
{
  return *p8 == ' ';
}

// 9  Data in file 1 refers to offset within common symbol in file 2.

char* p9 = &v5[8];

bool
t9()
{
  return *p9 == 'o';
}

// 10 Data in file 1 refers to function in file 2.

int (*pfn)() = &f10;

bool
t10()
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
t11()
{
  return f11b(&f11a) == 246;
}

// 12 Compare address of function for equality in both files.

bool
t12()
{
  return &t12 == f12();
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
