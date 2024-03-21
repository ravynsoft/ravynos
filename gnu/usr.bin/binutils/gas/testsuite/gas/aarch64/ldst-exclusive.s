/* ldst-exclusive.s Test file for AArch64 load-store exclusive
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


	/* <mnemonic>	<Wt>, [<Xn|SP>]{,#0}] */
	.macro LR32 op
	\op	w1, [x7]
	\op	w1, [x7, #0]
	\op	w1, [x7, 0]
	.endm

	/* <mnemonic>	<Xt>, [<Xn|SP>]{,#0}] */
	.macro LR64 op
	\op	x1, [x7]
	\op	x1, [x7, #0]
	\op	x1, [x7, 0]
	.endm

	/* <mnemonic>	<Ws>, <Wt>, [<Xn|SP>]{,#0}] */
	.macro SR32 op
	\op	w15, w1, [x7]
	\op	w15, w1, [x7, #0]
	\op	w15, w1, [x7, 0]
	.endm

	/* <mnemonic>	<Ws>, <Xt>, [<Xn|SP>]{,#0}] */
	.macro SR64 op
	\op	w15, x1, [x7]
	\op	w15, x1, [x7, #0]
	\op	w15, x1, [x7, 0]
	.endm

	/* <mnemonic>	<Wt1>, <Wt2>, [<Xn|SP>]{,#0}] */
	.macro LP32 op
	\op	w1, w2, [x7]
	\op	w1, w2, [x7, #0]
	\op	w1, w2, [x7, 0]
	.endm

	/* <mnemonic>	<Xt1>, <Xt2>, [<Xn|SP>]{,#0}] */
	.macro LP64 op
	\op	x1, x2, [x7]
	\op	x1, x2, [x7, #0]
	\op	x1, x2, [x7, 0]
	.endm

	/* <mnemonic>	<Ws>, <Wt1>, <Wt2>, [<Xn|SP>]{,#0}] */
	.macro SP32 op
	\op	w15, w1, w2, [x7]
	\op	w15, w1, w2, [x7, #0]
	\op	w15, w1, w2, [x7, 0]
	.endm

	/* <mnemonic>	<Ws>, <Xt1>, <Xt2>, [<Xn|SP>]{,#0}] */
	.macro SP64 op
	\op	w15, x1, x2, [x7]
	\op	w15, x1, x2, [x7, #0]
	\op	w15, x1, x2, [x7, 0]
	.endm

	/* <mnemonic>	<Wt>, [<Xn|SP>]{,#0}] */
	.macro SL32 op
	\op	w1, [x7]
	\op	w1, [x7, #0]
	\op	w1, [x7, 0]
	.endm

	/* <mnemonic>	<Xt>, [<Xn|SP>]{,#0}] */
	.macro SL64 op
	\op	x1, [x7]
	\op	x1, [x7, #0]
	\op	x1, [x7, 0]
	.endm

func:
	.irp	op, stxrb, stxrh, stxr
	SR32	\op
	.endr

	SR64	stxr

	.irp	op, ldxrb, ldxrh, ldxr
	LR32	\op
	.endr

	LR64	ldxr

	SP32	stxp
	SP64	stxp
	LP32	ldxp
	LP64	ldxp

	.irp	op, stlxrb, stlxrh, stlxr
	SR32	\op
	.endr

	SR64	stlxr

	.irp	op, ldaxrb, ldaxrh, ldaxr
	LR32	\op
	.endr

	LR64	ldaxr

	SP32	stlxp
	SP64	stlxp
	LP32	ldaxp
	LP64	ldaxp

	.irp	op, stlrb, stlrh, stlr
	SL32	\op
	.endr

	SL64	stlr

	.irp	op, ldarb, ldarh, ldar
	LR32	\op
	.endr

	LR64	ldar
