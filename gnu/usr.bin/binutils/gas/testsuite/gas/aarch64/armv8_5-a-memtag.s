func:
	# OP x[0,30], x[0,30], x[0,30]
	.macro expand_3_reg op
	\op x0, x0, x0
	\op x27, x0, x0
	\op x0, x27, x0
	\op x0, x0, x27
	\op x27, x27, x27
	.endm

	# OP x[0,30], x[0,30], #[0,30], #[0,14]
	.macro expand_2_reg op
	\op x0, x0, #0, #0
	\op x27, x0, #0, #0
	\op x0, x27, #0, #0
	\op x27, x27, #0, #0
	.endm

	.macro expand_stg op
	\op x0, [x0, #0]
	\op x0, [x27, #0]
	\op sp, [x0, #0]
	\op x27, [x0, #-80]
	\op x0, [x0, #0]!
	\op sp, [x0, #0]!
	\op x27, [x0, #160]!
	\op x0, [x0], #0
	\op sp, [x0], #0
	\op x27, [x0], #-1440
	\op x0, [sp, #4080]
	\op sp, [sp, #4080]
	\op x27, [sp, #-4096]
	\op x0, [sp, #4080]!
	\op sp, [sp], #-4096
	.endm

	.macro expand_ldg_bulk op
	\op x0, [x0]
	\op x27, [x0]
	\op x0, [x27]
	\op x25, [x27]
	\op x0, [sp]
	\op xzr, [x0]
	.endm

	# IRG
	expand_3_reg irg
	irg sp, x0
	irg x0, sp

	# GMI
	expand_3_reg gmi
	gmi x0, sp, x0
	gmi xzr, x0, x0

	# ADDG
	expand_2_reg addg
	addg x0, sp, #0x3f0, #0xf
	addg sp, x0, #0x2a0, #0xf

	# SUBG
	expand_2_reg subg
	subg x0, sp, #0x3f0, #0xf
	subg sp, x0, #0x3f0, #0x5

	# SUBP
	expand_3_reg subp
	subp x0, sp, x0
	subp x0, x0, sp
	subp xzr, x0, x0

	# SUBPS
	expand_3_reg subps
	subps x0, sp, x0
	subps x0, x0, sp
	subps xzr, x0, x0

	# CMPP
	cmpp x0, x0
	cmpp x27, x0
	cmpp x0, x27
	cmpp x27, x27
	cmpp sp, x0
	cmpp x0, sp

	expand_stg stg
	expand_stg stzg
	expand_stg st2g
	expand_stg stz2g

	stgp x0, x0, [x0, #0]
	stgp x0, x27, [x0, #0]
	stgp x27, x0, [x0, #0]
	stgp x27, x27, [x0, #0]
	stgp x0, x0, [x27, #0]
	stgp x0, x0, [x0, #-80]
	stgp x0, x0, [x0, #0]!
	stgp x0, x0, [x0, #160]!
	stgp x0, x0, [x0], #0
	stgp x0, x0, [x0], #-144
	stgp xzr, x0, [x0, #1008]
	stgp x0, xzr, [x0, #-1024]
	stgp x0, x0, [sp, #1008]!
	stgp x0, x0, [sp], #-1024

	ldg x0, [x0, #0]
	ldg x27, [x0, #0]
	ldg x0, [x27, #0]
	ldg x27, [x27, #0]
	ldg x0, [sp, #0]
	ldg xzr, [x0, #0]
	ldg x0, [x0, #4080]
	ldg x0, [x0, #-4096]

	expand_ldg_bulk stzgm
	expand_ldg_bulk ldgm
	expand_ldg_bulk stgm
