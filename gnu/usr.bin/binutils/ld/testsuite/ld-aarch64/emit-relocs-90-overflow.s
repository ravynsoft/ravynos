	.global	v1
	.size	v1, 16777216
	.global	v2
	.section	.tdata,"awT",%progbits
v1:
	.zero	16777216
v2:
	.word	0xcafecafe

	.text
	add	w20, w9, #:dtprel_hi12:v2
