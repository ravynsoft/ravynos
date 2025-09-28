	.cpu generic+fp+simd
	.global	p
	.comm	x,4,4
	.section	.rodata
	.align	3
	.type	p, %object
	.size	p, 8
p:
	.xword	x
