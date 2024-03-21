// many_sections_test.cc -- test lots of sections for gold

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
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

// This program tests having many sections.  It uses a generated .h
// files to define 70,000 variables, each in a different section.  It
// uses another generated .h file to verify that they all have the
// right value.

#include <cassert>

#include "many_sections_define.h"

// This tests a section group.
template<typename T>
class C
{
 public:
  static T val() { return C::val_; }
 private:
  static T val_;
};

template<typename T>
T C<T>::val_;

int
main(int, char**)
{
#include "many_sections_check.h"
  assert(C<int>::val() == 0);
  return 0;
}
