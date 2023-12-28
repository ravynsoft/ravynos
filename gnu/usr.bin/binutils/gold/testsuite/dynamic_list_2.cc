// dynamic_list_test_2.cc -- Test --dynamic-list with shared libraries.

// Copyright (C) 2014-2023 Free Software Foundation, Inc.
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

// The goal of this program is to verify that the --dynamic-list option
// allows overrides for symbols listed in the file, and does symbolic
// binding for symbols not listed.

#include <cstring>

extern const char* test_foo();
extern const char* test_bar();

int
main(void)
{
  if (strcmp(test_foo(), "override") != 0)
    return 1;
  if (strcmp(test_bar(), "original") != 0)
    return 2;
  return 0;
}
