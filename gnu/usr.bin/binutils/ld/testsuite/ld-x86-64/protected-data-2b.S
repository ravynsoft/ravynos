	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"'protected_data_1a' in main and shared library doesn't have same address"
	.align 8
.LC1:
	.string	"'protected_data_1a' in main and shared library doesn't have same value"
	.align 8
.LC2:
	.string	"'protected_data_1b' in main and shared library has same address"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC3:
	.string	"PASS"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	xorl	%eax, %eax
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	xorl	%ebx, %ebx
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	call	protected_data_1a_p
	movq	protected_data_1a@GOTPCREL(%rip), %rbp
	cmpq	%rbp, %rax
	je	.L2
	leaq	.LC0(%rip), %rdi
	movb	$1, %bl
	call	puts
.L2:
	movl	$-1, %edi
	movl	$-1, 0(%rbp)
	call	check_protected_data_1a
	testl	%eax, %eax
	jne	.L17
.L3:
	movl	$-3, %edi
	call	set_protected_data_1a
	cmpl	$-3, 0(%rbp)
	je	.L4
	leaq	.LC1(%rip), %rdi
	movl	$1, %ebx
	call	puts
.L4:
	xorl	%eax, %eax
	call	protected_data_1b_p
	leaq	protected_data_1b(%rip), %rdx
	cmpq	%rdx, %rax
	je	.L18
.L5:
	movl	$2, %edi
	movl	$-10, protected_data_1b(%rip)
	call	check_protected_data_1b
	testl	%eax, %eax
	jne	.L19
	movl	$-30, %edi
	call	set_protected_data_1b
	cmpl	$-10, protected_data_1b(%rip)
	je	.L9
.L7:
	leaq	.LC2(%rip), %rdi
	movl	$1, %ebx
	call	puts
.L8:
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	movl	%ebx, %eax
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
.L9:
	.cfi_restore_state
	testl	%ebx, %ebx
	jne	.L11
	leaq	.LC3(%rip), %rdi
	call	puts
	jmp	.L8
.L19:
	leaq	.LC2(%rip), %rdi
	call	puts
	movl	$-30, %edi
	call	set_protected_data_1b
	cmpl	$-10, protected_data_1b(%rip)
	jne	.L7
.L11:
	movl	$1, %ebx
	jmp	.L8
.L17:
	leaq	.LC1(%rip), %rdi
	movl	$1, %ebx
	call	puts
	jmp	.L3
.L18:
	leaq	.LC2(%rip), %rdi
	movl	$1, %ebx
	call	puts
	jmp	.L5
	.cfi_endproc
.LFE11:
	.size	main, .-main
	.globl	protected_data_1b
	.data
	.align 4
	.type	protected_data_1b, @object
	.size	protected_data_1b, 4
protected_data_1b:
	.long	3
	.section	.note.GNU-stack,"",@progbits
