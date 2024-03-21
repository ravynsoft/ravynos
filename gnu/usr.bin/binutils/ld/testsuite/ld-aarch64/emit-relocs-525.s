	.global	v1
	.size	v1, 65536
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.zero	65536
v2:
	.word	0xcafecafe

	.text
	movk	x17, #:dtprel_g1_nc:v2
