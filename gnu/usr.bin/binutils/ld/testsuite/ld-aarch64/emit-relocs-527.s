	.global	v1
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.word	0xdeaddead
v2:
	.word	0xcafecafe

	.text
	movk	x17, #:dtprel_g0_nc:v2
