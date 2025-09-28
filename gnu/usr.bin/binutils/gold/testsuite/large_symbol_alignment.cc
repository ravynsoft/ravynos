// large_symbol_alignment.cc -- a test case for gold

// Copyright (C) 2013-2023 Free Software Foundation, Inc.
// Written by Alexander Ivchenko <alexander.ivchenko@intel.com>.

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

// The goal of this program is to verify that Gold correctly aligns
// the symbol with a large alignemnt (often larger than the page size).

#include <stdint.h>

__attribute__ ((aligned(8192)))  int aligned_8k_var;
__attribute__ ((aligned(4096)))  int aligned_4k_var;
__attribute__ ((aligned(16384))) int aligned_16k_var;

bool
is_aligned(const int& var, int align) __attribute__((noinline));

bool
is_aligned(const int& var, int align)
{
  return (reinterpret_cast<uintptr_t>(&var) & (align - 1)) == 0;
}

int
main()
{
  if (!is_aligned(aligned_16k_var, 16384)
      || !is_aligned(aligned_8k_var, 8192)
      || !is_aligned(aligned_4k_var, 4096))
    return 1;
  return 0;
}
