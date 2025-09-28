// incr_comdat_test_1.cc -- test incremental update with comdat sections

// Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

#include <cstdio>

template <class T>
T GetMax(T a, T b)
{
  return a > b ? a : b;
}

extern int foo();

int bar()
{
  return GetMax<int>(4, 5);
}

class A
{
 public:
  int sum(int k)
  { 
    static int total = 0;
    total += k;
    return total;
  }
};

#define CHECK_EQ(var, expected)						\
  do									\
    {									\
      if ((var) != (expected))						\
	{								\
	  printf(#var ": expected %d, found %d\n", expected, var);	\
	  return 1;							\
	}								\
    }									\
  while (0)

int main()
{
  A a;
  CHECK_EQ(bar(), 5);
  CHECK_EQ(foo(), 11);
  CHECK_EQ(a.sum(55), 11 + 55);
  CHECK_EQ(a.sum(66), 11 + 55 + 66);
  return 0;
}
