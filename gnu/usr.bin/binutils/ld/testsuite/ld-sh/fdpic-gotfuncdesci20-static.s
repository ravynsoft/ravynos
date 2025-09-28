	.text
	.globl	_start
	.type	_start,@function
_start:
	movi20 #foo@GOTFUNCDESC, r0
	mov.l @(r0,r12), r1
	.type	foo,@function
foo:
	nop
