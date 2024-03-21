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
	movl	foo@GOTOFF(%ecx), %eax
	.size	_start, .-_start
