	.globl  _start
	.type	_start, @function
_start:
	movl	$foo, %eax
	.size	_start, .-_start
	.data
foo:
	.quad	0
