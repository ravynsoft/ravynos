	.globl	x1
	.csect	x1[RW]
x1:
	.long	0x0102

	.weak	x2
	.csect	x2[RW]
x2:
	.long	0x0304

	.toc
Tx1:
	.tc	x1[TC],x1
Tx2:
	.tc	x2[TC],x2
Tx3:
	.tc	x3[TC],x3

	.globl	.main
	.csect	.main[PR]
.main:
	.if     size == 32
	lwz     1,Tx1(2)
	lwz     1,Tx2(2)
	lwz     1,Tx3(2)
	.else
	ld      1,Tx1(2)
	ld      1,Tx2(2)
	ld      1,Tx3(2)
	.endif
