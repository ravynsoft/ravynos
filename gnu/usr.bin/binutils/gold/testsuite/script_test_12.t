/* script_test_12.t -- linker script test 12 for gold

   Copyright (C) 2008-2023 Free Software Foundation, Inc.
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
   MA 02110-1301, USA.  */

interleaved = 0;

SECTIONS
{
  . = 0x400000 + SIZEOF_HEADERS;

  .interp : { *(.interp) }
  .note : { *(.note .note.*) }
  .rel.dyn : { *(.rel.dyn) }
  .rela.dyn : { *(.rela.dyn) }
  .rel.plt : { *(.rel.plt) }
  .rela.plt : { *(.rela.plt) }
  .init : { *(.init) }
  .text : { *(.text) }
  .fini : { *(.fini) }
  .rodata : { *(.rodata .rodata.*) }
  .eh_frame_hdr : { *(.eh_frame_hdr) }
  .eh_frame : { *(.eh_frame) }

  . = DATA_SEGMENT_ALIGN(0x10000, 0x10000);

  .init_array : {
    __init_array_start = .;
    *(.init_array);
    __init_array_end = .;
  }
  .fini_array : { *(.fini_array) }
  .jcr : { *(.jcr) }
  .dynamic : { *(.dynamic) }
  .got : { *(.got) }
  .got.plt : { *(.got.plt) }
  .data : { *(.data) }
  .test : {
    test_array_start = .;
    *(.x1 .x2 .x3);
    test_array_end = .;
    *(.x4);
    }
  .bss : { *(.bss) }

}
