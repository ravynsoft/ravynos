	.globl	c1
	c1 = 0xdddd1111
	.weak	c2
	c2 = 0xdddd2222
	.globl	c3
	c3 = 0xdddd3333
	.weak	c4
	c4 = 0xdddd4444

	.globl	d1
	.csect	d1[DS]
d1:
	.long	0xeeee1111
	.weak	d2
	d2 = 0xeeee2222
	.globl	d3
	.csect	d3[DS]
d3:
	.long	0xeeee3333
	.weak	d4
	d4 = 0xeeee4444
