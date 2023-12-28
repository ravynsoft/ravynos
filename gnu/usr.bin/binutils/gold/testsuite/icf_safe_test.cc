// icf_safe_test.cc -- a test case for gold

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Sriraman Tallam <tmsriram@google.com>.

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

// The goal of this program is to verify if identical code folding
// in safe mode correctly folds only ctors and dtors. kept_func_1 must
// not be folded into kept_func_2 other than for X86 (32 and 64 bit)
// which can use relocation types to determine if function pointers are
// taken.  kept_func_3 should never be folded as its pointer is taken.
// The ctor and dtor of class A must be folded.

class A
{
  public:
    A()
    {
    }
    ~A()
    {
    }
};

A a;

int kept_func_1()
{
  return 1;
}

int kept_func_2()
{
  return 1;
}

int kept_func_3()
{
  return 1;
}

int main()
{
  int (*p)() = kept_func_3;
  p();
  return 0;
}
