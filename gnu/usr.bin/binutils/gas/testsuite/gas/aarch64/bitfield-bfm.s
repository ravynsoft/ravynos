/* bitfield-bfm.s Test file for AArch64 bitfield instructions
   sbfm, bfm and ubfm mnemonics.

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

/* This file tests the GAS's ability in assembling sbfm, bfm and ubfm
   instructions.  Disassembler should use alias mnemonics to display
   {[u|s]}bfm instructions.  bitfield-bfm.s and bitfield-alias.s will be
   assembled into idential binary, which is why the two tests share the
   same dump match file 'bitfield-dump'.  */

	// <op>	<Wd>, <Wn>
	.macro bf_32r op
	\op	wzr, w7
	.endm

	// <op>	<Xd>, <Wn>
	.macro bf_64x op
	\op	xzr, w7
	.endm

	// <op>	<Wd>, <Wn>, #<shift>
	.macro bf_32s op, shift
	\op	wzr, w7, \shift
	.endm

	// <op>	<Xd>, <Xn>, #<shift>
	.macro bf_64s op, shift
	\op	xzr, x7, \shift
	.endm


	.macro op_bfm	signed, reg, immr, imms
	\signed\()bfm	\reg\()zr, \reg\()7, #\immr, #\imms	// e.g. sbfm	xzr, x7, #0, #15
	.endm

	.macro ext2bfm	signed, reg, imms
	op_bfm	signed=\signed, reg=\reg, immr=0, imms=\imms
	.endm

	// shift right -> bfm
	.macro sr2bfm signed, reg, shift, imms
	op_bfm	signed=\signed, reg=\reg, immr=\shift, imms=\imms
	.endm

	// shift left -> bfm
	.macro sl2bfm signed, reg, shift
	.ifc \reg, w
	op_bfm	signed=\signed, reg=\reg, immr="((32-\shift)&31)", imms="(31-\shift)"
	.else
	op_bfm	signed=\signed, reg=\reg, immr="((64-\shift)&63)", imms="(63-\shift)"
	.endif
	.endm

	// bitfield insert -> bfm
	.macro ins2bfm signed, reg, lsb, width
	.ifc \reg, w
	op_bfm	signed=\signed, reg=\reg, immr="((32-\lsb)&31)", imms="(\width-1)"
	.else
	op_bfm	signed=\signed, reg=\reg, immr="((64-\lsb)&63)", imms="(\width-1)"
	.endif
	.endm

	// bitfield extract -> bfm
	.macro x2bfm signed, reg, lsb, width
	op_bfm	signed=\signed, reg=\reg, immr=\lsb, imms="(\lsb+\width-1)"
	.endm

.text
	/*
	 * aliasing extend
	 */

	ext2bfm	s, w, 7		// sxtb	wzr, w7
	ext2bfm	s, x, 7		// sxtb	xzr, x7
	ext2bfm s, w, 15 	// sxth	wzr, w7
	ext2bfm s, x, 15 	// sxth	xzr, x7
	ext2bfm s, x, 31 	// sxtw	xzr, x7

	ext2bfm u, w, 7 	// uxtb	wzr, w7
	ext2bfm u, w, 7 	// uxtb	xzr, w7
	ext2bfm u, w, 15	// uxth	wzr, w7
	ext2bfm u, w, 15 	// uxth	xzr, w7
	orr	wzr, wzr, w7	// uxtw	wzr, w7
	orr	wzr, wzr, w7	// uxtw	wzr, w7

	/*
	 * aliasing shift
	 */

	.irp	shift 0, 16, 31	// asr	wzr, w7, #\shift
	sr2bfm	s, w, \shift, 31
	.endr

	.irp	shift 0, 31, 63	// asr	xzr, x7, #\shift
	sr2bfm	s, x, \shift, 63
	.endr

	.irp	shift 0, 16, 31	// lsr	wzr, w7, #\shift
	sr2bfm	u, w, \shift, 31
	.endr

	.irp	shift 0, 31, 63	// lsr	xzr, x7, #\shift
	sr2bfm	u, x, \shift, 63
	.endr

	.irp	shift 0, 16, 31	// lsl	wzr, w7, #\shift
	sl2bfm	u, w, \shift
	.endr

	.irp	shift 0, 31, 63	// lsl	xzr, x7, #\shift
	sl2bfm	u, x, \shift
	.endr

	/*
	 * aliasing insert and extract
         */

	.irp	signed, s,  , u
	.irp	whichm, ins2bfm, x2bfm
	\whichm	\signed, w, 0, 1
	\whichm	\signed, w, 0, 16
	\whichm	\signed, w, 0, 32
	\whichm	\signed, w, 16, 1
	\whichm	\signed, w, 16, 8
	\whichm	\signed, w, 16, 16
	\whichm	\signed, w, 31, 1

	\whichm	\signed, x, 0, 1
	\whichm	\signed, x, 0, 32
	\whichm	\signed, x, 0, 64
	\whichm	\signed, x, 32, 1
	\whichm	\signed, x, 32, 16
	\whichm	\signed, x, 32, 32
	\whichm	\signed, x, 63, 1
	.endr
	.endr
