// incr_comdat_test_2.cc -- test incremental update with comdat sections

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

template <class T>
T GetMax(T a, T b)
{
  return a > b ? a : b;
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

int foo()
{
  A a;
  return GetMax<int>(10, a.sum(11));
}
