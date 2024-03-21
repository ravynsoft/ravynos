	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.text
	.p2align 4,,15
	.globl	check
	.type	check, @function
check:
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$8, %esp
	call	*get_func@GOT(%ebx)
	cmpl	func@GOT(%ebx), %eax
	jne	.L3
	movl	func_p@GOT(%ebx), %edx
	cmpl	%eax, (%edx)
	jne	.L3
	call	*func@GOT(%ebx)
	cmpl	$305419896, %eax
	jne	.L3
	call	*call_func@GOT(%ebx)
	cmpl	$305419896, %eax
	jne	.L3
	leal	.LC0@GOTOFF(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	*puts@GOT(%ebx)
	addl	$24, %esp
	popl	%ebx
	ret
.L3:
	call	*abort@GOT(%ebx)
	.size	check, .-check
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
	.section	.note.GNU-stack,"",@progbits
