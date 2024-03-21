	.globl	_start
	.type	_start, @function
_start:
	movq	_text@GOTPCREL(%rip), %rax
	.size	_start, .-_start
