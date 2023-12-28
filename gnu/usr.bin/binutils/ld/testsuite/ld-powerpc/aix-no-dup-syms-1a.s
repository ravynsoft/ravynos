	.globl	x
	.csect	x[RW]
x:
	.long	4
	.globl	x1
	.csect	x1[RW]
x1:
	.long	x
	.long	foo
