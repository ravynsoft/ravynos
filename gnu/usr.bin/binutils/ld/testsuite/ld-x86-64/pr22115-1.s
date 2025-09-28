	.text
	.globl	_start
	.type	_start, @function
_start:
	movq	__ehdr_start@GOTPCREL(%rip), %rax
	.size	_start, .-_start
	.weak	__ehdr_start
