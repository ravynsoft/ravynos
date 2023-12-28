	.text
	.globl _start
_start:
	leaq	foo@GOTTPOFF(%rip), %rax
	movq	(%rax), %rax
	.globl	foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
