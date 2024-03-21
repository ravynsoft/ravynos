/* floatdp2.s Test file for AArch64 Floating-point data-processing
   (2 source) instructions.

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

	.macro	floatdp2 op, type
	\op	\type\()0, \type\()7, \type\()15
	.endm

.text
	.irp	type, S, D
	.irp	op, FMUL, FDIV, FADD, FSUB, FMAX, FMIN, FMAXNM, FMINNM, FNMUL
	floatdp2	\op, \type
	.endr
	.endr
