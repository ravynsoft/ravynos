/* rdma.s Test file for AArch64 v8.1 Advanced-SIMD instructions.

   Copyright (C) 2012-2023 Free Software Foundation, Inc.  Contributed by ARM Ltd.

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


	.text
	.ifdef DIRECTIVE
	.arch_extension rdma
	.endif

         /* irp seems broken, so get creative.  */
	.macro vect_inst I, T
	.irp    x, 0.\T, 3.\T, 13.\T, 23.\T, 29.\T
	.irp    y, 1.\T, 4.\T, 14.\T, 24.\T, 30.\T
	.irp    z, 2.\T, 5.\T, 15.\T, 25.\T, 31.\T
	\I v\x, v\y, v\z
	.endr
	.endr
	.endr
	.endm

	.text
	.irp    inst, sqrdmlah, sqrdmlsh
        .irp    type, 4h, 8h, 2s, 4s
	vect_inst \inst \type
        .endr
	.endr

	.macro scalar_inst I R
	\I \R\()0, \R\()1, \R\()2
	.endm

	.text
	.irp    inst, sqrdmlah, sqrdmlsh
	.irp    reg, s,h
	scalar_inst \inst \reg
        .endr
        .endr

	.macro vect_indexed_inst I S T N
	.irp    x, 0.\S\T, 3.\S\T, 13.\S\T, 23.\S\T, 29.\S\T
	.irp    y, 1.\S\T, 4.\S\T, 14.\S\T, 24.\S\T, 30.\S\T
	.irp    z, 0.\T[\N], 5.\T[\N], 10.\T[\N], 13.\T[\N], 15.\T[\N]
	\I v\x, v\y, v\z
	.endr
	.endr
	.endr
	.endm

	.text
	.irp    inst, sqrdmlah, sqrdmlsh
	.irp    size, 4, 8
	.irp    index 0,1,2,3
	vect_indexed_inst \inst \size h \index
        .endr
	.endr
	.irp    size, 2, 4
	.irp    index 0,1,2,3
	vect_indexed_inst \inst \size s \index
        .endr
	.endr
	.endr

	.macro scalar_indexed_inst I T N
	\I \T\()0, \T\()1, v2.\T[\N]
	.endm

	.text
	.irp    inst, sqrdmlah, sqrdmlsh
	.irp    type h,s
	.irp    index 0,1,2,3
	scalar_indexed_inst \inst \type \index
	.endr
	.endr
	.endr
