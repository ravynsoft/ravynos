	.globl	x1
	x1 = 0x11223344
	.globl	x2
	x2 = 0x55667788
	.globl	x3
	.csect	x3[RW]
x3:
	.long	42
