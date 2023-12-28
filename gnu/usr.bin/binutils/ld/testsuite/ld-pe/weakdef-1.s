	.weak	_wsym
	.section	.data$wsym,"w"
	.align 4
_wsym:
	.long	1

	.section	.text$start,"x"
	.globl	_start
	.def	_start;	.scl	2;	.type	32;	.endef
_start:
	pushl	%ebp
	movl	%esp, %ebp
	movl	_wsym, %eax
	testl	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	nop
	popl	%ebp
	ret
