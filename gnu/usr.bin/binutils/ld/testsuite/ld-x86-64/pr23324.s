	.text
	.globl	_start
	.type	_start,@function
_start:
	movq	_start@GOTPCREL(%rip), %rsi
	ret
