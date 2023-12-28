/* bitfield-alias.s Test file for AArch64 bitfield instructions
   alias mnemonics.

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

/* This file tests the GAS's ability in assembling the alias mnemonics
   of sbfm, bfm and ubfm.  Disassembler should prefer to use alias
   mnemonics to display {[u|s]}bfm instructions.
   bitfield-bfm.s and bitfield-alias.s will be assembled into idential
   binary, which is why the two tests share the same dump match
   file 'bitfield-dump'.
   This assembly file is also used for the bitfield-no-aliases test.  */

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

	// <op>	<Wd>, <Wn>, #<lsb>, #<width>
	.macro bf_32 op, lsb, width
	\op	wzr, w7, #\lsb, #\width
	.endm

	// <op>	<Xd>, <Xn>, #<lsb>, #<width>
	.macro bf_64 op, lsb, width
	\op	xzr, x7, #\lsb, #\width
	.endm

.text
	/*
	 * extend
	 */

	bf_32r	sxtb
	bf_64x	sxtb
	bf_32r	sxth
	bf_64x	sxth
	bf_64x	sxtw

	bf_32r	uxtb
	bf_64x	uxtb
	bf_32r	uxth
	bf_64x	uxth
	bf_32r	uxtw
	bf_64x	uxtw

	/*
	 * shift
	 */

	.irp	op, asr, lsr, lsl
	.irp	shift, 0, 16, 31
	bf_32s	\op, \shift
	.endr
	.irp	shift, 0, 31, 63
	bf_64s	\op, \shift
	.endr
	.endr

	/*
	 * Insert & Extract
	 */

	.irp	op, sbfiz, sbfx, bfi, bfxil, ubfiz, ubfx
	bf_32	\op, 0, 1
	bf_32	\op, 0, 16
	bf_32	\op, 0, 32
	bf_32	\op, 16, 1
	bf_32	\op, 16, 8
	bf_32	\op, 16, 16
	bf_32	\op, 31, 1

	bf_64	\op, 0, 1
	bf_64	\op, 0, 32
	bf_64	\op, 0, 64
	bf_64	\op, 32, 1
	bf_64	\op, 32, 16
	bf_64	\op, 32, 32
	bf_64	\op, 63, 1
	.endr
