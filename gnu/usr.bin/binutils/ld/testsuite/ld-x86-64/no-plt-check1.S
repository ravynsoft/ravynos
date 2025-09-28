	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.text
	.p2align 4,,15
	.globl	check
	.type	check, @function
check:
	subq	$8, %rsp
	call	*get_func@GOTPCREL(%rip)
	cmpq	func@GOTPCREL(%rip), %rax
	jne	.L3
#ifdef __LP64__
	cmpq	%rax, func_p(%rip)
#else
	rex
	cmpl	%eax, func_p(%rip)
#endif
	jne	.L3
	call	*func@GOTPCREL(%rip)
	cmpl	$305419896, %eax
	jne	.L3
	call	*call_func@GOTPCREL(%rip)
	cmpl	$305419896, %eax
	jne	.L3
	leaq	.LC0(%rip), %rdi
	addq	$8, %rsp
	jmp	*puts@GOTPCREL(%rip)
.L3:
	call	*abort@GOTPCREL(%rip)
	.size	check, .-check
	.section	.note.GNU-stack,"",@progbits
