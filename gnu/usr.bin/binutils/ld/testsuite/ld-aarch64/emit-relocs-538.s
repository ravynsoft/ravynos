	.global	v1
	.global	v2
	.size	v2, 32768
	.global	v3
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
	.word	0xdeaddead
v2:
	.zero	32768
v3:
	.word	0xcafecafe
	.word	0xcafecafe

	.text
	ldr	x2, [x4, #:dtprel_lo12_nc:v2]

	# should not issue overflow error.
	ldr	x14, [x17, #:dtprel_lo12_nc:v3]
