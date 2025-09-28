/* Test view numbering.

   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

	.file "dwarf2-5.c"
	.text
	.balign 8
	.globl _start
_start:
	.file 1 "dwarf2-5.c"
	.loc 1 1 view 0
	.loc 1 2 view .L2
	.quad 0
	.loc 1 3 view 0
	.balign 8
	.loc 1 4 view .L4
	.loc 1 5 view .L5
	.org .+1
	.balign 8
	.loc 1 6 view 0
	.quad 0
	.text
	.globl func
	.type func, %function
func:
	.loc 1 7 view 0
	.loc 1 8 view .L8
	.quad 0
	.loc 1 9 view 0
	.loc 1 10 view .L10
	.pushsection .text
	.loc 1 11 view .L11
	.popsection
	.loc 1 12 view .L12
	.quad 0
	.size func, .-func

	.section .rodata
	.uleb128 .L2
	.uleb128 .L4
	.uleb128 .L5
	.uleb128 .L8
	.uleb128 .L10
	.uleb128 .L11
	.uleb128 .L12
