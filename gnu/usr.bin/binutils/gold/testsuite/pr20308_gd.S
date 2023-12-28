	.text
	.p2align 4,,15
	.globl	get_gd
	.type	get_gd, @function
get_gd:
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$8, %esp
	leal	gd@tlsgd(,%ebx,1), %eax
	call	___tls_get_addr@PLT
	addl	$8, %esp
	popl	%ebx
	ret
	.size	get_gd, .-get_gd
	.p2align 4,,15
	.globl	set_gd
	.type	set_gd, @function
set_gd:
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$8, %esp
	leal	gd@tlsgd(%ebx), %eax
	call	___tls_get_addr@PLT
	nop
	movl	16(%esp), %edx
	movl	%edx, (%eax)
	addl	$8, %esp
	popl	%ebx
	ret
	.size	set_gd, .-set_gd
	.text
	.p2align 4,,15
	.globl	test_gd
	.type	test_gd, @function
test_gd:
	call	__x86.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	subl	$12, %esp
	leal	gd@tlsgd(%ecx), %eax
	call	*___tls_get_addr@GOT(%ecx)
	movl	16(%esp), %ecx
	cmpl	%ecx, (%eax)
	sete	%al
	addl	$12, %esp
	movzbl	%al, %eax
	ret
	.size	test_gd, .-test_gd
	.section	.text.unlikely
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
	.section	.text.__x86.get_pc_thunk.cx,"axG",@progbits,__x86.get_pc_thunk.cx,comdat
	.globl	__x86.get_pc_thunk.cx
	.hidden	__x86.get_pc_thunk.cx
	.type	__x86.get_pc_thunk.cx, @function
__x86.get_pc_thunk.cx:
	movl	(%esp), %ecx
	ret
	.section	.note.GNU-stack,"",@progbits
