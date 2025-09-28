	.global	v1
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
v2:
	.word	0xcafecafe

	.text
	ldrsh	x11, [x7, #:tprel_lo12:v2]
