/* ld-reg-uns-imm.s Test file for AArch64 load-store reg. (uns.imm)
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

/* Prefetch memory instruction is not tested here.

   Also note that as a programmer-friendly assembler, GAS generates
   LDUR/STUR instructions in response to the standard LDR/STR mnemonics
   when the immediate offset is unambiguous, i.e. when it is negative
   or unaligned.  Similarly a disassembler could display these
   instructions using the standard LDR/STR mnemonics when the encoded
   immediate is negative or unaligned.  However this behaviour is not
   required by the architectural assembly language. */

	.macro	op2_no_imm op, reg
	\op	\reg\()7, [sp]
	.endm

	.macro	op2 op, reg, simm
	\op	\reg\()7, [sp, #\simm]
	.endm

	// load to or store from core register
	// size is the access size in byte
	.macro ld_or_st op, suffix, reg, size
	.irp simm, -256, -171
		op2	\op\suffix, \reg, \simm
	.endr
	op2_no_imm	\op\suffix, \reg
	.irp simm, 0, 2, 4, 8, 16, 85, 255
		op2	\op\suffix, \reg, \simm
	.endr
	op2	\op\suffix, \reg, "(4095*\size)"
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
		.ifc \reg, b
			op2	\op, \reg, 4095
		.endif
		.ifc \reg, h
			op2	\op, \reg, 8190
		.endif
		.ifc \reg, s
			op2	\op, \reg, 16380
		.endif
		.ifc \reg, d
			op2	\op, \reg, 32760
		.endif
		.ifc \reg, q
			op2	\op, \reg, 65520
		.endif
	.endr
	.endm

func:
	// load to or store from FP/SIMD register
	ld_or_st_v	str
	ld_or_st_v	ldr

	// load to or store from core register
	//      	op, suffix, reg, size(in byte)
	ld_or_st	str,  b, w, 1
	ld_or_st	str,  h, w, 2
	ld_or_st	str,   , w, 4
	ld_or_st	str,   , x, 8
	ld_or_st	ldr,  b, w, 1
	ld_or_st	ldr,  h, w, 2
	ld_or_st	ldr,   , w, 4
	ld_or_st	ldr,   , x, 8
	ld_or_st	ldr, sb, x, 1
	ld_or_st	ldr, sh, x, 2
	ld_or_st	ldr, sw, x, 4
	ld_or_st	ldr, sb, w, 1
	ld_or_st	ldr, sh, w, 2

