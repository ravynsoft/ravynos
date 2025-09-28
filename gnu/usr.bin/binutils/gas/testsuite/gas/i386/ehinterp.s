	.file	"hello.c"
	.text
	.section	.rodata
.LC0:
	.string	"Hello"
	.text
	.globl	dummy
	.type	dummy, @function
dummy:
	.cfi_startproc
	endbr64
	.cfi_undefined rip
	jmp .
	.cfi_endproc
	.size	dummy, .-dummy

	.globl	main
	.type	main, @function
main:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
	.size	main, .-main
