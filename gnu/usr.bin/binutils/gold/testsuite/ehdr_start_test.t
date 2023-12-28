/* ehdr_start_test.t -- __ehdr_start test for gold

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

/* With luck this will work on all platforms.  */

SECTIONS
{
  /* Set the text segment to start on a non-page boundary.  */
  . = 0x10000040;

  .text : { *(.text) }
  . += 0x100000;
  . = ALIGN(0x100);

  .tdata : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*)
		   *(.data.rel.ro* .gnu.linkonce.d.rel.ro.*) }
  .dynamic : { *(.dynamic) }
  .got : { *(.got) }
  .got.plt : { *(.got.plt) }
  .data : { *(.data .data.* .gnu.linkonce.d.*) }
}
