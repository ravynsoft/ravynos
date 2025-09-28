	.text
	.globl	_start
	.type	_start,@function
_start:
	movl	_start@GOTPCREL+4(%rip), %eax
