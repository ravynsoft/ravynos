	.sdata
c0101:	.word	0xabcd

	.text
	.align	4
test:
	lw	$2, c0101
