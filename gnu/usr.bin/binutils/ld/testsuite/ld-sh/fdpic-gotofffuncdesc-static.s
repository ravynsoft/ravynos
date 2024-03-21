	.text
	.globl	_start
	.type	_start,@function
_start:
	mov.l .L1, r1
	add r12, r1
	.align 2
.L1:
	.long	foo@GOTOFFFUNCDESC
	.type	foo,@function
foo:
	nop
