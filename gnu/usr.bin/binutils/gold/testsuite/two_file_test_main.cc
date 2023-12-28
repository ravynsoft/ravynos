// two_file_test_main.cc -- a two file test case for gold, main function

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

// This tests references between files.  This is the main file.  See
// two_file_test_1.cc for details.

#include <cassert>

#include "two_file_test.h"

int
main()
{
  // Initialize common data.
  v3 = 789;
  for (int i = 0; i < 13; ++i)
    v5[i] = v4[i];

  assert(t1());
  assert(t1a());
  assert(t2());
  assert(t3());
  assert(t4());
  assert(t5());
  assert(t6());
  assert(t7());
  assert(t8());
  assert(t9());
  assert(t10());
  assert(t11());
  assert(t12());
  assert(t13());
  assert(t16());
  assert(t16a());
  assert(t17());
  assert(t18());
  return 0;
}
