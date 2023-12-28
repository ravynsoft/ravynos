	.file	"sframe-bar.c"
	.text
	.globl	bar
	.type	bar, @function
bar:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	cmpl	$1000, -4(%rbp)
	jle	.L2
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	foo
	jmp	.L3
.L2:
	movl	-4(%rbp), %eax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	bar, .-bar
	.ident	"GCC: (GNU) 13.0.0 20220519 (experimental)"
	.section	.note.GNU-stack,"",@progbits
