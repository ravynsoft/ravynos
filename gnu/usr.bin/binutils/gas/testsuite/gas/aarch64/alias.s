/* alias.s Test file for AArch64 instructions aliases or disassembly
   preference.  It is also used to test the -Mno-aliases option in
   the disassemler.

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

.text
	extr	w0, w1, w2, #15
	extr	x0, x1, x2, #15
	extr	w0, w3, w3, #7
	extr	x0, x5, x5, #7
	ror	w6, w7, #18
	ror	x6, x7, #40

	madd	w0, w1, w2, w3
	madd	w0, w1, w2, wzr
	mul	w0, w1, w2
	msub	x0, x1, x2, x3
	msub	x0, x1, x2, xzr
	mneg	x0, x1, x2
	smaddl	x0, w1, w2, x3
	smaddl	x0, w1, w2, xzr
	smull	x0, w1, w2
	smsubl	x0, w1, w2, x3
	smsubl	x0, w1, w2, xzr
	smnegl	x0, w1, w2
	umaddl	x0, w1, w2, x3
	umaddl	x0, w1, w2, xzr
	umull	x0, w1, w2
	umsubl	x0, w1, w2, x3
	umsubl	x0, w1, w2, xzr
	umnegl	x0, w1, w2

	csinc	w0, w1, wzr, eq
	csinc	w0, w1, w1, eq
	cinc	w0, w1, ne
	csinc	w0, wzr, wzr, lo
	cset	w0, cs
	csinv	x0, x1, xzr, hs
	csinv	x0, x1, x1, hs
	cinv	x0, x1, cc
	csinv	x0, xzr, xzr, mi
	csetm	x0, pl
	csneg	x0, xzr, x30, lt
	csneg	x0, x30, x30, lt
	cneg	x0, x30, ge

	ands	x0, x1, x2
	ands	xzr, x1, x2
	tst	x1, x2
	ands	wzr, w1, w2, ror #31
	tst	w1, w2, ror #31

	orn	x0, x1, x2
	orn	xzr, x1, x2
	orn	x0, xzr, x2
	mvn	x0, x2
	orn	wzr, w1, w2, asr #15
	orn	w0, wzr, w2, asr #15
	mvn	w0, w2, asr #15

	mov	v0.8b, v1.8b
	orr	v0.8b, v1.8b, v2.8b
	orr	v0.8b, v1.8b, v1.8b

	mov	x3, x17
	orr	x3, x0, x17
	orr	x3, xzr, x17

	bic	x1, x1, #(1<<30)-1
	bic	x0, x0, #2
	bic	w0, w0, #2

	ands	wzr, w24, #0x7f8
	ands	w0, w24, #0x7f8
	tst	w24, #0x7f8

	subs	wzr, w3, #0x20
	subs	w3, wsp, #0x20
	cmp	w3, #0x20

	adds	xzr, x15, #0xfff
	subs	x15, sp, #0xfff
	cmn	x15, #0xfff

	.macro asimdshll	s
	 \s\()xtl v8.8h, v2.8b
	 \s\()shll v8.8h, v2.8b, #0
	 \s\()xtl2 v8.8h, v2.16b
	 \s\()shll2 v8.8h, v2.16b, #0
	 \s\()xtl v8.4s, v2.4h
	 \s\()shll v8.4s, v2.4h, #0
	 \s\()xtl2 v8.4s, v2.8h
	 \s\()shll2 v8.4s, v2.8h, #0
	 \s\()xtl v8.2d, v2.2s
	 \s\()shll v8.2d, v2.2s, #0
	 \s\()xtl2 v8.2d, v2.4s
	 \s\()shll2 v8.2d, v2.4s, #0
	.endm

	asimdshll	s
	asimdshll	u

	csinc	w0, w1, w1, nv
	csinc	w0, w1, w1, al
	csinc	w0, wzr, wzr, nv
	csinc	w0, wzr, wzr, al
	csinv	w0, w1, w1, nv
	csinv	w0, w1, w1, al
	csinv	w0, wzr, wzr, nv
	csinv	w0, wzr, wzr, al
	csneg	w0, w1, w1, nv
	csneg	w0, w1, w1, al
