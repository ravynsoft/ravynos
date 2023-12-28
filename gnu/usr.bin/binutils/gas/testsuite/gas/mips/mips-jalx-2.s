text_sym:
	.text
	jalx	1f
	
	.set	mips16
	.align	1
1:	nop
	.set	nomips16
	.align	2
	.fill	8
