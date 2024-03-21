	.text
	.globl	main
	.type	main, @function
main:
	subl	$12, %esp
	call	*foo@GOT
	call	*bar@GOT
	call	*plt@GOT
	movl	foo@GOT, %eax
	call	*%eax
	movl	bar@GOT, %eax
	call	*%eax
	movl	plt@GOT, %eax
	call	*%eax
	call	__x86.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	movl	foo@GOT(%ecx), %ecx
	call	*%ecx
	subl	$12, %esp
	pushl	$0
	pushl	$0	# Push a dummy return address onto stack.
	jmp	*myexit@GOT
	.size	main, .-main
	.section	.text.__x86.get_pc_thunk.cx,"axG",@progbits,__x86.get_pc_thunk.cx,comdat
	.globl	__x86.get_pc_thunk.cx
	.hidden	__x86.get_pc_thunk.cx
	.type	__x86.get_pc_thunk.cx, @function
__x86.get_pc_thunk.cx:
	movl	(%esp), %ecx
	ret
