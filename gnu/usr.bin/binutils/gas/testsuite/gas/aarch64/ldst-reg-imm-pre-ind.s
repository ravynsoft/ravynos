/* ldst-reg-imm-pre-ind.s Test file for AArch64
   load-store reg. (imm.pre-ind.) instructions.

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

	.macro	op2 op, reg, simm
	\op	\reg\()7, [sp, #\simm]!
	.endm

	// load to or store from core register
	.macro ld_or_st op, suffix, reg
	.irp simm, -256, -171, 0, 2, 4, 8, 16, 85, 255
		op2	\op\suffix, \reg, \simm
	.endr
	.endm

	// load to or store from FP/SIMD register
	.macro ld_or_st_v op
	.irp reg, b, h, s, d, q
		.irp simm, -256, -171, 0, 2, 4, 8, 16, 85, 255
			op2	\op, \reg, \simm
		.endr
	.endr
	.endm

func:
	// load to or store from FP/SIMD register
	ld_or_st_v	str
	ld_or_st_v	ldr

	// load to or store from core register
	//      	op, suffix, reg
	ld_or_st	str,  b, w
	ld_or_st	str,  h, w
	ld_or_st	str,   , w
	ld_or_st	str,   , x
	ld_or_st	ldr,  b, w
	ld_or_st	ldr,  h, w
	ld_or_st	ldr,   , w
	ld_or_st	ldr,   , x
	ld_or_st	ldr, sb, x
	ld_or_st	ldr, sh, x
	ld_or_st	ldr, sw, x
	ld_or_st	ldr, sb, w
	ld_or_st	ldr, sh, w
