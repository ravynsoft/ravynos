/* addsub.s Test file for AArch64 add-subtract instructions.

   Copyright (C) 2012-2023 Free Software Foundation, Inc.
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

// TODO: also cover the addsub_imm instructions.

	/*
	 * Adjust Rm
	 */
	.macro adjust_rm	op, rd, rn, rm_r, rm_n, extend, amount
	// for 64-bit instruction, Rm is Xm when <extend> is explicitely
	// or implicitly UXTX, SXTX or LSL; otherwise it Wm.
	.ifc \rm_r, X
		.ifnc \extend, UXTX
			.ifnc \extend, SXTX
				.ifnc \extend, LSL
					.ifb \amount
					\op	\rd, \rn, W\()\rm_n, \extend
					.else
					\op	\rd, \rn, W\()\rm_n, \extend #\amount
					.endif
					.exitm
				.endif
			.endif
		.endif
	.endif

	.ifb \amount
	\op	\rd, \rn, \rm_r\()\rm_n, \extend
	.else
	\op	\rd, \rn, \rm_r\()\rm_n, \extend #\amount
	.endif
	.endm

	/*
	 * Emitting addsub_ext instruction
	 */
	.macro do_addsub_ext	type, op, Rn, reg, extend, amount
	.ifc \type, 0
		// normal add/adds/sub/subs
		.ifb \extend
		\op	\reg\()16, \Rn, \reg\()1
		.else
		.ifb \amount
		adjust_rm	\op, \reg\()16, \Rn, \reg, 1, \extend
		.else
		adjust_rm	\op, \reg\()16, \Rn, \reg, 1, \extend, \amount
		.endif
		.endif
	.else
	.ifc \type, 1
		// adds/subs with ZR as Rd
		.ifb \extend
		\op	\reg\()ZR, \Rn, \reg\()1
		.else
		.ifb \amount
		adjust_rm	\op, \reg\()ZR, \Rn, \reg, 1, \extend
		.else
		adjust_rm	\op, \reg\()ZR, \Rn, \reg, 1, \extend, \amount
		.endif
		.endif
	.else
		// cmn/cmp
		.ifb \extend
		\op	\Rn, \reg\()1
		.else
		.ifb \amount
		\op	\Rn, \reg\()1, \extend
		.else
		\op	\Rn, \reg\()1, \extend #\amount
		.endif
		.endif
	.endif
	.endif
	.endm

	/*
	 * Optional extension and optional shift amount
	 */
	.macro do_extend type, op, Rn, reg
	// <extend> absent
	// note that when SP is not used, the GAS will encode it as addsub_shift
	do_addsub_ext	\type, \op, \Rn, \reg
	// optional absent <amount>
	.irp extend, UXTB, UXTH, UXTW, UXTX, SXTB, SXTH, SXTW, SXTX
	.irp amount,  , 0, 1, 2, 3, 4
	do_addsub_ext	\type, \op, \Rn, \reg, \extend, \amount
	.endr
	.endr
	// when <extend> is LSL, <amount> cannot be absent
	// note that when SP is not used, the GAS will encode it as addsub_shift
	.irp amount, 0, 1, 2, 3, 4
	do_addsub_ext	\type, \op, \Rn, \reg, LSL, \amount
	.endr
	.endm

	/*
	 * Leaf macro emitting addsub_shift instruction
	 */
	.macro do_addsub_shift	type, op, R, reg, shift, amount
	.ifc \type, 0
		// normal add/adds/sub/subs
		.ifb \shift
		\op	\reg\()16, \R, \reg\()1
		.else
		\op	\reg\()16, \R, \reg\()1, \shift #\amount
		.endif
	.else
	.ifc \type, 1
		// adds/subs with ZR as Rd
		.ifb \shift
		\op	\reg\()ZR, \R, \reg\()1
		.else
		\op	\reg\()ZR, \R, \reg\()1, \shift #\amount
		.endif
	.else
	.ifc \type, 2
		// cmn/cmp/neg/negs
		.ifb \shift
		\op	\R, \reg\()1
		.else
		\op	\R, \reg\()1, \shift #\amount
		.endif
	.else
		// sub/subs with ZR as Rn
		.ifb \shift
		\op	\R, \reg\()ZR, \reg\()1
		.else
		\op	\R, \reg\()ZR, \reg\()1, \shift #\amount
		.endif
	.endif
	.endif
	.endif
	.endm

	/*
	 * Optional shift and optional shift amount
	 */
	.macro do_shift type, op, R, reg
	// <shift> absent
	do_addsub_shift	\type, \op, \R, \reg
	// optional absent <amount>
	.irp shift, LSL, LSR, ASR
	.irp amount, 0, 1, 2, 3, 4, 5, 16, 31
	// amount cannot be absent when shift is present.
	do_addsub_shift	\type, \op, \R, \reg, \shift, \amount
	.endr
	.ifc \reg, X
	do_addsub_shift	\type, \op, \R, \reg, \shift, 63
	.endif
	.endr
	.endm

func:
	/*
	 * Add-subtract (extended register)
	 */

	.irp op, ADD, ADDS, SUB, SUBS
	do_extend	0, \op, W7, W
	do_extend	0, \op, WSP, W
	do_extend	0, \op, X7, X
	do_extend	0, \op, SP, X
	.endr

	.irp op, ADDS, SUBS
	do_extend	1, \op, W7, W
	do_extend	1, \op, WSP, W
	do_extend	1, \op, X7, X
	do_extend	1, \op, SP, X
	.endr

	.irp op, CMN, CMP
	do_extend	2, \op, W7, W
	do_extend	2, \op, WSP, W
	do_extend	2, \op, X7, X
	do_extend	2, \op, SP, X
	.endr

	/*
	 * Add-subtract (shift register)
	 */

	.irp op, ADD, ADDS, SUB, SUBS
	do_shift	0, \op, W7, W
	do_shift	0, \op, X7, X
	.endr

	.irp op, ADDS, SUBS
	do_shift	1, \op, W7, W
	do_shift	1, \op, X7, X
	.endr

	.irp op, CMN, CMP
	do_shift	2, \op, W7, W
	do_shift	2, \op, X7, X
	.endr

	.irp op, SUB, SUBS
	do_shift	3, \op, W7, W
	do_shift	3, \op, X7, X
	.endr

	.irp op, NEG, NEGS
	do_shift	2, \op, W7, W
	do_shift	2, \op, X7, X
	.endr

	/*
	 * Check for correct aliasing
	 */

	.irp op, NEGS
	do_shift	2, \op, WZR, W
	do_shift	2, \op, XZR, X
	.endr

	.irp op, SUBS
	do_shift	3, \op, W7, W
	do_shift	3, \op, X7, X
	do_shift	0, \op, WZR, W
	do_shift	0, \op, XZR, X
	.endr
