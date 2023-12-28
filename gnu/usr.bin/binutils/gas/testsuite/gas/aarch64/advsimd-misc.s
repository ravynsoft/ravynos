/* advsimd-abs.s Test file for AArch64 Advanced-SIMD Integer absolute
   instruction.

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

	.macro asimdabs	op, T
	\op	v0.\()\T, v31.\()\T
	.endm

	.text
	.irp	op, abs, neg, sqabs, sqneg
	.irp	type, 8b, 16b, 4h, 8h, 2s, 4s, 2d
	asimdabs	\op \type
	.endr
	.endr
