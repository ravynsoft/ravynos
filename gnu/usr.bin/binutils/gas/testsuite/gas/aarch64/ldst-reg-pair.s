/* ldst-reg-pair.s Test file for AArch64 load-store reg.pair instructions.

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

/* Includes:
 *   Load-store reg.pair (offset)
 *   Load-store reg.pair (post-ind.)
 *   Load-store reg.pair (pre-ind.)
 *   Load-store na.pair (pre-ind.)
 */

	// offset format
	.macro	op3_offset op, reg, imm
	\op	\reg\()7, \reg\()15, [sp, #\imm]
	.endm

	// post-ind. format
	.macro	op3_post_ind op, reg, imm
	\op	\reg\()7, \reg\()15, [sp], #\imm
	.endm

	// pre-ind. format
	.macro	op3_pre_ind op, reg, imm
	\op	\reg\()7, \reg\()15, [sp, #\imm]!
	.endm

	.macro	op3 op, reg, size, type
	// a variety of values for the imm7 field
	.irp	imm7, -64, -31, -1, 0, 15, 63
		// offset format
		.ifc \type, 1
		op3_offset	\op, \reg, "(\imm7*\size)"
		.endif
		// post-ind. format
		.ifc \type, 2
		op3_post_ind	\op, \reg, "(\imm7*\size)"
		.endif
		// pre-ind. format
		.ifc \type, 3
		op3_pre_ind	\op, \reg, "(\imm7*\size)"
		.endif
	.endr
	.endm

	.macro ldst_reg_pair type
	//     	op, reg, size(in byte) of one of the pair, type
	op3	stp, w, 4, \type
	op3	ldp, w, 4, \type

	op3	ldpsw, x, 4, \type

	op3	stp, x, 8, \type
	op3	ldp, x, 8, \type

	op3	stp, s, 4, \type
	op3	ldp, s, 4, \type

	op3	stp, d, 8, \type
	op3	ldp, d, 8, \type

	op3	stp, q, 16, \type
	op3	ldp, q, 16, \type
	.endm

	.macro ldst_reg_na_pair type
	//     	op, reg, size(in byte) of one of the pair, type
	op3	stnp, w, 4, \type
	op3	ldnp, w, 4, \type

	op3	stnp, x, 8, \type
	op3	ldnp, x, 8, \type

	op3	stnp, s, 4, \type
	op3	ldnp, s, 4, \type

	op3	stnp, d, 8, \type
	op3	ldnp, d, 8, \type

	op3	stnp, q, 16, \type
	op3	ldnp, q, 16, \type
	.endm

func:
	// Load-store reg.pair (offset)
	ldst_reg_pair	1

	// Load-store reg.pair (post-ind.)
	ldst_reg_pair	2

	// Load-store reg.pair (pre-ind.)
	ldst_reg_pair	3

	// Load-store na.pair (offset)
	ldst_reg_na_pair	1
