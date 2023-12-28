	.global	v1
	.size	v1, 4096
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.zero	4096
v2:
	.word	0xcafecafe

	.text
	ldrsb	x22, [x1, #:tprel_lo12:v2]
