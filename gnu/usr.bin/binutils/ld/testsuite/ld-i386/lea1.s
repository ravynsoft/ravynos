	.section	my_section,"aw",@progbits
	.long	0x12345678
	.text
	.globl	foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl	_start
	.type	_start, @function
_start:
	movl	foo@GOT(%ecx), %eax
	movl	bar@GOT(%ecx), %eax
	movl	__start_my_section@GOT(%ecx), %eax
	movl	__stop_my_section@GOT(%ecx), %eax
	.size	_start, .-_start
	.comm	pad,4,4
	.comm	bar,4,4
