	.globl  _start
	.type	_start, @function
_start:
	movq	$foo, %rax
	.size	_start, .-_start
	.data
foo:
	.quad	0
