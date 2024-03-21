/* script_test_2.t -- linker script test 2 for gold

   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <iant@google.com>.

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
   MA 02110-1301, USA.  */

test_addr_alias = test_addr;

SECTIONS
{
  /* With luck this will work everywhere.  */
  . = 0x10000000;

  /* With luck this will be enough to get the program working.  */
  .text : { *(.text) }
  . += 0x100000;
  . = ALIGN(0x100);
  .data : { *(.data) }
  .got : { *(.got .toc) }
  .got.plt : { *(.got.plt) }
  .bss : { *(.bss) }

  /* Now the real test.  */
  . = 0x20000001;
  start_test_area = .;
  .gold_test ALIGN(16) : SUBALIGN(32) {
    start_test_area_1 = .;

    /* No sections should wind up here, because of the EXCLUDE_FILE.  */
    *( EXCLUDE_FILE(script_test*) .gold_test)

    /* This should match only script_test_2b.o.  */
    script_test_2b.o(.gold_test)

    /* This should match the remaining sections.  */
    *(.gold_test)

    . = 60;
    start_data = .;
    BYTE(1)
    SHORT(2)
    LONG(4)
    QUAD(8)
    end_data = .;

    start_fill = .;
    FILL(0x12345678);
    . = . + 7;
    BYTE(0)
    end_fill = .;
  }
  end_test_area = .;
  test_addr = ADDR(.gold_test);
}
