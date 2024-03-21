	.text
	.globl _start
_start:
1:	movabsq	$_GLOBAL_OFFSET_TABLE_-1b, %r11
	pushq	%r15
	pushq	%r15
	leaq	1b(%rip), %r15
	addq	%r11, %r15

	/* GD, -mcmodel=large  */
	leaq	foo@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax

	popq	%r15
	popq	%r15
	ret
