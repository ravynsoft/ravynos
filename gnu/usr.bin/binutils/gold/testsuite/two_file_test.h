// two_file_test.h -- a two file test case for gold, header file  -*- C++ -*-

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

// This tests references between files.  This is the shared header
// file.  See two_file_test_1.cc for details.

extern bool t1();
extern bool t1a();
extern int t1_2();

extern bool t2();
extern int v2;

extern bool t3();
extern int v3;

extern bool t4();
extern char v4[];

extern bool t5();
extern char v5[];

extern bool t6();

extern bool t7();

extern bool t8();

extern bool t9();

extern bool t10();
extern int f10();

extern bool t11();
extern int f11a();
extern int f11b(int (*)());

extern bool t12();
extern bool (*f12())();

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
