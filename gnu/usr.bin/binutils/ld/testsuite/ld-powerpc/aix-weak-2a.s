	.weak	c1
	c1 = 0xcccc1111
	.weak	c2
	c2 = 0xcccc2222
	.globl	c3
	c3 = 0xcccc3333
	.globl	c4
	c4 = 0xcccc4444

	.weak	d1
	d1 = 0xffff1111
	.weak	d2
	d2 = 0xffff2222
	.globl	d3
	.csect	d3[DS]
d3:
	.long	0xffff3333
	.globl	d4
	.csect	d4[DS]
d4:
	.long	0xffff4444
