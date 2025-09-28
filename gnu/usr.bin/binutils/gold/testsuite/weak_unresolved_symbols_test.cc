// weak_unresolved_symbols_test.cc -- a test case for gold

// Copyright (C) 2015-2023 Free Software Foundation, Inc.
// Written by Sriraman Tallam <tmsriram@google.com>.

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

// Test --weak-unresolved-symbols.  Symbol foo remains unresolved but
// with -fPIE, needs a GOT entry and has a dynsym entry and a dynamic
// relocation against it created.  This  will fail to link and run without
// --weak-unresolved-symbols.  With --warn-unresolved-symbols, it will link
// but the dynamic linker will complain that foo(_Z3foov) is unresolved.

extern int foo();

int bar() {
  return 0;
}

int (*p)() = &bar;

int main() {
  if (p == &foo)
    {
      foo();
    }
  else
    (*p)();
  return 0;
}
