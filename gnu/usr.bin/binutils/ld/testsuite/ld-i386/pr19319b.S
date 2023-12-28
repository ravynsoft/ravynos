	.text
	.globl	_start
	.type	_start, @function
_start:
	movl	__start_my_section@GOT(%ecx), %eax
	ret
	.size	_start, .-_start
	.section	my_section,"aw",@progbits
	.align 8
	.long	4660
	.long	22136
