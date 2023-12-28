	.syntax unified
	.text
	.thumb
foo:
	# OP0 : Unsigned, 5-bit, even
	bfx   0, r3
	bfx  -2, r3
	bflx  3, r3
	bflx 32, r3
	# OP1 : no SP or PC
	bfx   2, PC
	bflx  2, SP
