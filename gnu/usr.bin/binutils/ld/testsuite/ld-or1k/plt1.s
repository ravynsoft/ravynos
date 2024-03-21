	.data
	.p2align 16

	.text
	.globl	_start
_start:

	l.j plt(x)
	 l.nop
	l.j plt(y)
	 l.nop
