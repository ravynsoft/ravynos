/* fp_cvt_ins.s Test file for AArch64 floating-point<->fixed-point
   conversion and floating-point<->integer conversion instructions.

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

	// SCVTF & UCVTF
	.macro do_cvtf fbits, reg
	.ifc	\fbits, 0
	// Floating-point<->integer conversions
	SCVTF	\reg\()7, W7
	SCVTF	\reg\()7, X7
	UCVTF	\reg\()7, W7
	UCVTF	\reg\()7, X7
	.else
	// Floating-point<->fixed-point conversions
	.ifle	\fbits-32
	SCVTF	\reg\()7, W7, #\fbits
	.endif
	SCVTF	\reg\()7, X7, #\fbits
	.ifle	\fbits-32
	UCVTF	\reg\()7, W7, #\fbits
	.endif
	UCVTF	\reg\()7, X7, #\fbits
	.endif
	.endm

	// FMOV
	.macro	do_fmov	type
	.ifc	\type, S
	// 32-bit
	FMOV	W7, S7
	FMOV	S7, W7
	.elseif	\type == D
	// 64-bit
	FMOV	X7, D7
	FMOV	D7, X7
	.else
	// 64-bit with V reg element
	FMOV	X7, V7.D[1]
	FMOV	V7.D[1], X7
	.endif
	.endm

	.macro	do_fcvt suffix, fbits, reg
	.ifc	\fbits, 0
	// Floating-point<->integer conversions
	FCVT\suffix	W7, \reg\()7
	FCVT\suffix	X7, \reg\()7
	.else
	// Floating-point<->fixed-point conversions
	.ifle	\fbits-32
	FCVT\suffix	W7, \reg\()7, #\fbits
	.endif
	FCVT\suffix	X7, \reg\()7, #\fbits
	.endif
	.endm

	.macro	fcvts_with_fbits fbits
	.ifc \fbits, 0
	// fp <-> integer
		.irp reg, S, D
		// single-precision and double precision
		do_fcvt	NS, \fbits, \reg
		do_fcvt	NU, \fbits, \reg
		do_fcvt	PS, \fbits, \reg
		do_fcvt	PU, \fbits, \reg
		do_fcvt	MS, \fbits, \reg
		do_fcvt	MU, \fbits, \reg
		do_fcvt	ZS, \fbits, \reg
		do_fcvt	ZU, \fbits, \reg
		do_cvtf	\fbits, \reg
		do_fcvt	AS, \fbits, \reg
		do_fcvt	AU, \fbits, \reg
		do_fmov	S
		.endr
	.else
	// fp <-> fixed-point
	// After ISA 2.06, only FCVTZ[US] and [US]CVTF are available
		.irp reg, S, D
		// single-precision and double precision
		do_fcvt	ZS, \fbits, \reg
		do_fcvt	ZU, \fbits, \reg
		do_cvtf	\fbits, \reg
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
	// and fmov.
	fcvts_with_fbits_wrapper from=0, to=64
	do_fmov V
