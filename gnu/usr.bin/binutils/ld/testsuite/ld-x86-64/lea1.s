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
	movl	foo@GOTPCREL(%rip), %eax
	movl	bar@GOTPCREL(%rip), %r11d
	movq	foo@GOTPCREL(%rip), %rax
	movq	bar@GOTPCREL(%rip), %r11
	movq	__start_my_section@GOTPCREL(%rip), %rax
	movq	__stop_my_section@GOTPCREL(%rip), %r11
	.size	_start, .-_start
	.comm	pad,4,4
	.comm	bar,4,4
