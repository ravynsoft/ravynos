	.text
	.globl	_start
	.type	_start, @function
_start:
	movq	__start_my_section@GOTPCREL(%rip), %rax
	ret
	.size	_start, .-_start
	.section	my_section,"aw",@progbits
	.align 8
	.long	4660
	.long	22136
	.section	.note.GNU-stack
