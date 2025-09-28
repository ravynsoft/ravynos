	.text
_start:
	movl	%edi, %eax
	.balign 8, 0x90
	movl	$0, %edx
	.balign 8, 0x90
	addl	%eax, %edx
