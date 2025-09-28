	.text
	.globl	_start
	.type	_start, @function
_start:
	movl	foo_p(%rip), %eax
	movq	foo@GOTPCREL(%rip), %rax
	ret
	.size	_start, .-_start
	.globl	foo_p
	.data
	.align 4
	.type	foo_p, @object
	.weak foo
	.size	foo_p, 4
foo_p:
	.long	foo
