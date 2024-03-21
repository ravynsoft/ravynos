	.text
	.globl	_start
	.type	_start, @function
_start:
	movq	__ehdr_start(%rip), %rax
