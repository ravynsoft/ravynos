// debug_msg.cc -- a test case for printing debug info for missing symbols.

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

// This file is constructed to have undefined references.  In
// debug_msg.sh, we will try to link this file, and analyze the
// error messages that are produced.

extern int undef_int;
extern float undef_float;
extern void undef_fn1();
extern void undef_fn2();

int* badref1 = &undef_int;
static float* badref2 = &undef_float;
void (*fn_array[])() =
{
  undef_fn1,
  undef_fn2
};

template<class Foo>
int testfn(Foo x)
{
  undef_fn1();
  undef_fn2();
  return undef_int;
}

class Base
{
  virtual void virtfn() { undef_fn1(); }
};

class Derived : public Base
{
  virtual void virtfn() { undef_fn2(); }
};

// This tests One Definition Rule (ODR) violations.
void SortAscending(int array[], int size);   // in odr_violation1.cc
void SortDescending(int array[], int size);  // in odr_violation2.cc
// This tests One Definition Rule (ODR) non-violations.
#include "odr_header2.h"
OdrBase* CreateOdrDerived1();  // in odr_violation1.cc
OdrBase* CreateOdrDerived2();  // in odr_violation2.cc

extern "C" int OverriddenCFunction(int i);  // in odr_violation*.cc

inline int SometimesInlineFunction(int i) {  // strong in odr_violation2.cc.
  return i * i * 3;
}


int main()
{
  testfn(5);
  testfn(4.0);

  Base b;
  Derived d;

  int kInput1[] = {1, 6, 9, 7, 3, 4, 2, 10, 5, 8};
  int kSize1 = sizeof(kInput1) / sizeof(int);
  SortAscending(kInput1, kSize1);

  int kInput2[] = {1, 6, 9, 7, 3, 4, 2, 10, 5, 8};
  int kSize2 = sizeof(kInput2) / sizeof(int);
  SortDescending(kInput2, kSize2);

  OverriddenCFunction(3);
  SometimesInlineFunction(3);

  delete CreateOdrDerived1();
  delete CreateOdrDerived2();

  return 0;
}
