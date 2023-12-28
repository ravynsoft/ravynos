/* ldst-reg-unscaled-imm.s Test file for AArch64 
   load-store reg. (unscaled imm.) instructions.

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


/* Prefetch memory instruction is not tested here.

   Also note that a programmer-friendly disassembler could display
   LDUR/STUR instructions using the standard LDR/STR mnemonics when
   the encoded immediate is negative or unaligned. However this behaviour
   is not required by the architectural assembly language.  */

	.macro	op2_no_imm op, reg
	\op	\reg\()7, [sp]
	.endm

	.macro	op2 op, reg, simm
	\op	\reg\()7, [sp, #\simm]
	.endm

	// load to or store from core register
	.macro ld_or_st op, suffix, reg 
	.irp simm, -256, -171
		op2	\op\suffix, \reg, \simm
	.endr
	op2_no_imm	\op\suffix, \reg
	.irp simm, 0, 2, 4, 8, 16, 85, 255
		op2	\op\suffix, \reg, \simm
	.endr
	.endm

	// load to or store from FP/SIMD register
	.macro ld_or_st_v op
	.irp reg, b, h, s, d, q
		.irp simm, -256, -171
			op2	\op, \reg, \simm
		.endr
		op2_no_imm	\op, \reg
		.irp simm, 0, 2, 4, 8, 16, 85, 255
			op2	\op, \reg, \simm
		.endr
	.endr
	.endm

func:
	// load to or store from FP/SIMD register
	ld_or_st_v	stur
	ld_or_st_v	ldur

	// load to or store from core register
	//      	op, suffix, reg
	ld_or_st	stur,  b, w
	ld_or_st	stur,  h, w
	ld_or_st	stur,   , w
	ld_or_st	stur,   , x
	ld_or_st	ldur,  b, w
	ld_or_st	ldur,  h, w
	ld_or_st	ldur,   , w
	ld_or_st	ldur,   , x
	ld_or_st	ldur, sb, x
	ld_or_st	ldur, sh, x
	ld_or_st	ldur, sw, x
	ld_or_st	ldur, sb, w
	ld_or_st	ldur, sh, w
