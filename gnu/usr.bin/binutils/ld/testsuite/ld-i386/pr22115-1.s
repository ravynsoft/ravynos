	.text
	.globl	_start
	.type	_start, @function
_start:
	movl	__ehdr_start@GOT(%eax), %eax
	.size	_start, .-_start
	.weak	__ehdr_start
