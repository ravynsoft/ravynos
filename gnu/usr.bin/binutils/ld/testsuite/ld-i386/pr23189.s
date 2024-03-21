	.globl	_start
	.type	_start, @function
_start:
	movl	__hidden_sym@GOT(%eax), %eax
	.size	_start, .-_start
