	.global	v1
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.zero	65536
v2:
	.word	0xcafecafe

	.text
	movk	x21, #:dtprel_g0:v2
