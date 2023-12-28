// movi.s Test file for AArch64 AdvSIMD modified immediate instruction MOVI

	.text

	.macro all_64bit_mask_movi	dst
	.irp	b7, 0, 0xff00000000000000
	.irp	b6, 0, 0xff000000000000
	.irp	b5, 0, 0xff0000000000
	.irp	b4, 0, 0xff00000000
	.irp	b3, 0, 0xff000000
	.irp	b2, 0, 0xff0000
	.irp	b1, 0, 0xff00
	.irp	b0, 0, 0xff
	movi	\dst, \b7 + \b6 + \b5 + \b4 + \b3 + \b2 + \b1 + \b0
	.endr
	.endr
	.endr
	.endr
	.endr
	.endr
	.endr
	.endr
	.endm

	// MOVI <Dd>, #<imm>
	// MOVI <Vd>.2D, #<imm>
	all_64bit_mask_movi d31
	all_64bit_mask_movi v15.2d


	.macro	all_8bit_imm_movi dst, from=0, to=255
	movi	\dst, \from
	.if	\to-\from
	all_8bit_imm_movi \dst, "(\from+1)", \to
	.endif
	.endm

	// Per byte
	// MOVI <Vd>.<T>, #<imm8>
	.irp	T, 8b, 16b
	all_8bit_imm_movi v15.\T, 0, 63
	all_8bit_imm_movi v15.\T, 64, 127
	all_8bit_imm_movi v15.\T, 128, 191
	all_8bit_imm_movi v15.\T, 192, 255
	.endr


	.macro	all_8bit_imm_movi_sft dst, from=0, to=255, shift_op, amount
	movi	\dst, \from, \shift_op \amount
	.if	\to-\from
	all_8bit_imm_movi_sft \dst, "(\from+1)", \to, \shift_op, \amount
	.endif
	.endm

	// Shift ones, per word
	// MOVI <Vd>.<T>, #<imm8>, MSL #<amount>
	.irp	T, 2s, 4s
	.irp	amount, 8, 16
	// Have to break into four chunks to avoid "Fatal error: macros nested
	// too deeply".
	all_8bit_imm_movi_sft v7.\T, 0, 63, MSL, \amount
	all_8bit_imm_movi_sft v7.\T, 64, 127, MSL, \amount
	all_8bit_imm_movi_sft v7.\T, 128, 191, MSL, \amount
	all_8bit_imm_movi_sft v7.\T, 192, 255, MSL, \amount
	.endr
	.endr


	// Shift zeros, per halfword
	// MOVI <Vd>.<T>, #<imm8> {, LSL #<amount>}
	.irp	T, 4h, 8h
	.irp	amount, 0, 8
	all_8bit_imm_movi_sft v7.\T, 0, 63, LSL, \amount
	all_8bit_imm_movi_sft v7.\T, 64, 127, LSL, \amount
	all_8bit_imm_movi_sft v7.\T, 128, 191, LSL, \amount
	all_8bit_imm_movi_sft v7.\T, 192, 255, LSL, \amount
	all_8bit_imm_movi v15.\T, 0, 63
	all_8bit_imm_movi v15.\T, 64, 127
	all_8bit_imm_movi v15.\T, 128, 191
	all_8bit_imm_movi v15.\T, 192, 255
	.endr
	.endr


	// Shift zeros, per word
	// MOVI <Vd>.<T>, #<imm8> {, LSL #<amount>}
	.irp	T, 2s, 4s
	.irp	amount, 0, 8, 16, 24
	all_8bit_imm_movi_sft v7.\T, 0, 63, LSL, \amount
	all_8bit_imm_movi_sft v7.\T, 64, 127, LSL, \amount
	all_8bit_imm_movi_sft v7.\T, 128, 191, LSL, \amount
	all_8bit_imm_movi_sft v7.\T, 192, 255, LSL, \amount
	all_8bit_imm_movi v15.\T, 0, 63
	all_8bit_imm_movi v15.\T, 64, 127
	all_8bit_imm_movi v15.\T, 128, 191
	all_8bit_imm_movi v15.\T, 192, 255
	.endr
	.endr

	// Shift zeros, per byte
	// MOVI <Vd>.<T>, #<imm8>, LSL #0
	// This used to be a programmer-friendly feature (allowing LSL #0),
	// but it is now part of the architecture specification.
	.irp	T, 8b, 16b
	all_8bit_imm_movi_sft v7.\T, 0, 63, LSL, 0
	all_8bit_imm_movi_sft v7.\T, 64, 127, LSL, 0
	all_8bit_imm_movi_sft v7.\T, 128, 191, LSL, 0
	all_8bit_imm_movi_sft v7.\T, 192, 255, LSL, 0
	.endr

	movi	v0.2d, 18446744073709551615
	movi	v0.2d, -1
	movi	v0.2d, bignum
	movi	d31, 18446744073709551615
.set    bignum, 0xffffffffffffffff

	// Allow -128 to 255 in #<imm8>
	movi	v3.8b, -128
	movi	v3.8b, -127
	movi	v3.8b, -1
