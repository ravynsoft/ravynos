	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.section	.text,"ax",@progbits,unique,1
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
	leal	4(%esp), %ecx
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	pushl	%ecx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$12, %esp
	call	get_copy@PLT
	movl	copy@GOT(%ebx), %esi
	cmpl	(%esi), %eax
	je	.L7
.L3:
	leal	-12(%ebp), %esp
	xorl	%eax, %eax
	popl	%ecx
	popl	%ebx
	popl	%esi
	popl	%ebp
	leal	-4(%ecx), %esp
	ret
.L7:
	call	get_copy_p@PLT
	cmpl	%esi, %eax
	jne	.L3
	leal	.LC0@GOTOFF(%ebx), %eax
	subl	$12, %esp
	pushl	%eax
	call	puts@PLT
	addl	$16, %esp
	jmp	.L3
	.size	main, .-main
	.section .text,"axG",@progbits,__x86.get_pc_thunk.bx,comdat,unique,2
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
	.section	.note.GNU-stack,"",@progbits
