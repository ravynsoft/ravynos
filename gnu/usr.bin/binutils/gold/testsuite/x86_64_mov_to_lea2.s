	.text
	.globl	_start
	.type	_start, @function
_start:
	movq	_DYNAMIC@GOTPCREL(%rip), %rax
	.size	_start, .-_start
