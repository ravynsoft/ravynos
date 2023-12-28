	.text
test1:
	.space	65536
test2:
	nop
	b16	1f
1:
	nop
	bnez16	$2,1f
1:
	nop
	beqz16	$2,1f
1:
	nop
	b	1f
1:
	nop
	bnez	$2,1f
1:
	nop
	beqz	$2,1f
1:
	nop
