// dwp_test.h -- a test case for dwp, header file  -*- C++ -*-

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

// Adapted from two_file_test.h.

class C1
{
 public:
  bool testcase1();
  bool t1a();
  int t1_2();
  bool testcase2();
  bool testcase3();
  bool testcase4();
  int member1;
};

class C2
{
 public:
  bool testcase1();
  bool testcase2();
  bool testcase3();
  bool testcase4();
  int member1;
};

class C3
{
 public:
  bool testcase1();
  bool testcase2();
  bool testcase3();
  bool (*f4())();
  int member1;
};

extern C3 c3;

extern int v2;
extern int v3;
extern char v4[];
extern char v5[];

extern int f10();
extern int f11a();
extern int f11b(int (*)());
extern bool t12();

extern bool t13();
inline void f13i() { }
extern void (*f13())();

#define TEST_STRING_CONSTANT "test string constant"
extern const char* f14();

#define TEST_WIDE_STRING_CONSTANT L"test wide string constant"
extern const wchar_t* f15();

extern bool t16();
extern bool t16a();

extern bool t17();
extern const char* t17data[];
#define T17_COUNT 5

extern bool t18();
extern const char* f18(int);
