	.text
	.p2align 4,,15
	.globl	get_gd
	.type	get_gd, @function
get_gd:
	subq	$8, %rsp
#ifdef __LP64__
	.byte	0x66
#endif
	leaq	gd@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	addq	$8, %rsp
	ret
	.size	get_gd, .-get_gd
	.text
	.p2align 4,,15
	.globl	set_gd
	.type	set_gd, @function
set_gd:
	pushq	%rbx
	movl	%edi, %ebx
#ifdef __LP64__
	.byte	0x66
#endif
	leaq	gd@tlsgd(%rip), %rdi
	.value	0x6666
	rex64
	call	__tls_get_addr@PLT
	movl	%ebx, (%rax)
	popq	%rbx
	ret
	.size	set_gd, .-set_gd
	.text
	.p2align 4,,15
	.globl	test_gd
	.type	test_gd, @function
test_gd:
	pushq	%rbx
	movl	%edi, %ebx
#ifdef __LP64__
	.byte	0x66
#endif
	leaq	gd@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	cmpl	%ebx, (%rax)
	popq	%rbx
	sete	%al
	movzbl	%al, %eax
	ret
	.size	test_gd, .-test_gd
	.section	.note.GNU-stack,"",@progbits
