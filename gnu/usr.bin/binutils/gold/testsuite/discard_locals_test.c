/* discard_locals_test.c -- test --discard-locals option.

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Doug Kwan <dougkwan@google.com>.

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

/* Local symbol format for generic ELF target. */
__asm__ (".Lshould_be_discarded:");

#ifdef __i386__
/* Additional local symbol format for the i386 target. */
__asm__ (".Xshould_be_discarded:");
#endif

int
main (void)
{
  return 0;
}
