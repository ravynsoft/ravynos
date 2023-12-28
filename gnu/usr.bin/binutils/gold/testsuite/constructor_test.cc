// constructor_test.cc -- a test case for gold global constructors

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

// This file just runs some global constructors and destructors.  The
// last global destructor verifies that the state is as expected, and
// we assume that it runs correctly itself.

#include <cassert>
#include <cstdlib>

// These counters let us verify the state.

int c1_count;
int c2_count;
int atexit_count;

// This class verifies that there are no objects left when it is
// destroyed.  Therefore, we can only have one instance of this
// object.

class c1
{
 public:
  static int count;

  c1()
  { ++c1_count; }

  ~c1()
  {
    --c1_count;
    assert(c1_count == 0 && c2_count == 0 && atexit_count == 0);
  }
};

c1 c1v;

// A function called at atexit time.

void
atexit_function()
{
  --atexit_count;
  assert(atexit_count == c2_count);
}

// A class which counts itself and also calls atexit.

class c2
{
 public:
  c2()
  {
    assert(atexit_count == c2_count);
    ++c2_count;
    atexit(atexit_function);
    ++atexit_count;
  }

  ~c2()
  { --c2_count; }
};

c2 c2v1;
c2 c2v2;

int
main()
{
  return 0;
}
