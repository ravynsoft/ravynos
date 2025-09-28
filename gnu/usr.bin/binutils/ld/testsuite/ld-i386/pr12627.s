	.section ".text16","ax"
	.globl	_start
_start:
	movl	$__bss16_start, %edi
	movl	$__bss16_dwords, %ecx
	xorl	%eax, %eax
	rep movsl
	ret

	.section ".bss16","ax"
	.space	1024
