/* advsisd-copy.s Test file for AArch64 Advanced-SISD copy instructions.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

	.macro	element2scalar	op, type, index
	\op	\type\()31, V7.\type[\index]
	.endm

	.macro	iterate	op, type, from, to
	element2scalar	\op, \type, \from
	.if \to-\from
	iterate	\op, \type, "(\from+1)", \to
	.endif
	.endm

.text
	.irp	op, dup, mov
	iterate	\op, b, 0, 15
	iterate	\op, h, 0, 7
	iterate	\op, s, 0, 3
	iterate	\op, d, 0, 1
	.endr
