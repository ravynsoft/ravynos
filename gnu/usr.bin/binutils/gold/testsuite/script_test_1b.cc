// script_test_1b.cc -- linker script test 1 for gold  -*- C++ -*-

// Copyright (C) 2015-2023 Free Software Foundation, Inc.

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

// A test for a linker script which sets symbols to values.

#include <cassert>

#include "script_test_1.h"

void
check_int(intptr_t i, intptr_t j)
{
  assert (i == j);
}

void
check_ptr(int *i, int *j)
{
  assert (i == j);
}
