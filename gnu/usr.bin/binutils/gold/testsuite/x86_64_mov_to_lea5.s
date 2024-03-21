	.text
	.globl	foo
	.hidden foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl	_start
	.type	_start, @function
_start:
	movl	foo@GOTPCREL+4(%rip), %eax
	.size	_start, .-_start
