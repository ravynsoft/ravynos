	.text
	.globl	_start
	.type	_start,@function
_start:
	movi20 #foo@GOTOFFFUNCDESC, r1
	add r12, r1
	.type	foo,@function
foo:
	nop
