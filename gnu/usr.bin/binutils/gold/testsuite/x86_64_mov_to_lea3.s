	.text
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl	_start
	.type	_start, @function
_start:
	movq	foo@GOTPCREL(%rip), %rax
	.size	_start, .-_start
