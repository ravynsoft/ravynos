	.section .rodata.cst4,"aM",@progbits,4
	.set	x,0x01000000
	.set	y,0x02000000
	# Add the 16 values that the next input file wants, but in such
	# a way that each one lives on a separate page.
	.rept	15
	.word	y
	.set	y,y+1
	.rept	0x4000
	.word	x
	.set	x,x+1
	.endr
	.endr
	.word	y
