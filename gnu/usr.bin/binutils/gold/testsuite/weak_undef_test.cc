// weak_undef_test.cc -- test handling of weak undefined symbols for gold

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

// We test that we correctly deal with weak undefined symbols.
// We need to make sure that a weak undefined symbol in the main
// program is resolved to zero by the linker and that no dynamic
// relocation is generated.  We also make sure that a weak undefined
// symbol in a shared library can resolve to a symbol in the main
// program.

// This file will be linked with a shared library that does not
// define the symbol, so that the symbol remains undefined.
// Through the use of LD_LIBRARY_PATH, the program will load
// an alternate shared library that does define the symbol,
// so that we can detect whether the symbol was left for runtime
// resolution.

// Similarly, this file will be linked with a shared library that
// does define a different symbol, and loaded with an alternate
// shared library that does not define that symbol.  We check that
// the weak reference remains weak, and that it is resolved to
// zero at runtime.


#include <cstdio>
#include "weak_undef.h"

extern int no_such_symbol_ __attribute__ ((weak));

extern int link_time_only __attribute__ ((weak));

int *p1 = &no_such_symbol_;

int *p2 = &link_time_only;

int v2 = 42;

int
main()
{
  int status = 0;
  int v;

  if ((v = t1()) != 2)
    {
      fprintf(stderr, "FAILED weak undef test 1: %s\n",
              "bound to wrong library");
      status = 1;
    }

  if ((v = t2()) != 42)
    {
      fprintf(stderr, "FAILED weak undef test 2: expected %d, got %d\n",
              42, v);
      status = 1;
    }

  if ((v = t3()) != 42)
    {
      fprintf(stderr, "FAILED weak undef test 3: expected %d, got %d\n",
              42, v);
      status = 1;
    }

  if (&no_such_symbol_ != NULL)
    {
      fprintf(stderr, "FAILED weak undef test 4: %s\n",
              "&no_such_symbol_ is not NULL");
      status = 1;
    }

  if (p1 != NULL)
    {
      fprintf(stderr, "FAILED weak undef test 5: %s\n",
              "p1 is not NULL");
      status = 1;
    }

  if (p2 != NULL)
    {
      fprintf(stderr, "FAILED weak undef test 6: %s\n",
              "p2 is not NULL");
      status = 1;
    }

  return status;
}
