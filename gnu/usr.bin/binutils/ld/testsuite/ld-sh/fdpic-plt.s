	.text
	.globl	f
	.type	f,@function
f:
	mov.l .L1, r0
	mov.l .L2, r1
	.align 2
.L1:
	.long	foo@PLT
.L2:
	.long	bar@PLT
