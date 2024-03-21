/* discard_locals_relocatable_test.c -- test --discard-locals/--discard-all -r

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Viktor Kutuzov <vkutuzov@accesssoftek.com>.

   This file is part of gold.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.

   This is a test of a common symbol in the main program and a
   versioned symbol in a shared library.  The common symbol in the
   main program should override the shared library symbol.  */

/* Note: use GCC -fPIC option to compile this test.  */

/* Local symbol format for generic ELF target. 
   Use GCC -Wa,-L option to preserve this local symbol
   in the output object file. */
__asm__ (".Lshould_be_discarded:");

#ifdef __powerpc__
/* Test wants to keep one local.  Satisfy it.  */
#ifdef __powerpc64__
__asm__ (".reloc 0,R_PPC64_NONE,.LC0");
#else
__asm__ (".reloc 0,R_PPC_NONE,.LC0");
#endif
#endif

extern void print_func (const char* s);

extern int func (void);

int
func (void)
{
  print_func ("local string");
  return 0;
}
