	.text
	bne	r4, r5, 1f
	.zero 0x10000
1:
	ret
