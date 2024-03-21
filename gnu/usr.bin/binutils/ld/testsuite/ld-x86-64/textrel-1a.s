	.text
	.globl	foo
	.data
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	4
	.section	.note.GNU-stack,"",@progbits
