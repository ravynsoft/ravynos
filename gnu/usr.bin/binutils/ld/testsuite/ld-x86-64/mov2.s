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
	movq	foo@GOTPCREL+1(%rip), %rax
	movq	__start_my_section@GOTPCREL+1(%rip), %rax
	movq	__stop_my_section@GOTPCREL+1(%rip), %rax
	.size	_start, .-_start
