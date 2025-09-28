/* arm_thm_jump8.t -- linker script to test R_ARM_THM_JUMP8 relocation.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by Doug Kwan <dougkwan@google.com>.

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

SECTIONS
{
  . = 0x8000;

  .text.pre : { *(.text.pre) }
  . = ALIGN(0x100);
  .text : { *(.text) }
  . = ALIGN(0x100);
  .text.post : { *(.text.post) }
  . += 0x1000;
  .data : { *(.data) }
  .bss : { *(.bss) }
  .ARM.attributes : { *(.ARM.attributes) }
}
