	.global	v1
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
v2:
	.word	0xcafecafe

	.text
	ldrsb	x21, [x8, #:dtprel_lo12:v2]
