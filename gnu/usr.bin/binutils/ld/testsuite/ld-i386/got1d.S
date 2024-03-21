	.globl	bar
	.type	bar, @function
bar:
	pushl	%ebx
	call	__x86.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	subl	$24, %esp
	leal	__FUNCTION__.1862@GOTOFF(%ecx), %eax
	movl	%eax, (%esp)
	call	*puts@GOT(%ecx)
	addl	$24, %esp
	popl	%ebx
	ret
	.size	bar, .-bar

	.globl	plt
	.type	plt, @function
plt:
	pushl	%esi
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$20, %esp
	leal	__FUNCTION__.1866@GOTOFF(%ebx), %esi
	movl	%esi, (%esp)
	call	*puts@GOT(%ebx)
	addl	$20, %esp
	popl	%ebx
	popl	%esi
	ret
	.section	.rodata
	.type	__FUNCTION__.1866, @object
	.size	__FUNCTION__.1866, 4
__FUNCTION__.1866:
	.string	"plt"
	.type	__FUNCTION__.1862, @object
	.size	__FUNCTION__.1862, 4
__FUNCTION__.1862:
	.string	"bar"
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
