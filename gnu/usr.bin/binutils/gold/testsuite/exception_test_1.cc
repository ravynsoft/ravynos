// exception_test_1.cc -- test exception handling for gold, file 1 of 2

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

// This tests throwing an exception across various boundaries.  This
// is a general test of the exception frame handling, and the
// interaction with the compiler support libraries.  This is file 1,
// which catches the exception.  We test in several different ways:

// Files 1 and 2 linked together in executable.
// File 1 in executable, file 2 in shared library.
// File 1 in shared library, file 2 in executable.
// Files 1 and 2 linked together in shared library.
// Files 1 and 2 in different shared libraries.

#include "exception_test.h"

bool
t1()
{
  int i;
  try
    {
      i = 0;
      f1();
      i = 1;
    }
  catch (...)
    {
      return i == 0;
    }

  return false;
}
