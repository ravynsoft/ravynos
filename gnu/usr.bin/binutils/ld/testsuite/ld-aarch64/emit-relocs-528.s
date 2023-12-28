	.global	v1
	.size v1, 40960
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.zero	40960
v2:
	.word	0xcafecafe

	.text
	add	x20, x9, #:dtprel_hi12:v2
