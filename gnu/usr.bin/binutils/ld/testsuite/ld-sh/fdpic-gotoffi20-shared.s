	.text
	.globl	f
	.type	f,@function
f:
	movi20 #foo@GOTOFF, r0
	mov.l @(r0,r12), r1
	.data
	.type	foo,@object
	.size	foo,4
foo:
	.long	1
