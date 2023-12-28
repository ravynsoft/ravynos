// ehdr_start_test.cc -- test for __ehdr_start linker-defined symbol.

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

// The goal of this program is to produce as many different types of
// relocations as we can in a stand-alone program that does not use
// TLS.  This program is compiled without optimization.

#include "config.h"

#include <cassert>
#include <cstdio>

#include "elfcpp.h"

#ifdef EHDR_START_WEAK
#define WEAK_ATTR __attribute__ ((weak))
#else
#define WEAK_ATTR
#endif

extern char __ehdr_start[] WEAK_ATTR;

int
main() {
  printf("&__ehdr_start = %p\n", &__ehdr_start);

#ifdef EHDR_START_UNDEF
  assert(&__ehdr_start == 0);
#else
  assert(&__ehdr_start != NULL);

  printf("ELF header: \\x%02x%c%c%c\n", __ehdr_start[0], __ehdr_start[1],
	 __ehdr_start[2], __ehdr_start[3]);
#ifdef EHDR_START_USER_DEF
  assert(__ehdr_start[0] == 'a'
	 && __ehdr_start[1] == 'b'
	 && __ehdr_start[2] == 'c'
	 && __ehdr_start[3] == 'd');
#else
  assert(__ehdr_start[elfcpp::EI_MAG0] == elfcpp::ELFMAG0
	 && __ehdr_start[elfcpp::EI_MAG1] == elfcpp::ELFMAG1
	 && __ehdr_start[elfcpp::EI_MAG2] == elfcpp::ELFMAG2
	 && __ehdr_start[elfcpp::EI_MAG3] == elfcpp::ELFMAG3);
#endif
#endif

  return 0;
}
