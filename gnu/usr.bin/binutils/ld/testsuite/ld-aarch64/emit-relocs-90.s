	.global	v1
	.size v1, 409600
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.zero	409600
v2:
	.word	0xcafecafe

	.text
	add	w20, w9, #:dtprel_hi12:v2
