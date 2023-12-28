	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.text
	.p2align 4,,15
	.globl	check
	.type	check, @function
check:
	subl	$12, %esp
	call	*get_func1@GOT
	cmpl	$func1, %eax
	jne	.L3
	call	*func1@GOT
	cmpl	$1, %eax
	jne	.L3
	call	*call_func1@GOT
	cmpl	$1, %eax
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
