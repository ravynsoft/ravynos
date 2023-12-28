/* script_test_12a.c -- a test case for gold

   Copyright (C) 2015-2023 Free Software Foundation, Inc.
   Written by Cary Coutant <ccoutant@gmail.com>.

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

   This tests linker script behavior, that gold distinguishes correctly
   between
      *(.x1) *(.x2) *(.x3)
   and
      *(.x1 .x2 .x3)
   in an input section spec. In the first case, the output section
   should contain all .x1 sections, followed by all .x2 sections,
   then all .x3 sections; i.e.:
      script_test_12a.o(.x1)
      script_test_12b.o(.x1)
      script_test_12a.o(.x2)
      script_test_12b.o(.x2)
      script_test_12a.o(.x3)
      script_test_12b.o(.x3)
   
   In the second case, the output section should interleave the
   .x1, .x2, and .x3 sections in the order seen; i.e.:
      script_test_12a.o(.x1)
      script_test_12a.o(.x2)
      script_test_12a.o(.x3)
      script_test_12b.o(.x1)
      script_test_12b.o(.x2)
      script_test_12b.o(.x3)

   The linker scripts set the absolute symbol INTERLEAVED, which we
   test here to determine which ordering to expect. The scripts also
   define the symbols test_array_start and test_array_end.
*/

extern int test_array_start;
extern int test_array_end;
extern char interleaved __attribute__((__aligned__(1)));

int
main(void)
{
  int last = 0;
  int *p;
  long should_be_interleaved = (long)&interleaved;
  int mask = (should_be_interleaved == 1 ? 0x7f : 0xff);
  for (p = &test_array_start; p < &test_array_end; ++p)
    {
      int next = *p & mask;
      if (next <= last)
	return 1;
      last = next;
    }
  return 0;
}

int a1[] __attribute((section(".x1"))) = { 0x01, 0x02, 0x03, 0x04 };
int a2[] __attribute((section(".x2"))) = { 0x11, 0x12, 0x13, 0x14};
int a3[] __attribute((section(".x3"))) = { 0x21, 0x22, 0x23, 0x24 };
int a4[] __attribute((section(".x4"))) = { 0xff, 0xff, 0xff, 0xff };
