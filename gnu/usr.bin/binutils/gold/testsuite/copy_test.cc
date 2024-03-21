// copy_test.cc -- test copy relocs for gold

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

#include <cassert>
#include <stdint.h>

// Misalign the BSS section.
static char c;

// From copy_test_1.cc.
extern char b;

// From copy_test_2.cc.
extern long long l;
extern int ip; // protected visibility; may not be copied

int* ipp = &ip;

int
main()
{
  assert(c == 0);
  assert(b == 1);
  assert(l == 2);
  assert((reinterpret_cast<uintptr_t>(&l) & 0x7) == 0);
  assert(*ipp == 3);
  return 0;
}
