/* relro_test.t -- relro script test for gold

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

using_script = 1;

SECTIONS
{
  . = SIZEOF_HEADERS;

  .text : { *(.text) }

  .eh_frame : ONLY_IF_RO { KEEP(*(.eh_frame)) }

  . = (ALIGN(CONSTANT(MAXPAGESIZE))
       - ((CONSTANT(MAXPAGESIZE) - .) & (CONSTANT(MAXPAGESIZE) - 1)));
  . = DATA_SEGMENT_ALIGN(CONSTANT(MAXPAGESIZE), CONSTANT(COMMONPAGESIZE));

  .eh_frame : ONLY_IF_RW { KEEP(*(.eh_frame)) }
  .tdata : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*)
		   *(.data.rel.ro* .gnu.linkonce.d.rel.ro.*) }
  .dynamic : { *(.dynamic) }
  .got : { *(.got) }

  . = DATA_SEGMENT_RELRO_END(0, .);

  .got.plt : { *(.got.plt) }

  .data : { *(.data .data.* .gnu.linkonce.d.*) }

  . = DATA_SEGMENT_END (.);
}
