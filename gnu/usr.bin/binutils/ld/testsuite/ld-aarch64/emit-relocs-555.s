	.global	v1
	.global	v2
	.size	v2, 5000
	.global	v3
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
v2:
	.zero	5000
v3:
	.word	0xcafecafe

	.text
	ldrsh	x22, [x14, #:tprel_lo12_nc:v2]

	# should not issue overflow error.
	ldrsh	x8, [x17, #:tprel_lo12_nc:v3]
