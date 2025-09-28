/* advsimd-across.s Test file for AArch64 Advanced-SIMD across
   instructions.

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

	.macro asimdall	op, V, T
	\op	\V\()7, v31.\()\T
	.endm

.text
	.irp	op, saddlv, uaddlv
	asimdall	\op, h, 8b
	asimdall	\op, h, 16b
	asimdall	\op, s, 4h
	asimdall	\op, s, 8h
	asimdall	\op, d, 4s
	.endr

	.irp	op, smaxv, umaxv, sminv, uminv, addv
	asimdall	\op, b, 8b
	asimdall	\op, b, 16b
	asimdall	\op, h, 4h
	asimdall	\op, h, 8h
	asimdall	\op, s, 4s
	.endr

	.irp	op, fmaxnmv, fminnmv, fmaxv, fminv
	asimdall	\op, s, 4s
	.endr
