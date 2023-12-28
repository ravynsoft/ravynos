// script_test_2.cc -- linker script test 2 for gold  -*- C++ -*-

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

// A test of some uses of the SECTIONS clause.  Look at
// script_test_2.t to make sense of this test.

#include <cassert>
#include <cstddef>
#include <cstring>
#include <stdint.h>

extern char start_test_area[] __attribute__((__aligned__(1)));
extern char start_test_area_1[] __attribute__((__aligned__(1)));
extern char start_data[] __attribute__((__aligned__(1)));
extern char end_data[] __attribute__((__aligned__(1)));
extern char start_fill[] __attribute__((__aligned__(1)));
extern char end_fill[] __attribute__((__aligned__(1)));
extern char end_test_area[] __attribute__((__aligned__(1)));
extern char test_addr[] __attribute__((__aligned__(1)));
extern char test_addr_alias[] __attribute__((__aligned__(1)));

int
main(int, char**)
{
  assert(reinterpret_cast<uintptr_t>(start_test_area) == 0x20000001);
  assert(reinterpret_cast<uintptr_t>(start_test_area_1) == 0x20000020);

  assert(strcmp(start_test_area_1, "test bb") == 0);

  // Next the string from script_test_2a.o, after the subalign.
  for (int i = 7; i < 32; ++i)
    assert(start_test_area_1[i] == 0);
  assert(strcmp(start_test_area_1 + 32, "test aa") == 0);

  // Skip to start_data at relative offset 60.
  for (int i = 32 + 7; i < 60; ++i)
    assert(start_test_area_1[i] == 0);
  assert(reinterpret_cast<uintptr_t>(start_test_area_1 + 60)
	 == reinterpret_cast<uintptr_t>(start_data));
  assert(memcmp(start_data, "\1\2\0\4\0\0\0\010\0\0\0\0\0\0\0", 15) == 0
	 || memcmp(start_data, "\1\0\2\0\0\0\4\0\0\0\0\0\0\0\010", 15) == 0);
  assert(end_data == start_data + 15);

  // Check that FILL works as expected.
  assert(&start_fill[0] == &end_data[0]);
  assert(memcmp(start_fill, "\x12\x34\x56\x78\x12\x34\x56\0", 8) == 0);
  assert(end_fill == start_fill + 8);

  assert(&end_test_area[0] == &end_fill[0]);

  assert(&test_addr[0] == &start_test_area_1[0]);
  assert(&test_addr_alias[0] == &test_addr[0]);
}
