	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	xorl	%eax, %eax
	cmpl	$2, a_glob(%rip)
	setne	%al
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
