	.text
	.globl	f
	.type	f,@function
f:
	movi20 #foo@GOT, r0
	mov.l @(r0,r12), r1
