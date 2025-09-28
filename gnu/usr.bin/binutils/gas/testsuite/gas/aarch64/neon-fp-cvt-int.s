/* neon-fp-cvt-ins.s Test file for AArch64 NEON
   floating-point<->fixed-point conversion and
   floating-point<->integer conversion instructions.

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

	.macro	do_cvt op, fbits, reg, reg_shape
	.ifc	\fbits, 0
	// Floating-point<->integer conversions
		.ifc	\reg, V
		\op	V7.\()\reg_shape, V7.\()\reg_shape
		.else
		\op	\reg\()7, \reg\()7
		.endif
	.else
	// Floating-point<->fixed-point conversions
		.ifc	\reg, V
			.ifle	\fbits-32
				.ifc	\reg_shape, 2S
				\op	V7.2S, V7.2S, #\fbits
				.endif
				.ifc	\reg_shape, 4S
				\op	V7.4S, V7.4S, #\fbits
				.endif
			.endif
			.ifc	\reg_shape, 2D
			\op	V7.2D, V7.2D, #\fbits
			.endif
		.else
			.ifc	\reg, S
				.ifle	\fbits-32
				\op	S7, S7, #\fbits
				.endif
			.endif
			.ifc	\reg, D
			\op	D7, D7, #\fbits
			.endif
		.endif
	.endif
	.endm

	.macro	fcvts_with_fbits fbits
	.ifc \fbits, 0
	// fp <-> int
		// AdvSIMD
		.irp reg_shape, 2S, 4S, 2D
		do_cvt	FCVTNS, \fbits, V, \reg_shape
		do_cvt	FCVTNU, \fbits, V, \reg_shape
		do_cvt	FCVTPS, \fbits, V, \reg_shape
		do_cvt	FCVTPU, \fbits, V, \reg_shape
		do_cvt	SCVTF, \fbits, V, \reg_shape
		do_cvt	UCVTF, \fbits, V, \reg_shape
		do_cvt	FCVTMS, \fbits, V, \reg_shape
		do_cvt	FCVTMU, \fbits, V, \reg_shape
		do_cvt	FCVTZS, \fbits, V, \reg_shape
		do_cvt	FCVTZU, \fbits, V, \reg_shape
		do_cvt	FCVTAS, \fbits, V, \reg_shape
		do_cvt	FCVTAU, \fbits, V, \reg_shape
		.endr
		// AdvSISD
		.irp reg, S, D
		do_cvt	FCVTNS, \fbits, \reg
		do_cvt	FCVTNU, \fbits, \reg
		do_cvt	FCVTPS, \fbits, \reg
		do_cvt	FCVTPU, \fbits, \reg
		do_cvt	SCVTF, \fbits, \reg
		do_cvt	UCVTF, \fbits, \reg
		do_cvt	FCVTMS, \fbits, \reg
		do_cvt	FCVTMU, \fbits, \reg
		do_cvt	FCVTZS, \fbits, \reg
		do_cvt	FCVTZU, \fbits, \reg
		do_cvt	FCVTAS, \fbits, \reg
		do_cvt	FCVTAU, \fbits, \reg
		.endr
	.else
	// fp <-> fixed-point
		// AdvSIMD
		.irp reg_shape, 2S, 4S, 2D
		do_cvt	SCVTF, \fbits, V, \reg_shape
		do_cvt	UCVTF, \fbits, V, \reg_shape
		do_cvt	FCVTZS, \fbits, V, \reg_shape
		do_cvt	FCVTZU, \fbits, V, \reg_shape
		.endr
		// AdvSISD
		.irp reg, S, D
		do_cvt	SCVTF, \fbits, \reg
		do_cvt	UCVTF, \fbits, \reg
		do_cvt	FCVTZS, \fbits, \reg
		do_cvt	FCVTZU, \fbits, \reg
		.endr
	.endif
	.endm


	.macro	fcvts_with_fbits_wrapper from=0, to=64
	fcvts_with_fbits \from
	.if	\to-\from
	fcvts_with_fbits_wrapper "(\from+1)", \to
	.endif
	.endm

func:
	// Generate fcvt instructions without fbits and
	// with fbits from 1 to 64, also generate [us]cvtf
	fcvts_with_fbits_wrapper from=0, to=64
