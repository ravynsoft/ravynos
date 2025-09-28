 .text
 .globl	_start
 .globl	__start
_start:
__start:
 .ifdef	HPUX
one	.comm	1
 .else
	.comm	one, 1, 1
 .endif
