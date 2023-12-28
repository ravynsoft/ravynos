	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.text
	.globl	check
	.type	check, @function
check:
	subl	$12, %esp
	call	*get_func1@GOT
#ifdef CHECK_PLT
	movl	$func1, %edx
#else
	movl	func1@GOT, %edx
#endif
	cmpl	%edx, %eax
	jne	.L3
	cmpl	%edx, func1_p
	jne	.L3
	call	*func1@GOT
	cmpl	$1, %eax
	jne	.L3
	call	*call_func1@GOT
	cmpl	$1, %eax
	jne	.L3
	call	*get_func2@GOT
	cmpl	%eax, func2_p
	jne	.L3
	call	*call_func2@GOT
	cmpl	$2, %eax
	jne	.L3
	subl	$12, %esp
	pushl	$.LC0
	call	*puts@GOT
	addl	$28, %esp
	ret
.L3:
	call	*abort@GOT
	.size	check, .-check
	.section	.note.GNU-stack,"",@progbits
