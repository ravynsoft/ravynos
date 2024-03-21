	.text
	.globl	_start
	.type	_start, @function
_start:
	movl	foo_p(%rip), %eax
	ret
	.size	_start, .-_start
	.globl	foo_p
	.data
	.align 4
	.type	foo_p, @object
	.size	foo_p, 4
foo_p:
	.long	foo
	.section	.note.GNU-stack
