	.text
	.globl	foo
	.type	foo, @function
foo:
	movq	__start_my_section@GOTPCREL(%rip), %rax
	ret
	.size	foo, .-foo
	.section	my_section,"aw",@progbits
	.align 8
	.long	4660
	.long	22136
	.section	.note.GNU-stack
