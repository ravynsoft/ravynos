	.globl	_start
	.type	_start, @function
_start:
	movl	_text@GOT(%ecx), %eax
	.size	_start, .-_start
