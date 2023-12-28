	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.text
	.globl	check
	.type	check, @function
check:
	subq	$8, %rsp
	call	*get_func1@GOTPCREL(%rip)
#ifdef CHECK_PLT
	cmpl	$func1, %eax
#else
	cmpq	func1@GOTPCREL(%rip), %rax
#endif
	jne	.L3
	movq	func1_p@GOTPCREL(%rip), %rdx
#ifdef __LP64__
	cmpq	%rax, (%rdx)
#else
	cmpl	%eax, (%rdx)
#endif
	jne	.L3
	call	*func1@GOTPCREL(%rip)
	cmpl	$1, %eax
	jne	.L3
	call	*call_func1@GOTPCREL(%rip)
	cmpl	$1, %eax
	jne	.L3
	call	*get_func2@GOTPCREL(%rip)
	movq	func2_p@GOTPCREL(%rip), %rdx
#ifdef __LP64__
	cmpq	%rax, (%rdx)
#else
	cmpl	%eax, (%rdx)
#endif
	jne	.L3
	call	*call_func2@GOTPCREL(%rip)
	cmpl	$2, %eax
	jne	.L3
	leaq	.LC0(%rip), %rdi
	addq	$8, %rsp
	jmp	*puts@GOTPCREL(%rip)
.L3:
	call	*abort@GOTPCREL(%rip)
	.size	check, .-check
	.section	.note.GNU-stack,"",@progbits
