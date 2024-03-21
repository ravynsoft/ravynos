// tls_test.cc -- test TLS variables for gold

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

// This provides a set of test functions for TLS variables.  The
// functions are called by a main function in tls_test_main.cc.  This
// lets us test TLS access from a shared library.  We currently don't
// bother to test TLS access between two different files, on the
// theory that that is no more complicated than ordinary variable
// access between files.

// We start two threads, and stop the second one.  Then we run the
// first thread through the following cases.  Then we let the second
// thread continue, and run it through the same set of cases.  All the
// actual thread manipulation is in tls_test_main.cc.

// 1  Access to an uninitialized global thread variable.
// 2  Access to an uninitialized static thread variable.
// 3  Access to an initialized global thread variable.
// 4  Access to an initialized static thread variable.
// 5  Taking the address of a global thread variable.
// 6  Taking the address of a static thread variable.
// 8  Like test 1, but with the thread variable defined in another file.
// 9  Like test 3, but with the thread variable defined in another file.
// 10 Like test 5, but with the thread variable defined in another file.
// last  Verify that the above tests left the variables set correctly.


#include "config.h"
#include <cstdio>
#include "tls_test.h"

#define CHECK_EQ_OR_RETURN(var, expected)				\
  do									\
    {									\
      if ((var) != (expected))						\
	{								\
	  printf(#var ": expected %d, found %d\n", expected, var);	\
	  return false;							\
	}								\
    }									\
  while (0)

__thread int v1;
static __thread int v2;

// We don't use these pointers, but putting them in tests alignment on
// a 64-bit target.
__thread char* p1;
char dummy;
__thread char* p2 = &dummy;

__thread int v3 = 3;
static __thread int v4 = 4;
__thread int v5;
static __thread int v6;

struct int128
{
  long long hi;
  long long lo;
};

static __thread struct int128 v12 = { 115, 125 };

bool
t1()
{
  CHECK_EQ_OR_RETURN(v1, 0);
  v1 = 10;
  return true;
}

bool
t2()
{
  CHECK_EQ_OR_RETURN(v2, 0);
  v2 = 20;
  return true;
}

bool
t3()
{
  CHECK_EQ_OR_RETURN(v3, 3);
  v3 = 30;
  return true;
}

bool
t4()
{
  CHECK_EQ_OR_RETURN(v4, 4);
  v4 = 40;
  return true;
}

// For test 5 the main function calls f5b(f5a()), then calls t5().

int*
f5a()
{
  return &v5;
}

void
f5b(int* p)
{
  *p = 50;
}

bool
t5()
{
  CHECK_EQ_OR_RETURN(v5, 50);
  return true;
}

// For test 6 the main function calls f6b(f6a()), then calls t6().

int*
f6a()
{
  return &v6;
}

void
f6b(int* p)
{
  *p = 60;
}

bool
t6()
{
  CHECK_EQ_OR_RETURN(v6, 60);
  return true;
}

// The slot for t7() is unused.

bool
t8()
{
  CHECK_EQ_OR_RETURN(o1, 0);
  o1 = -10;
  return true;
}

bool
t9()
{
  CHECK_EQ_OR_RETURN(o2, -2);
  o2 = -20;
  return true;
}

// For test 10 the main function calls f10b(f10a()), then calls t10().

int*
f10a()
{
  return &o3;
}

void
f10b(int* p)
{
  *p = -30;
}

bool
t10()
{
  CHECK_EQ_OR_RETURN(o3, -30);
  return true;
}

bool
t12()
{
  struct int128 newval = { 335, 345 };
  CHECK_EQ_OR_RETURN((int) v12.hi, 115);
  CHECK_EQ_OR_RETURN((int) v12.lo, 125);
  v12 = newval;
  return true;
}

bool
t_last()
{
  CHECK_EQ_OR_RETURN(v1, 10);
  CHECK_EQ_OR_RETURN(v2, 20);
  CHECK_EQ_OR_RETURN(v3, 30);
  CHECK_EQ_OR_RETURN(v4, 40);
  CHECK_EQ_OR_RETURN(v5, 50);
  CHECK_EQ_OR_RETURN(v6, 60);
  CHECK_EQ_OR_RETURN((int) v12.hi, 335);
  CHECK_EQ_OR_RETURN((int) v12.lo, 345);
  CHECK_EQ_OR_RETURN(o1, -10);
  CHECK_EQ_OR_RETURN(o2, -20);
  CHECK_EQ_OR_RETURN(o3, -30);
  int check = t11_last();
  CHECK_EQ_OR_RETURN(check, 1);
  return true;
}
