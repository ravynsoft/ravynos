	.text
	.globl	_start
	.type	_start,@function
_start:
	movi20 #foo@GOT, r0
	mov.l @(r0,r12), r1
	.data
	.type	foo,@object
	.size	foo,4
foo:
	.long 1
