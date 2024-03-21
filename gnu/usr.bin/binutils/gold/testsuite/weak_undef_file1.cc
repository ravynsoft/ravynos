// weak_undef_file1.cc -- test handling of weak undefined symbols for gold

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
// We need to make sure that the symbol is resolved to zero
// by the linker and that no dynamic relocation is generated.

// This source is used to build a shared library that defines
// an arbitrary symbol other than the weak undefined symbol
// referenced by the main program.  The main program will be
// linked with this library so that the symbol remains undefined.
// Through the use of the embedded RPATH, the program will load
// an alternate shared library that does define the symbol,
// so that we can detect whether the symbol was left for runtime
// resolution.

#include <cstdio>

#include "weak_undef.h"

int is_such_symbol_ = 1;

// We also define a symbol that is not defined by the alternate
// library.  The main program contains a weak reference to this
// symbol, and tests that the reference remains weak even after
// the definition was found at link time.

int link_time_only = 1;

extern int v2 __attribute__ ((weak));

int *v3 = &v2;

int
t1()
{
  return is_such_symbol_;
}

// Test that a weak reference from a shared library to a symbol
// defined in the main program does get resolved.

int
t2()
{
  return (&v2 == NULL) ? -1 : v2;
}

// Test that a weak reference from a shared library to a symbol
// defined in the main program does get resolved.

int
t3()
{
  return (v3 == NULL) ? -1 : *v3;
}
