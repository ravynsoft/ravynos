	.text
	.weak __start_my_section
	.weak __stop_my_section
	.globl	_start
	.type	_start, @function
_start:
	movl	_DYNAMIC@GOT(%ecx), %eax
	movl	__start_my_section@GOT(%ecx), %eax
	movl	__stop_my_section@GOT(%ecx), %eax
	.size	_start, .-_start
