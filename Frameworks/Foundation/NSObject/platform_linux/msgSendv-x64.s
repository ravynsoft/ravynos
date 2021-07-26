	.file	"msgSendv-template.c"
	.text
	.globl	objc_msgSendv
	.type	objc_msgSendv, @function
objc_msgSendv:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r12
	pushq	%rbx
	subq	$192, %rsp
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movq	%rdx, -56(%rbp)
	movq	%rcx, -64(%rbp)
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	.cfi_offset 3, -32
	.cfi_offset 12, -24
	call	object_getClass@PLT
	movq	-48(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	class_getMethodImplementation@PLT
	movq	%rax, -32(%rbp)
	movq	-56(%rbp), %rax
	movslq	%eax, %rcx
	movq	-32(%rbp), %r12
	movq	-64(%rbp), %rax
	movq	(%rax), %rsi
	movq	%rsp, %rbx
	leaq	15(%rcx), %rax
	movl	$16, %edx
	subq	$1, %rdx
	addq	%rdx, %rax
	movq	$16, -200(%rbp)
	movl	$0, %edx
	divq	-200(%rbp)
	imulq	$16, %rax, %rax
	subq	%rax, %rsp
	movq	%rsp, %rax
	movq	%rcx, %rdx
	movq	%rax, %rdi
	call	memcpy@PLT
	movq	-64(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	-64(%rbp), %rax
	addq	$16, %rax
	movq	(%rax), %rdx
	movq	-64(%rbp), %rax
	addq	$24, %rax
	movq	(%rax), %rcx
	movq	-64(%rbp), %rax
	addq	$32, %rax
	movq	(%rax), %rsi
	movq	-64(%rbp), %rax
	addq	$40, %rax
	movq	(%rax), %rdi
	movq	-64(%rbp), %rax
	addq	$48, %rax
	movdqu	(%rax), %xmm0
	movq	-64(%rbp), %rax
	addq	$64, %rax
	movdqu	(%rax), %xmm1
	movq	-64(%rbp), %rax
	addq	$80, %rax
	movdqu	(%rax), %xmm2
	movq	-64(%rbp), %rax
	addq	$96, %rax
	movdqu	(%rax), %xmm3
	movq	-64(%rbp), %rax
	addq	$112, %rax
	movdqu	(%rax), %xmm4
	movq	-64(%rbp), %rax
	subq	$-128, %rax
	movdqu	(%rax), %xmm5
	movq	-64(%rbp), %rax
	addq	$144, %rax
	movdqu	(%rax), %xmm6
	movq	-64(%rbp), %rax
	addq	$160, %rax
	movdqu	(%rax), %xmm7
	movq	-64(%rbp), %rax
	addq	$176, %rax
	movq	(%rax), %r8
	movq	-64(%rbp), %rax
	addq	$184, %rax
	movq	(%rax), %r9
	movl	$7, %eax
	call	*%r12
	fstp	%st(1)
	movq	%rax, -192(%rbp)
	fstpt	-176(%rbp)
	movdqa	%xmm0, -160(%rbp)
	movq	%rbx, %rsp
	leaq	-192(%rbp), %rax
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	(%rax), %rax
	movq	-24(%rbp), %rdx
	addq	$16, %rdx
	fldt	(%rdx)
	movq	-24(%rbp), %rdx
	addq	$32, %rdx
	movdqu	(%rdx), %xmm0
	fstp	%st(0)
	leaq	-16(%rbp), %rsp
	popq	%rbx
	popq	%r12
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	objc_msgSendv, .-objc_msgSendv
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
