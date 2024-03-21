// justsyms_1.cc -- test --just-symbols for gold

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

// The Linux kernel builds an object file using a linker script, and
// then links against that object file using the -R option.  This is a
// test for that usage.

#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <stdint.h>

extern char justsyms_string[];

// We expect to get a SIGSEGV.
static void
handle_sigsegv(int)
{
  exit(0);
}

int
main(int, char**)
{
  // The linker script should arrange for this symbol to be exactly at
  // address 0x10000.
  assert(reinterpret_cast<uintptr_t>(justsyms_string) == 0x100);

  // However, since the file was linked with --just-symbols, we should
  // not be able to actually access the symbol.
  signal(SIGSEGV, handle_sigsegv);
  char c = justsyms_string[0];
  exit(c == '\0' ? 1 : c);
}
