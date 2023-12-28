/* illegal-lse.s Test file For AArch64 LSE atomic instructions that
   could be rejected by the assembler.

   Copyright (C) 2014-2023 Free Software Foundation, Inc.
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


	.macro format_0_check op
	.irp suffix, , a, l, al, b, h, ab, lb, alb, ah, lh, alh
		\op\suffix w0, x1, [x2]
		\op\suffix w2, w3, [w4]
	.endr
	.irp suffix, , a, l, al
		\op\suffix w0, x1, [x2]
		\op\suffix x2, x3, [w4]
	.endr
	.endm

	.macro format_0_no_rt_no_acquire_check op
	.irp suffix, , l, b, h, lb, lh
		\op\suffix x0, [x2]
		\op\suffix w2, [w3]
	.endr
	.irp suffix, , l
		\op\suffix x0, [w3]
	.endr
	.endm

	.macro format_1_check op
	.irp suffix, , a, l, al
		\op\suffix w1, w1, w2, w3, [x5]
		\op\suffix w4, w4, w6, w7, [sp]
		\op\suffix w0, x1, x2, x3, [x2]
		\op\suffix x4, x5, x6, x7, [w8]
	.endr
	.endm

	.macro format_2_check op
	.irp suffix, add, clr, eor, set, smax, smin, umax, umin
		format_0_check \op\suffix
	.endr
	.endm

	.macro format_3_check op
	.irp suffix, add, clr, eor, set, smax, smin, umax, umin
		format_0_no_rt_no_acquire_check \op\suffix
	.endr
	.endm

	.text
func:
	format_0_check cas
	format_0_check swp
	format_1_check casp
	format_2_check ld
	format_3_check st
