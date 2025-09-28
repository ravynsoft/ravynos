	.text
	.p2align 4,,15
	.globl	get_ld
	.type	get_ld, @function
get_ld:
	subq	$8, %rsp
	leaq	ld@tlsld(%rip), %rdi
	call	__tls_get_addr@PLT
	addq	$8, %rsp
	addq	$ld@dtpoff, %rax
	ret
	.size	get_ld, .-get_ld
	.text
	.p2align 4,,15
	.globl	set_ld
	.type	set_ld, @function
set_ld:
	pushq	%rbx
	movl	%edi, %ebx
	leaq	ld@tlsld(%rip), %rdi
	call	*__tls_get_addr@GOTPCREL(%rip)
	movl	%ebx, ld@dtpoff(%rax)
	popq	%rbx
	ret
	.size	set_ld, .-set_ld
	.text
	.p2align 4,,15
	.globl	test_ld
	.type	test_ld, @function
test_ld:
	pushq	%rbx
	movl	%edi, %ebx
	leaq	ld@tlsld(%rip), %rdi
	call	*__tls_get_addr@GOTPCREL(%rip)
	cmpl	%ebx, ld@dtpoff(%rax)
	popq	%rbx
	sete	%al
	movzbl	%al, %eax
	ret
	.size	test_ld, .-test_ld
	.section	.tbss,"awT",@nobits
	.align 4
	.type	ld, @object
	.size	ld, 4
ld:
	.zero	4
	.section	.note.GNU-stack,"",@progbits
