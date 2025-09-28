	.text
	.globl	foo
	.type	foo, @function
foo:
	movl	__start_my_section@GOT(%ecx), %eax
	ret
	.size	foo, .-foo
	.section	my_section,"aw",@progbits
	.align 8
	.long	4660
	.long	22136
