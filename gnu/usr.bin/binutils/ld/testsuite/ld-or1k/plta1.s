	.data
	.p2align 16

	.text
	.globl	_start
_start:

	l.j plta(x)
	 l.nop
	l.j plta(y)
	 l.nop
