// dwp_test_main.cc -- a test case for dwp

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

// Adapted from two_file_test_main.cc.

#include <cassert>

#include "dwp_test.h"

int
main()
{
  C1 c1;
  C2 c2;

  // Initialize common data.
  v3 = 789;
  for (int i = 0; i < 13; ++i)
    v5[i] = v4[i];

  assert(c1.testcase1());
  assert(c1.t1a());
  assert(c1.testcase2());
  assert(c1.testcase3());
  assert(c1.testcase4());
  assert(c2.testcase1());
  assert(c2.testcase2());
  assert(c2.testcase3());
  assert(c2.testcase4());
  assert(c3.testcase1());
  assert(c3.testcase2());
  assert(c3.testcase3());
  assert(t12());
  assert(t13());
  assert(t16());
  assert(t16a());
  assert(t17());
  assert(t18());
  return 0;
}
