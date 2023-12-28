	.global	v1
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
	.word	0xdeaddead
v2:
	.word	0xcafecafe
	.word	0xcafecafe

	.text
	ldr	x0, [x9, #:tprel_lo12:v2]
