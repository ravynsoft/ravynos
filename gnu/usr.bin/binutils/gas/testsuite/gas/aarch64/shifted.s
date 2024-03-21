/* shifted.s Test file for AArch64 add-substract (extended reg.) and
   add-substract (shifted reg.) instructions.

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

	.macro	op3_64 op, shift
	\op	x1, x2, x3, \shift #0
	\op	x1, x2, x3, \shift #1
	\op	x1, x2, x3, \shift #3
	\op	x1, x2, x3, \shift #7
	\op	x1, x2, x3, \shift #15
	\op	x1, x2, x3, \shift #31
	\op	x1, x2, x3, \shift #63
	.endm	
	
	.macro	op3_32 op, shift
	\op	w1, w2, w3, \shift #0
	\op	w1, w2, w3, \shift #1
	\op	w1, w2, w3, \shift #3
	\op	w1, w2, w3, \shift #7
	\op	w1, w2, w3, \shift #15
	\op	w1, w2, w3, \shift #31
	.endm	

	.macro	op3_64x op, shift
	\op	x1, x2, w3, \shift
	\op	x1, x2, w3, \shift #1
	\op	x1, x2, w3, \shift #2
	\op	x1, x2, w3, \shift #3
	\op	x1, x2, w3, \shift #4
	.endm
	
	.macro	op3_64x_more op, shift
	\op	x1, x2, x3, \shift
	\op	x1, x2, x3, \shift #1
	\op	x1, x2, x3, \shift #2
	\op	x1, x2, x3, \shift #3
	\op	x1, x2, x3, \shift #4
	.endm

	.macro	op3_32x op, shift
	\op	w1, w2, w3, \shift
	\op	w1, w2, w3, \shift #1
	\op	w1, w2, w3, \shift #2
	\op	w1, w2, w3, \shift #3
	\op	w1, w2, w3, \shift #4
	.endm	
	
	.macro	op2_64 op, shift
	\op	x2, x3, \shift #0
	\op	x2, x3, \shift #1
	\op	x2, x3, \shift #3
	\op	x2, x3, \shift #7
	\op	x2, x3, \shift #15
	\op	x2, x3, \shift #31
	\op	x2, x3, \shift #63
	.endm	
	
	.macro	op2_32 op, shift
	\op	w2, w3, \shift #0
	\op	w2, w3, \shift #1
	\op	w2, w3, \shift #3
	\op	w2, w3, \shift #7
	\op	w2, w3, \shift #15
	\op	w2, w3, \shift #31
	.endm	

	.macro	op2_64x op, shift
	\op	x2, w3, \shift
	\op	x2, w3, \shift #1
	\op	x2, w3, \shift #2
	\op	x2, w3, \shift #3
	\op	x2, w3, \shift #4
	.endm	
	
	.macro	op2_32x op, shift
	\op	w2, w3, \shift
	\op	w2, w3, \shift #1
	\op	w2, w3, \shift #2
	\op	w2, w3, \shift #3
	\op	w2, w3, \shift #4
	.endm	
	
	.macro logical op
	op3_64	\op, lsl
	op3_64	\op, lsr
	op3_64	\op, asr
	op3_64	\op, ror
	op3_32	\op, lsl
	op3_32	\op, lsr
	op3_32	\op, asr
	op3_32	\op, ror
	.endm
	
	.macro arith3 op
	op3_64	\op, lsl
	op3_64	\op, lsr
	op3_64	\op, asr
	op3_64x	\op, uxtb
	op3_64x	\op, uxth
	op3_64x	\op, uxtw
	op3_64x_more	\op, uxtx
	op3_64x	\op, sxtb
	op3_64x	\op, sxth
	op3_64x	\op, sxtw
	op3_64x_more	\op, sxtx
	op3_32	\op, lsl
	op3_32	\op, lsr
	op3_32	\op, asr
	op3_32x	\op, uxtb
	op3_32x	\op, uxth
	op3_32x	\op, sxtb
	op3_32x	\op, sxth
	.endm
	
	.macro arith2 op, if_ext=1
	op2_64	\op, lsl
	op2_64	\op, lsr
	op2_64	\op, asr
	.if \if_ext
	op2_64x	\op, uxtb
	op2_64x	\op, uxth
	op2_64x	\op, uxtw
	op2_64x	\op, sxtb
	op2_64x	\op, sxth
	op2_64x	\op, sxtw
	.endif
	op2_32	\op, lsl
	op2_32	\op, lsr
	op2_32	\op, asr
	.if \if_ext
	op2_32x	\op, uxtb
	op2_32x	\op, uxth
	op2_32x	\op, sxtb
	op2_32x	\op, sxth
	.endif
	.endm
	
func:	
	logical	orr
	logical	and
	logical	eor
	
	logical	bic
	logical	orn
	logical	eon
	
	arith3	add	
	arith3	sub
	
	arith2	neg, 0
	arith2	cmp
	arith2	cmn
