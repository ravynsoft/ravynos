	.text
	.p2align 4,,15
	.globl	get_ld
	.type	get_ld, @function
get_ld:
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$8, %esp
	leal	ld@tlsldm(%ebx), %eax
	call	___tls_get_addr@PLT
	leal	ld@dtpoff(%eax), %eax
	addl	$8, %esp
	popl	%ebx
	ret
	.size	get_ld, .-get_ld
	.p2align 4,,15
	.globl	set_ld
	.type	set_ld, @function
set_ld:
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$8, %esp
	leal	ld@tlsldm(%ebx), %eax
	call	___tls_get_addr@PLT
	movl	16(%esp), %edx
	leal	ld@dtpoff(%eax), %eax
	movl	%edx, (%eax)
	addl	$8, %esp
	popl	%ebx
	ret
	.size	set_ld, .-set_ld
	.p2align 4,,15
	.globl	test_ld
	.type	test_ld, @function
test_ld:
	call	__x86.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	subl	$12, %esp
	leal	ld@tlsldm(%ecx), %eax
	call	*___tls_get_addr@GOT(%ecx)
	movl	16(%esp), %ecx
	leal	ld@dtpoff(%eax), %eax
	cmpl	%ecx, (%eax)
	sete	%al
	addl	$12, %esp
	movzbl	%al, %eax
	ret
	.size	test_ld, .-test_ld
	.section	.tbss,"awT",@nobits
	.align 4
	.type	ld, @object
	.size	ld, 4
ld:
	.zero	4
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
