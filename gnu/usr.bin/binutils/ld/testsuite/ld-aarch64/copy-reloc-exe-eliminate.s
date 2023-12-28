	.global	p
	.section	.data.rel.ro,"aw",%progbits
	.align	3
	.type	p, %object
	.size	p, 8
p:
	.xword	global_a
