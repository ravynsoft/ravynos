	.global	v1
	.global	v2
	.size	v2, 16384
	.global	v3
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
v2:
	.zero	16384
v3:
	.word	0xcafecafe

	.text
	ldrsw	x22, [x14, #:dtprel_lo12_nc:v2]

	# should not issue overflow error.
	ldrsw	x8, [x17, #:dtprel_lo12_nc:v3]
