	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.text
	.globl	check
	.type	check, @function
check:
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$8, %esp
	call	*get_func1@GOT(%ebx)
#ifdef CHECK_PLT
	cmpl	$func1, %eax
#else
	cmpl	func1@GOT(%ebx), %eax
#endif
	jne	.L3
	movl	func1_p@GOT(%ebx), %edx
	cmpl	%eax, (%edx)
	jne	.L3
	call	*func1@GOT(%ebx)
	cmpl	$1, %eax
	jne	.L3
	call	*call_func1@GOT(%ebx)
	cmpl	$1, %eax
	jne	.L3
	call	*get_func2@GOT(%ebx)
	movl	func2_p@GOT(%ebx), %edx
	cmpl	%eax, (%edx)
	jne	.L3
	call	*call_func2@GOT(%ebx)
	cmpl	$2, %eax
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
