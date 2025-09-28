// binary_test.cc -- test --format binary for gold

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

// This program is linked with a small text file named binary.txt
// using --formatbinary.

#include <cassert>
#include <cstddef>
#include <cstring>
#include <stdint.h>

extern char _binary_binary_txt_start[] __attribute__((__aligned__(1)));
extern char _binary_binary_txt_end[] __attribute__((__aligned__(1)));
extern char _binary_binary_txt_size[] __attribute__((__aligned__(1)));

int
main(int, char**)
{
  int size = reinterpret_cast<uintptr_t>(_binary_binary_txt_size);
  assert(size == _binary_binary_txt_end - _binary_binary_txt_start);

  const char* const txt = "This file is used for the binary test.\n";
  assert(strncmp(txt, _binary_binary_txt_start, size) == 0);
  assert(static_cast<size_t>(size) == strlen(txt));

  return 0;
}
