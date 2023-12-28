// two_file_test_1b_v1.cc -- supplementary file for a three-file test case
// for gold.

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
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

// This is an alternate version of the source file two_file_test_1b.cc,
// used to test incremental linking.  We build a binary first using this
// source file, then do an incremental link with the primary version of
// the file.

// This file is used as part of a mixed PIC/non-PIC test.
// Files 1 and 1b are linked together in a shared library.
// File 1 is compiled non-PIC, and file 1a is compiled PIC.
// File 2 is compiled PIC and linked in a second shared library.
// This tests that a non-PIC call does not accidentally get
// bound to a PIC PLT entry.

// We test the following cases.

#include "two_file_test.h"

// 16 Call a function directly after its address has been taken.

bool
t16a()
{
  return f10() == 125;
}
