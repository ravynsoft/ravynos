	.text
selector:
	movl	foo@GOT(%eax), %eax
	mov	$bar, %ebx
	ret
	.type   selector, %gnu_indirect_function
	.globl	bar
bar:
	jmp	selector@PLT
