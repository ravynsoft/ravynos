// weak_alias_test_5.cc -- test versioned weak aliases for gold

// Copyright (C) 2011-2023 Free Software Foundation, Inc.
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

// Define a versioned symbol.
int versioned_symbol = 1;
__asm__(".symver versioned_symbol,versioned_symbol@@VER1");

// Define a weak alias for the versioned symbol, with a different version.
extern int versioned_alias __attribute__ ((weak, alias("versioned_symbol")));
__asm__(".symver versioned_alias,versioned_alias@@VER2");

bool
t2()
{
  if (versioned_symbol != 1)
    return false;
  if (versioned_alias != 1)
    return false;
  return true;
}
