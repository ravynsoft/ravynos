	.text
	.globl	f
	.type	f,@function
f:
	mov.l .L1, r0
	mov.l @(r0,r12), r1
	.align 2
.L1:
	.long	foo@GOTFUNCDESC
