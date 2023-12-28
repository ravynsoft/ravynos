// weak_alias_test_main.cc -- test weak aliases for gold

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

#include <assert.h>

// Defined in both weak_alias_test_1.cc and weak_alias_test_2.cc, but
// we should get the one in weak_alias_test_1.cc.
extern int weak_symbol;

// Defined in weak_alias_test_2.cc.
extern int strong_symbol;

// weak_aliased is an alias for this.
int strong_aliased = 3;

// Defined as a weak alias in weak_alias_test_1.cc.
int weak_aliased_2 = 6;

// Defined in weak_alias_test_1.cc
extern int strong_aliased_3;
extern int weak_aliased_4;

// Defined in weak_alias_test_5.cc
extern int versioned_symbol;
extern int versioned_alias;

extern bool t1();
extern bool t2();

int
main()
{
  // weak_symbol should come from weak_alias_test_3.cc.
  assert(weak_symbol == 4);

  // strong_symbol should come from weak_alias_test_2.cc.
  assert(strong_symbol == 2);

  // strong_aliased should come from this file, above.
  assert(strong_aliased == 3);

  // weak_aliased_2 should come from this file, above.
  assert(weak_aliased_2 == 6);

  // strong_aliased_3 should come from weak_alias_test_1.cc.
  assert(strong_aliased_3 == 7);

  // weak_aliased_4 should come from weak_alias_test_1.cc.
  assert(weak_aliased_4 == 8);

  // Make sure the symbols look right from a shared library.
  assert(t1());

  // versioned_symbol comes from weak_alias_test_5.cc.
  assert(versioned_symbol == 1);
  // So does versioned_alias.
  assert(versioned_alias == 1);

  // Make sure the versioned symbols look right from a shared library.
  assert(t2());
}
