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
	movl	foo@GOT+1(%ecx), %eax
	movl	__start_my_section@GOT+1(%ecx), %eax
	movl	__stop_my_section@GOT+1(%ecx), %eax
	.size	_start, .-_start
