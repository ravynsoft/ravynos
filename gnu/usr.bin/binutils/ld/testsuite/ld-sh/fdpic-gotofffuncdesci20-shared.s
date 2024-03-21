	.text
	.globl	f
	.type	f,@function
f:
	movi20 #foo@GOTOFFFUNCDESC, r1
	add r12, r1
	.type	foo,@function
foo:
	nop
