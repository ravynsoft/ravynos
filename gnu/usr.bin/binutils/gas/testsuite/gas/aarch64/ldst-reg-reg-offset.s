/* ldst-reg-reg-offset.s Test file for AArch64 load-store reg. (reg.offset)
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

/* Only instructions loading from/storing to FP/SIMD register are
   tested here.  */

	.macro	op3_32 op, reg, ext, imm
	.ifc \imm, -1
	\op	\reg\()7, [sp, w7, \ext]
	.else
	\op	\reg\()7, [sp, w7, \ext #\imm]
	.endif
	.endm

	.macro	op3_64 op, reg, ext, imm
	.ifc \imm, -1
	\op	\reg\()7, [sp, x7, \ext]
	.else
	\op	\reg\()7, [sp, x7, \ext #\imm]
	.endif
	.endm

	.macro	op3 op, reg, ext, imm=-1
	.ifc \ext, uxtw
	op3_32	\op, \reg, \ext, \imm
	.endif
	.ifc \ext, sxtw 
	op3_32	\op, \reg, \ext, \imm
	.endif
	.ifc \ext, lsl 
		.ifnc \imm, -1
		// shift <amount> is mandatory when 'lsl' is used
		op3_64	\op, \reg, \ext, \imm
		.else
		// absent shift; lsl by default
		\op	\reg\()7, [sp, x7]
		.endif
	.endif
	.ifc \ext, sxtx
	op3_64	\op, \reg, \ext, \imm
	.endif
	.endm

	.macro shift op, ext
	op3	\op, b, \ext
	op3	\op, b, \ext, 0
	op3	\op, h, \ext, 0
	op3	\op, h, \ext, 1
	op3	\op, s, \ext, 0
	op3	\op, s, \ext, 2
	op3	\op, d, \ext, 0
	op3	\op, d, \ext, 3
	op3	\op, q, \ext, 0
	op3	\op, q, \ext, 4 
	.endm

	.macro extend op
	.irp ext, uxtw, lsl, sxtw, sxtx
	shift	\op, \ext
	.endr
	.endm

	.macro ld_or_st op
	extend	\op
	.endm

func:
	ld_or_st	str
	ld_or_st	ldr

	/* When the index register is of register 31, it should be ZR.  */
	ldr	x1, [sp, xzr, sxtx #3]
	str	x1, [sp, xzr, sxtx #3]
	ldr	w1, [sp, wzr, sxtw #2]
	str	w1, [sp, wzr, sxtw #2]
