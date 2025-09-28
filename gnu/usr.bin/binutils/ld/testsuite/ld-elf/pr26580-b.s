 .text
 .globl	_start
 .globl	__start
_start:
__start:
 .ifdef	HPUX
one	.comm	8
 .else
	.comm	one, 8, 8
 .endif
