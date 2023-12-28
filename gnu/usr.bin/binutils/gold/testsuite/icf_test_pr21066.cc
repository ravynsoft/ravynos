// icf_test.cc -- a test case for gold

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Test case from PR 21066 submitted by Gandhi Shaheen

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

// The goal of this program is to verify if identical code folding
// correctly identifies and folds functions.  folded_func must be
// folded into kept_func.

// Written by Sriraman Tallam <tmsriram@google.com>.

#include <stdio.h>

struct first_exception {
};

struct second_exception {
};

typedef void (*callback_fn_t)();

void raise_first_exception()
{
  throw first_exception();
}

void raise_second_exception()
{
  throw second_exception();
}

template<typename E>
void capture_exception_of_type(volatile callback_fn_t f)
{
  try {
    f();
  } catch (E& e) {
    puts("caught expected exception");
  } catch (...) {
    puts("ERROR: caught unexpected exception");
    throw;
  }
}

int main()
{
  capture_exception_of_type<first_exception>(raise_first_exception);
  capture_exception_of_type<second_exception>(raise_second_exception);
  return 0;
}
