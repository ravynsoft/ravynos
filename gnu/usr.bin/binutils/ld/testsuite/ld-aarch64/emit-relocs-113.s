	.global	v1
	.global	v2
	.size	v2, 4100
	.global	v3
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
v2:
	.zero	4100
v3:
	.word	0xcafecafe

	.text
	ldrsb	x29, [x4, #:tprel_lo12_nc:v2]

	# should not issue overflow error.
	ldrsb	x18, [x7, #:tprel_lo12_nc:v3]
