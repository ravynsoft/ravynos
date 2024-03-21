	.globl _start
	.type	_start, @function
_start:
	movabsq	$foo@GOTPLT, %rax
	.size	_start, .-_start
