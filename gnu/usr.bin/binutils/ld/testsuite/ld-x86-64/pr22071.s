	.text
	.p2align 4,,15
	.globl	get_ld
	.type	get_ld, @function
get_ld:
	subq	$8, %rsp
	leaq	_TLS_MODULE_BASE_@TLSDESC(%rip), %rax
	call	*_TLS_MODULE_BASE_@TLSCALL(%rax)
	addq	$8, %rsp
	addq	$ld@dtpoff, %rax
	addq	%fs:0, %rax
	ret
	.size	get_ld, .-get_ld
	.p2align 4,,15
	.globl	set_ld
	.type	set_ld, @function
set_ld:
	subq	$8, %rsp
	leaq	_TLS_MODULE_BASE_@TLSDESC(%rip), %rax
	call	*_TLS_MODULE_BASE_@TLSCALL(%rax)
	movl	%edi, %fs:ld@dtpoff(%rax)
	addq	$8, %rsp
	ret
	.size	set_ld, .-set_ld
	.p2align 4,,15
	.globl	test_ld
	.type	test_ld, @function
test_ld:
	subq	$8, %rsp
	leaq	_TLS_MODULE_BASE_@TLSDESC(%rip), %rax
	call	*_TLS_MODULE_BASE_@TLSCALL(%rax)
	cmpl	%edi, %fs:ld@dtpoff(%rax)
	sete	%al
	addq	$8, %rsp
	movzbl	%al, %eax
	ret
	.size	test_ld, .-test_ld
	.p2align 4,,15
	.globl	get_gd
	.type	get_gd, @function
get_gd:
	subq	$8, %rsp
	leaq	gd@TLSDESC(%rip), %rax
	call	*gd@TLSCALL(%rax)
	addq	$8, %rsp
	addq	%fs:0, %rax
	ret
	.size	get_gd, .-get_gd
	.p2align 4,,15
	.globl	set_gd
	.type	set_gd, @function
set_gd:
	subq	$8, %rsp
	leaq	gd@TLSDESC(%rip), %rax
	call	*gd@TLSCALL(%rax)
	movl	%edi, %fs:(%rax)
	addq	$8, %rsp
	ret
	.size	set_gd, .-set_gd
	.p2align 4,,15
	.globl	test_gd
	.type	test_gd, @function
test_gd:
	subq	$8, %rsp
	leaq	gd@TLSDESC(%rip), %rax
	call	*gd@TLSCALL(%rax)
	cmpl	%edi, %fs:(%rax)
	sete	%al
	addq	$8, %rsp
	movzbl	%al, %eax
	ret
	.size	test_gd, .-test_gd
	.section	.tbss,"awT",@nobits
	.align 4
	.type	ld, @object
	.size	ld, 4
ld:
	.zero	4
