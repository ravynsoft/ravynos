	.text
	.weak __start_my_section
	.weak __stop_my_section
	.globl	_start
	.type	_start, @function
_start:
	movq	_DYNAMIC@GOTPCREL(%rip), %rax
	movq	__start_my_section@GOTPCREL(%rip), %rax
	movq	__stop_my_section@GOTPCREL(%rip), %rax
	.size	_start, .-_start
