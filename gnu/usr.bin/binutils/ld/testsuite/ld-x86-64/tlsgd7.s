	.text
	.globl _start
_start:
1:	movabsq	$_GLOBAL_OFFSET_TABLE_-1b, %r11
	pushq	%rbx
	pushq	%rbx
	leaq	1b(%rip), %rbx
	addq	%r11, %rbx

	/* GD, -mcmodel=large  */
	leaq	foo@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%rbx, %rax
	call	*%rax

	popq	%rbx
	popq	%rbx
	ret
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
