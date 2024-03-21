/* Copyright (C) 2021-2023 Free Software Foundation, Inc.

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

        .text
	/* The following instruction is in the area set aside for
	   custom instruction extensions.  As such it is unlikely that
           an upstream extension should ever clash with this.  */
	.insn r 0x0b, 0x0, 0x0, x3, x4, x5
        /* Unlike the above, the following is just a reserved
	   instruction encoding.  This means that in the future an
	   extension to the compressed instruction set might use this
	   encoding.  If/when that happens we'll need to find a
	   different unused encoding within the compressed instruction
	   space.  */
	.insn ca 0x1, 0x27, 0x2, x8, x9
