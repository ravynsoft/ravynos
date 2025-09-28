	.section .rodata.foo,"aMS",@progbits,1,unique,1
.LC0:
	.string	"PASS"
	.text
	.globl	check
	.type	check, @function
check:
	subq	$8, %rsp
	call	*get_func1@GOTPCREL(%rip)
	cmpl	$func1, %eax
	jne	.L3
	movq	func1_p@GOTPCREL(%rip), %rdx
#ifdef __LP64__
	cmpq	%rax, (%rdx)
#else
	cmpl	%eax, (%rdx)
#endif
	jne	.L3
	call	*func1@GOTPCREL(%rip)
	cmpl	$1, %eax
	jne	.L3
	call	*call_func1@GOTPCREL(%rip)
	cmpl	$1, %eax
	jne	.L3
	leaq	.LC0(%rip), %rdi
	addq	$8, %rsp
	jmp	*puts@GOTPCREL(%rip)
.L3:
	call	*abort@GOTPCREL(%rip)
	.size	check, .-check
	.globl	get_func1
	.type	get_func1, @function
get_func1:
	movq	func1@GOTPCREL(%rip), %rax
	ret
	.size	get_func1, .-get_func1
	.globl	call_func1
	.type	call_func1, @function
call_func1:
	jmp	*func1@GOTPCREL(%rip)
	.size	call_func1, .-call_func1
	.globl	func1_p
	.section .rodata.foo,"a",@progbits,unique,2
	.align 8
	.size	func1_p, 8
	.type	func1_p, @object
func1_p:
	.dc.a	func1
	.text
implementation1:
	movl	$1, %eax
	ret
	.size	implementation1, .-implementation1
	.globl	func1
	.type	func1, @gnu_indirect_function
func1:
	leaq	implementation1(%rip), %rax
	ret
	.size	func1, .-func1
	.section	.note.GNU-stack,"",@progbits
