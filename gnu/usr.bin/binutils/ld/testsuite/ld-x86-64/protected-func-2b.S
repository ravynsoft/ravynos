	.text
	.p2align 4
	.globl	protected_func_1b
	.type	protected_func_1b, @function
protected_func_1b:
.LFB11:
	.cfi_startproc
	movl	$3, %eax
	ret
	.cfi_endproc
.LFE11:
	.size	protected_func_1b, .-protected_func_1b
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"'protected_func_1a' in main and shared library doesn't have same address"
	.align 8
.LC1:
	.string	"'protected_func_1a' doesn't return the correct value"
	.align 8
.LC2:
	.string	"'protected_func_1b' in main and shared library has same address"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC3:
	.string	"PASS"
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB12:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	xorl	%r12d, %r12d
	call	protected_func_1a_p
	cmpq	protected_func_1a@GOTPCREL(%rip), %rax
	je	.L4
	leaq	.LC0(%rip), %rdi
	movl	$1, %r12d
	call	puts
.L4:
	call	protected_func_1a
	cmpl	$1, %eax
	jne	.L13
	call	protected_func_1b_p
	leaq	protected_func_1b(%rip), %rdx
	cmpq	%rax, %rdx
	je	.L6
	testl	%r12d, %r12d
	jne	.L12
	leaq	.LC3(%rip), %rdi
	call	puts
	movl	%r12d, %eax
	popq	%r12
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L13:
	.cfi_restore_state
	leaq	.LC1(%rip), %rdi
	call	puts
	call	protected_func_1b_p
	leaq	protected_func_1b(%rip), %rdx
	cmpq	%rax, %rdx
	je	.L6
.L12:
	movl	$1, %r12d
	movl	%r12d, %eax
	popq	%r12
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L6:
	.cfi_restore_state
	leaq	.LC2(%rip), %rdi
	call	puts
	jmp	.L12
	.cfi_endproc
.LFE12:
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
