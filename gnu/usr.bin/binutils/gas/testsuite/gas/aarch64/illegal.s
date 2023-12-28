/* illegal.s Test file for AArch64 instructions that should be rejected
   by the assembler.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.  Contributed by ARM Ltd.

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
	// For urecpe and ursqrte, only 2s and 4s are accepted qualifiers.
	urecpe	v0.1d, v7.1d
	urecpe	v0.2d, v7.2d
	ursqrte	v0.1d, v7.1d
	ursqrte	v0.2d, v7.2d

	// For AdvSIMD (across) instructions, there are restraints on the register type and qualifiers.
	saddlv	b7, v31.8b
	saddlv	d7, v31.2s
	saddlv	q7, v31.2d
	smaxv	s7, v31.2s
	sminv	d7, v31.2d
	fmaxv	h7, v31.2h
	fmaxv	s7, v31.4h
	fminv	d7, v31.2d

	abs b0, b31
	neg b0, b31
	abs h0, h31
	neg h0, h31
	abs s0, s31
	neg s0, s31

	fcvt	s0, s0

	bfm	w0, w1, 8, 43
	ubfm	w0, x1, 8, 31

	aese	v1.8b, v2.8b
	sha1h	s7, d31
	sha1h	q7, d31
	sha1su1 v7.4s, v7.2s
	sha256su0 v7.2d, v7.2d
	sha1c	q7, q3, v7.4s
	sha1p	s7, q8, v9.4s
	sha1m	v8.4s, v7.4s, q8
	sha1su0	v0.2d, v1.2d, v2.2d
	sha256h	q7, s2, v8.4s

	pmull	v7.8b, v15.8b, v31.8b
	pmull	v7.1q, v15.1q, v31.1d
	pmull2	v7.8h, v15.8b, v31.8b
	pmull2	v7.1q, v15.2d, v31.1q

	ld2	{v1.4h, v0.4h}, [x1]
	strb	x0, [sp, x1, lsl #0]
	strb	w7, [x30, x0, lsl]
	strb	w7, [x30, x0, lsl #1]
	ldtr	x7, [x15, 266]
	sttr	x7, [x15, #1]!
	stxrb	x2, w1, [sp]
	stxp	w2, x3, w4, [x0]
	ldxp	w3, x4, [x30]

	st2	{v4.2d, v5.2d}, [x3, #3]
	st2	{v4.2d, v5.2d, v6.2d}, [x3]
	st1	{v4.2d, v6.2d, v8.2d}, [x3]
	st3	{v4.2d, v6.2d}, [x3]
	st4	{v4.2d, v6.2d}, [x3]
	st2	{v4.2d, v6.2d, v8.2d, v10.2d}, [x3]
	st2	{v4.2d, v6.2d, v8.2d, v10.2d}, [x3], 48

	ext	v0.8b, v1.8b, v2.8b, 8
	ext	v0.16b, v1.16b, v2.16b, 20

	tbz	w0, #40, 0x17c

	svc

	fmov	v1.D[0], x0
	fmov	v2.S[2], x0
	fmov	v2.S[1], x0
	fmov	v2.D[1], w0

	smaddl	w0, w1, w2, x3
	smaddl	x0, x1, w2, x3
	smaddl	x0, w1, x2, x3
	smaddl	x0, w1, w2, w3

	ld1	{v1.s, v2.s}[1], [x3]
	st1	{v2.s, v3.s}[1], [x4]
	ld2	{v1.s, v2.s, v3.s}[1], [x3]
	st2	{v2.s, v2.s, v3.s}[1], [x4]
	ld3	{v1.s, v2.s, v3.s, v4.s}[1], [x3]
	st3	{v2.s, v3.s, v4.s, v5.s}[1], [x4]
	ld4	{v1.s}[1], [x3]
	st4	{v2.s}[1], [x4]

	ld2	{v1.b, v3.b}[1], [x3]
	st2	{v2.b, v4.b}[1], [x4]
	ld3	{v1.b, v3.b, v5.b}[1], [x3]
	st3	{v2.b, v4.b, v6.b}[1], [x4]
	ld4	{v1.b, v3.b, v5.b, v7.b}[1], [x3]
	st4	{v2.b, v4.b, v6.b, v8.b}[1], [x4]

	ld1	{v1.q}[1], [x3]

	ld1r	{v1.4s, v3.4s}, [x3]
	ld1r	{v1.4s, v2.4s, v3.4s}, [x3]
	ld2r	{v1.4s, v2.4s, v3.4s}, [x3]
	ld3r	{v1.4s, v2.4s, v3.4s, v4.4s}, [x3]
	ld4r	{v1.4s}, [x3]

	ld1r	{v1.4s, v3.4s}, [x3], x4
	ld1r	{v1.4s, v2.4s, v3.4s}, [x3], x4
	ld2r	{v1.4s, v2.4s, v3.4s}, [x3], x4
	ld3r	{v1.4s, v2.4s, v3.4s, v4.4s}, [x3], x4
	ld4r	{v1.4s}, [x3], x4

	ld1r	{v1.4s}, [x3], #1
	ld1r	{v1.4s, v2.4s}, [x3], #8
	ld2r	{v1.4s, v2.4s}, [x3], #4
	ld3r	{v1.4s, v2.4s, v3.4s}, [x3], #16
	ld4r	{v1.4s, v2.4s, v3.4s, v4.4s}, [x3], #32

	addp	s1, v2.2s
	addp	s1, v2.2d
	addp	d1, v2.2s
	fmaxp	s1, v2.4s

	add	s1, s2, s3
	cmhi	d1, d2, s3

	shll	v0.8h, v1.8b, 16
	shll2	v0.2d, v1.4s, 16

	dup	s1, v2.d[1]
	dup	s1, v2.s[4]
	mov	s1, v2.h[1]

	clrex	#16

	msr     daif, w5
	mrs	w15, midr_el1
	mrs	x0, dummy

	sshr	v0.4s, v1.4s, #0
	sshr	v0.4s, v1.4s, #33
	sshr	v0.4h, v1.4h, #20

	shl	v0.4s, v1.4s, #32
	fcvtzs	v0.2h, v1.2h, #2
	uqshrn	v0.2s, v1.2d, 33
	uqrshrn	v0.2s, v1.2s, 32
	sshll	v8.8h, v2.8b, #8

	sysl	x7, #10, C15, C7, #11
	sysl	w7, #1, C15, C7, #1

	dsb	dummy
	dmb	#16
	isb	osh

	prfm	0x2f, LABEL1
	prfm	pldl3strm, [sp, #8]!
	prfm	pldl3strm, [sp], #8
	prfm	pldl3strm, [sp, w0, sxtw #3]!
	prfm	pldl3strm, =0x100

	sttr	x0, LABEL1
	sttr	x0, [sp, #16]!
	sttr	x0, [sp], #16
	sttr	x0, [sp, x1]

	ldur	x0, LABEL1
	ldur	x0, [sp, #16]!
	ldur	x0, [sp], #16
	ldur	x0, [sp, x1]

	ldr	b0, =0x100
	ldr	h0, LABEL1

	ic	ivau
	ic	ivau, w0
	ic	ialluis, xzr
	ic	ialluis, x0
	sys	#0, c0, c0, 0, w0
	msr	spsel, #16
	msr	cptr_el2, #15

	movz	x1,#:abs_g2:u48, lsl #16
	movz	x1, 0xddee, lsl #8
	movz	w1,#:abs_g2:u48
	movz	w1,#:abs_g3:u48
	movk	x1,#:abs_g1_s:s12

	movi	v0.4s, #256
	movi	v0.2d, #0xabcdef

	bic	v0.4s, #255, msl #8
	bic	v0.4s, #512
	bic	v0.4s, #1, lsl #31
//	bic	v0.4h, #1, lsl #16

	orr	v0.4s, #255, msl #8
	orr	v0.4s, #512

	movi	v0.4s, #127, lsl #4
	movi	v0.4s, #127, msl #24
//	movi	v0.4h, #127, lsl #16

	mvni	v0.4s, #127, lsl #4
	mvni	v0.4s, #127, msl #24
//	mvni	v0.4h, #127, lsl #16

	fmov	v0.2s, #3.1415926
	fmov	v0.4s, #3.1415926
	fmov	v0.2d, #3.1415926
	fmov	x0, #1.0
	fmov	w0, w1

	msr	#5, #0
	msr	SPSel, #2

	tbl	v0.16b, {v1.16b, v3.16b, v5.16b}, v2.16b
	tbx	v0.8b, {v1.16b, v3.16b, v5.16b, v7.16b}, v2.8b

	// Alternating register list forms are no longer available A64 ISA

	.macro ldst2_reg_list_post_imm_reg_64 inst type postreg
	\inst\()2 {v0.\type, v2.\type}, [x0], #16
	\inst\()2 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], #32
	.ifnb \postreg
	\inst\()2 {v0.\type, v2.\type}, [x0], \postreg
	\inst\()2 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], \postreg
	.endif
	.endm

	.macro ldst2_reg_list_post_imm_reg_128 inst type postreg
	\inst\()2 {v0.\type, v2.\type}, [x0], #32
	\inst\()2 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], #64
	.ifnb \postreg
	\inst\()2 {v0.\type, v2.\type}, [x0], \postreg
	\inst\()2 {v0.\type, v1.\type, v2.\type, v3.\type}, [x0], \postreg
	.endif
	.endm

	.irp instr ld,st
	.irp bits_64 8b, 4h, 2s
	ldst2_reg_list_post_imm_reg_64 \instr \bits_64 x7
	.endr
	.endr

	.irp instr ld,st
	.irp bits_128 16b, 8h, 4s, 2d
	ldst2_reg_list_post_imm_reg_128 \instr \bits_128 x7
	.endr
	.endr

	.macro ldst34_reg_list_post_imm_reg_64 inst type postreg
	\inst\()3 {v0.\type, v2.\type, v4.\type}, [x0], #24
	\inst\()4 {v0.\type, v2.\type, v4.\type, v6.\type}, [x0], #32
	\inst\()3 {v0.\type, v2.\type, v4.\type}, [x0], \postreg
	\inst\()4 {v0.\type, v2.\type, v4.\type, v6.\type}, [x0], \postreg
	.endm

	.macro ldst34_reg_list_post_imm_reg_128 inst type postreg
	\inst\()3 {v0.\type, v2.\type, v4.\type}, [x0], #48
	\inst\()4 {v0.\type, v2.\type, v4.\type, v6.\type}, [x0], #64
	\inst\()3 {v0.\type, v2.\type, v4.\type}, [x0], \postreg
	\inst\()4 {v0.\type, v2.\type, v4.\type, v6.\type}, [x0], \postreg
	.endm

	.irp instr ld,st
	.irp bits_64 8b, 4h, 2s
	ldst34_reg_list_post_imm_reg_64 \instr \bits_64 x7
	.endr
	.endr

	.irp instr ld,st
	.irp bits_128 16b, 8h, 4s, 2d
	ldst34_reg_list_post_imm_reg_128 \instr \bits_128 x7
	.endr
	.endr

	// LD1R expects one register only.

	ld1r {v0.8b, v1.8b}, [x0], #1
	ld1r {v0.16b, v1.16b}, [x0], #1
	ld1r {v0.4h, v1.4h}, [x0], #2
	ld1r {v0.8h, v1.8h}, [x0], #2
	ld1r {v0.2s, v1.2s}, [x0], #4
	ld1r {v0.4s, v1.4s}, [x0], #4
	ld1r {v0.1d, v1.1d}, [x0], #8
	ld1r {v0.2d, v1.2d}, [x0], #8

	.macro ldstn_index_rep_H_altreg_imm inst index type rep
	\inst\()2\rep {v0.\type, v2.\type}\index, [x0], #4
	\inst\()3\rep {v0.\type, v2.\type, v4.\type}\index, [x0], #6
	\inst\()4\rep {v0.\type, v2.\type, v4.\type, v6.\type}\index, [x0], #8
	.endm

	.irp instr, ld, st
	ldstn_index_rep_H_altreg_imm  \instr index="[1]" type=h rep=""
	.ifnc \instr, st
	.irp types 4h, 8h
	ldstn_index_rep_H_altreg_imm  \instr index="" type=\types rep="r"
	.endr
	.endif
	.endr

	.macro ldstn_index_rep_S_altreg_imm inst index type rep
	\inst\()2\rep {v0.\type, v2.\type}\index, [x0], #8
	\inst\()3\rep {v0.\type, v2.\type, v4.\type}\index, [x0], #12
	\inst\()4\rep {v0.\type, v2.\type, v4.\type, v6.\type}\index, [x0], #16
	.endm

	.irp instr, ld, st
	ldstn_index_rep_S_altreg_imm  \instr index="[1]" type=s rep=""
	.ifnc \instr, st
	.irp types 2s, 4s
	ldstn_index_rep_S_altreg_imm  \instr index="" type=\types rep="r"
	.endr
	.endif
	.endr

	.macro ldstn_index_rep_D_altreg_imm inst index type rep
	\inst\()2\rep {v0.\type, v2.\type}\index, [x0], #16
	\inst\()3\rep {v0.\type, v2.\type, v4.\type}\index, [x0], #24
	\inst\()4\rep {v0.\type, v2.\type, v4.\type, v6.\type}\index, [x0], #32
	.endm

	.irp instr, ld, st
	ldstn_index_rep_D_altreg_imm  \instr index="[1]" type=d rep=""
	.ifnc \instr, st
	.irp types 1d, 2d
	ldstn_index_rep_D_altreg_imm  \instr index="" type=\types rep="r"
	.endr
	.endif
	.endr

	.irp type 8b, 16b, 4h, 8h, 2s, 4s, 1d, 2d
	ld1r {v0.\type, v1.\type}, [x0], x7
	.endr

	.macro ldstn_index_rep_reg_altreg inst index type rep postreg
	\inst\()2\rep {v0.\type, v2.\type}\index, [x0], \postreg
	\inst\()3\rep {v0.\type, v2.\type, v4.\type}\index, [x0], \postreg
	\inst\()4\rep {v0.\type, v2.\type, v4.\type, v6.\type}\index, [x0], \postreg
	.endm

	.irp instr, ld, st
	.irp itypes b,h,s,d
	ldstn_index_rep_reg_altreg  \instr index="[1]" type=\itypes rep="" postreg=x7
	.endr
	.ifnc \instr, st
	.irp types 8b, 16b, 4h, 8h, 2s, 4s, 1d, 2d
	ldstn_index_rep_reg_altreg  \instr index="" type=\types rep="r" postreg=x7
	.endr
	.endif
	.endr

	.macro ldnstn_reg_list type inst index rep
	.ifb \index
	.ifnb \rep
	\inst\()1\rep {v0.\type, v1.\type}\index, [x0]
	.endif
	.endif

	.ifnc \type, B
	\inst\()2\rep {v0.\type, v2.\type}\index, [x0]
	.endif

	.ifnc \type, B
	\inst\()3\rep {v0.\type, v2.\type, v4.\type}\index, [x0]
	.endif

	.ifnc \type, B
	\inst\()4\rep {v0.\type, v2.\type, v4.\type, v6.\type}\index, [x0]
	.endif

	.endm

	ldnstn_reg_list type="8B", inst="ld" index="" rep=""
	ldnstn_reg_list type="8B", inst="st" index="" rep=""

	ldnstn_reg_list type="16B", inst="ld" index="" rep=""
	ldnstn_reg_list type="16B", inst="st" index="" rep=""

	ldnstn_reg_list type="4H", inst="ld" index="" rep=""
	ldnstn_reg_list type="4H", inst="st" index="" rep=""

	ldnstn_reg_list type="8H", inst="ld" index="" rep=""
	ldnstn_reg_list type="8H", inst="st" index="" rep=""

	ldnstn_reg_list type="2S", inst="ld" index="" rep=""
	ldnstn_reg_list type="2S", inst="st" index="" rep=""

	ldnstn_reg_list type="4S", inst="ld" index="" rep=""
	ldnstn_reg_list type="4S", inst="st" index="" rep=""

	ldnstn_reg_list type="2D", inst="ld" index="" rep=""
	ldnstn_reg_list type="2D", inst="st" index="" rep=""

	ldnstn_reg_list type="B", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="B", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="B", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="B", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="H", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="H", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="H", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="H", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="S", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="S", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="S", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="S", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="D", inst="ld" index="[1]" rep=""
	ldnstn_reg_list type="D", inst="st" index="[1]" rep=""

	ldnstn_reg_list type="8B", inst="ld" index="" rep="r"

	ldnstn_reg_list type="16B", inst="ld" index="" rep="r"

	ldnstn_reg_list type="4H", inst="ld" index="" rep="r"

	ldnstn_reg_list type="8H", inst="ld" index="" rep="r"

	ldnstn_reg_list type="2S", inst="ld" index="" rep="r"

	ldnstn_reg_list type="4S", inst="ld" index="" rep="r"

	ldnstn_reg_list type="1D", inst="ld" index="" rep="r"

	ldnstn_reg_list type="2D", inst="ld" index="" rep="r"

	pmull	v0.1q, v1.1d, v2.1d
	pmull2	v0.1q, v1.2d, v2.2d

	// #<fbits> out of range
	.irp instr, scvtf, ucvtf
	\instr	d0, w1, 33
	\instr	s0, w0, 33
	\instr	d0, x1, 65
	\instr	s0, x1, 65
	.endr
	.irp instr, fcvtzs, fcvtzu
	\instr	w1, d0, 33
	\instr	w0, s0, 33
	\instr	x1, d0, 65
	\instr	x1, s0, 65
	.endr

	// Invalid instruction.
	mockup-op


	ldrh	w0, [x1, x2, lsr #1]

	add	w0, w1, w2, ror #1
	sub	w0, w1, w2, asr #32
	eor	w0, w1, w2, ror #32

	add	x0, x1, #20, LSL #16
	add	x0, x1, #20, UXTX #12
	add	x0, x1, #20, LSR
	add	x0, x1, #20, LSL

	ldnp	h7, h15, [x0, #2]
	ldnp	b15, b31, [x0], #4
	ldnp	h0, h1, [x0, #6]!

	uqrshrn	h0, s1, #63
	sqshl	b7, b15, #8

	bfxil	w7, w15, #15, #30
	bfi	x3, x7, #31, #48

	str	x1,page_table_count

	prfm    PLDL3KEEP, [x9, x15, sxtx #2]

	mrs	x5, S1_0_C17_C8_0
	msr	S3_1_C13_C15_1, x7
	msr	S3_1_C11_C15_-1, x7
	msr	S3_1_11_15_1, x7

	// MOVI (alias of ORR immediate) is no longer supported.
	movi	w1, #15
.set u48, 0xaabbccddeeff

	uxtb	x7, x15
	uxth	x7, x15
	uxtw	x7, x15
	sxtb	w15, xzr
	sxth	w15, xzr
	sxtw	w15, xzr

	mov	w0, v0.b[0]
	mov	w0, v0.h[0]
	mov	w0, v0.d[0]
	mov	x0, v0.b[0]
	mov	x0, v0.h[0]
	mov	x0, v0.s[0]

	uabdl2	v20.4S, v12.8H, v29.8

	movi	d1, 0xffff, lsl #16

	ST3 {v18.D-v20.D}[0],[x28],x
	ST1 {v7.B}[2],[x4],x
	ST1 {v22.1D-v25.1D},[x10],x

	ldr	w0, [x0]!
	ldr	w0, [x0], {127}

	orr	x0, x0, #0xff, lsl #1
	orr	x0. x0, #0xff, lsl #1
	orr	x0, x0, #0xff lsl #1

	mov	x0, ##5

	msr	daifset, x0
	msr	daifclr, x0

	fmov	s0, #0x11
	fmov	s0, #0xC0280000C1400000
	fmov	d0, #0xC02f800000000000

	// No 16-byte relocation
	ldr	q0, =one_label

	ands	w0, w24, #0xffeefffffffffffd

one_label:

	cinc	w0, w1, al
	cinc	w0, w1, nv
	cset	w0, al
	cset	w0, nv
	cinv	w0, w1, al
	cinv	w0, w1, nv
	csetm	w0, al
	csetm	w0, nv
	cneg	w0, w1, al
	cneg	w0, w1, nv

	mrs	x5, S4_0_C12_C8_0
	mrs	x6, S0_8_C11_C7_5
	mrs	x7, S1_1_C16_C6_6
	mrs	x8, S2_2_C15_C16_7
	mrs	x9, S3_3_C14_C15_8

	fmov	s0, #-0.0
	fmov	s0, #0x40000000 // OK
	fmov	s0, #0x80000000
	fmov	s0, #0xc0000000 // OK
	fmov	d0, #-0.0
	fmov	d0, #0x4000000000000000 // OK
	fmov	d0, #0x8000000000000000
	fmov	d0, #0xc000000000000000 // OK

	fcmgt	v0.4s, v0.4s, #0.0 // OK
	fcmgt	v0.4s, v0.4s, #0 // OK
	fcmgt	v0.4s, v0.4s, #-0.0
	fcmgt	v0.2d, v0.2d, #0.0 // OK
	fcmgt	v0.2d, v0.2d, #0 // OK
	fcmgt	v0.2d, v0.2d, #-0.0

	# PR 20319: FMOV instructions changing the size from 32 bits
	# to 64 bits and vice versa are illegal.
	fmov 	s9, x0
	fmov	d7, w1

	st1 {v0.16b}[0],[x0]
	st2 {v0.16b-v1.16b}[1],[x0]
	st3 {v0.16b-v2.16b}[2],[x0]
	st4 {v0.8b-v3.8b}[4],[x0]

	prfm	#0x18, [sp, x15, lsl #0]
	prfm	#0x1f, [sp, x15, lsl #0]
	prfm	#0x20, [sp, x15, lsl #0]
	prfm	#0x20, FOO

	// End (for errors during literal pool generation)
