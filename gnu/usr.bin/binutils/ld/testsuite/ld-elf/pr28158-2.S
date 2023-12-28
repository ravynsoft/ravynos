	.text
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
	.cfi_startproc
#ifdef __x86_64__
	movl	foo(%rip), %eax
#else
	movl	foo, %eax
#endif
	ret
	.cfi_endproc
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
