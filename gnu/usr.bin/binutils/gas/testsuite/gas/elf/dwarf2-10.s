/* Test view numbering zero-assert checking with zero-sized align.

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

	.file "dwarf2-10.c"
	.text
	.balign 8
	.globl _start
_start:
	.file 1 "dwarf2-10.c"
	.loc 1 1 view 0
	.balign 8	/* No skip needed here...  */
	.loc 1 2 view 0	/* so this zero-view check fails.  */
	.quad 0
	.loc 1 3 view 0
	.balign 16	/* Skip 8 more bytes after .quad...  */
	.loc 1 4 view 0	/* so this is a zero view indeed.  */
	.quad 0
	.size _start, .-_start
