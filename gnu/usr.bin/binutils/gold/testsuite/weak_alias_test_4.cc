// weak_alias_test_4.cc -- test weak aliases for gold

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

// Verify that all the symbols look right from a shared library.

extern int weak_symbol;
extern int strong_aliased;
extern int weak_aliased;
extern int strong_aliased_2;
extern int weak_aliased_2;
extern int strong_aliased_3;
extern int weak_aliased_3;
extern int strong_aliased_4;
extern int weak_aliased_4;
extern int strong_symbol;

bool
t1()
{
  // Should come from weak_alias_test_3.cc.
  if (weak_symbol != 4)
    return false;

  // Should come from weak_alias_test_main.cc.
  if (strong_aliased != 3)
    return false;

  // weak_aliased need not match strong_aliased, which is overridden
  // by weak_test_main.cc.

  // Should come from weak_alias_test_main.cc.
  if (weak_aliased_2 != 6)
    return false;

  // strong_aliased_2 need not match weak_aliased_2, which is
  // overidden by weak_test_main.cc.

  // The others should match.
  if (weak_aliased_3 != 7 || strong_aliased_3 != 7)
    return false;
  if (weak_aliased_4 != 8 || strong_aliased_4 != 8)
    return false;

  // Should come from weak_alias_test_2.cc.
  if (strong_symbol != 2)
    return false;

  return true;
}
