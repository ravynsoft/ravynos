	.text
	.protected	foo
	.globl foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl _start
	.type	_start, @function
_start:
	movabsq	$foo@GOTOFF, %rax
	.size	_start, .-_start
