# Check that CFI directives can accept all of the register names (including
# aliases).  The results for this test also ensures that the DWARF
# register numbers for the GPRs/FPRs/vector registers shouldn't change.
# Note that, because vector register size is "variable" in principle,
# vector registers are very unlikely to be used within .cfi_offset directive.

	.text
	.global _start
_start:
	.cfi_startproc
	nop

	# GPRs (ABI)
	.cfi_offset zero,  4
	.cfi_offset ra,    8
	.cfi_offset sp,   12
	.cfi_offset gp,   16
	.cfi_offset tp,   20
	.cfi_offset t0,   24
	.cfi_offset t1,   28
	.cfi_offset t2,   32
	.cfi_offset s0,   36
	.cfi_offset s1,   40
	.cfi_offset a0,   44
	.cfi_offset a1,   48
	.cfi_offset a2,   52
	.cfi_offset a3,   56
	.cfi_offset a4,   60
	.cfi_offset a5,   64
	.cfi_offset a6,   68
	.cfi_offset a7,   72
	.cfi_offset s2,   76
	.cfi_offset s3,   80
	.cfi_offset s4,   84
	.cfi_offset s5,   88
	.cfi_offset s6,   92
	.cfi_offset s7,   96
	.cfi_offset s8,  100
	.cfi_offset s9,  104
	.cfi_offset s10, 108
	.cfi_offset s11, 112
	.cfi_offset t3,  116
	.cfi_offset t4,  120
	.cfi_offset t5,  124
	.cfi_offset t6,  128

	# GPR (ABI alias)
	.cfi_offset fp,   36

	# GPRs (Numeric)
	.cfi_offset x0,    4
	.cfi_offset x1,    8
	.cfi_offset x2,   12
	.cfi_offset x3,   16
	.cfi_offset x4,   20
	.cfi_offset x5,   24
	.cfi_offset x6,   28
	.cfi_offset x7,   32
	.cfi_offset x8,   36
	.cfi_offset x9,   40
	.cfi_offset x10,  44
	.cfi_offset x11,  48
	.cfi_offset x12,  52
	.cfi_offset x13,  56
	.cfi_offset x14,  60
	.cfi_offset x15,  64
	.cfi_offset x16,  68
	.cfi_offset x17,  72
	.cfi_offset x18,  76
	.cfi_offset x19,  80
	.cfi_offset x20,  84
	.cfi_offset x21,  88
	.cfi_offset x22,  92
	.cfi_offset x23,  96
	.cfi_offset x24, 100
	.cfi_offset x25, 104
	.cfi_offset x26, 108
	.cfi_offset x27, 112
	.cfi_offset x28, 116
	.cfi_offset x29, 120
	.cfi_offset x30, 124
	.cfi_offset x31, 128

	# FPRs (ABI)
	.cfi_offset ft0,  132
	.cfi_offset ft1,  136
	.cfi_offset ft2,  140
	.cfi_offset ft3,  144
	.cfi_offset ft4,  148
	.cfi_offset ft5,  152
	.cfi_offset ft6,  156
	.cfi_offset ft7,  160
	.cfi_offset fs0,  164
	.cfi_offset fs1,  168
	.cfi_offset fa0,  172
	.cfi_offset fa1,  176
	.cfi_offset fa2,  180
	.cfi_offset fa3,  184
	.cfi_offset fa4,  188
	.cfi_offset fa5,  192
	.cfi_offset fa6,  196
	.cfi_offset fa7,  200
	.cfi_offset fs2,  204
	.cfi_offset fs3,  208
	.cfi_offset fs4,  212
	.cfi_offset fs5,  216
	.cfi_offset fs6,  220
	.cfi_offset fs7,  224
	.cfi_offset fs8,  228
	.cfi_offset fs9,  232
	.cfi_offset fs10, 236
	.cfi_offset fs11, 240
	.cfi_offset ft8,  244
	.cfi_offset ft9,  248
	.cfi_offset ft10, 252
	.cfi_offset ft11, 256

	# FPRs (Numeric)
	.cfi_offset f0,  132
	.cfi_offset f1,  136
	.cfi_offset f2,  140
	.cfi_offset f3,  144
	.cfi_offset f4,  148
	.cfi_offset f5,  152
	.cfi_offset f6,  156
	.cfi_offset f7,  160
	.cfi_offset f8,  164
	.cfi_offset f9,  168
	.cfi_offset f10, 172
	.cfi_offset f11, 176
	.cfi_offset f12, 180
	.cfi_offset f13, 184
	.cfi_offset f14, 188
	.cfi_offset f15, 192
	.cfi_offset f16, 196
	.cfi_offset f17, 200
	.cfi_offset f18, 204
	.cfi_offset f19, 208
	.cfi_offset f20, 212
	.cfi_offset f21, 216
	.cfi_offset f22, 220
	.cfi_offset f23, 224
	.cfi_offset f24, 228
	.cfi_offset f25, 232
	.cfi_offset f26, 236
	.cfi_offset f27, 240
	.cfi_offset f28, 244
	.cfi_offset f29, 248
	.cfi_offset f30, 252
	.cfi_offset f31, 256

	# Vector registers (numeric only)
	.cfi_offset v0,  388
	.cfi_offset v1,  392
	.cfi_offset v2,  396
	.cfi_offset v3,  400
	.cfi_offset v4,  404
	.cfi_offset v5,  408
	.cfi_offset v6,  412
	.cfi_offset v7,  416
	.cfi_offset v8,  420
	.cfi_offset v9,  424
	.cfi_offset v10, 428
	.cfi_offset v11, 432
	.cfi_offset v12, 436
	.cfi_offset v13, 440
	.cfi_offset v14, 444
	.cfi_offset v15, 448
	.cfi_offset v16, 452
	.cfi_offset v17, 456
	.cfi_offset v18, 460
	.cfi_offset v19, 464
	.cfi_offset v20, 468
	.cfi_offset v21, 472
	.cfi_offset v22, 476
	.cfi_offset v23, 480
	.cfi_offset v24, 484
	.cfi_offset v25, 488
	.cfi_offset v26, 492
	.cfi_offset v27, 496
	.cfi_offset v28, 500
	.cfi_offset v29, 504
	.cfi_offset v30, 508
	.cfi_offset v31, 512

	nop
	.cfi_endproc
