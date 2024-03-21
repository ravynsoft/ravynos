	.globl	_start
	.type	_start, @function
_start:
	movq	__hidden_sym@GOTPCREL(%rip), %rax
	.size	_start, .-_start
