	.global	v1
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
v2:
	.word	0xcafecafe

	.text
	add	x20, x9, #:dtprel_lo12:v2
