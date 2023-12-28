	.globl	x
	.csect	x[RW]
x:
	.long	8
	.globl	x2
	.csect	x2[RW]
x2:
	.long	x
	.long	foo
