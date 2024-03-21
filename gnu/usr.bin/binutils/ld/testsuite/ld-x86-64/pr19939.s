	.text
selector:
	movq	foo@GOTPCREL(%rip), %rax
	movabs	$bar, %rbx
	ret
	.type   selector, %gnu_indirect_function
	.globl	bar
bar:
	jmp	selector@PLT
