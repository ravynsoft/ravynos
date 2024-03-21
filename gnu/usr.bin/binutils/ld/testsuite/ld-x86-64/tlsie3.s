	.text
	.globl _start
_start:
	leaq	foo@GOTTPOFF(%rip), %r12
	movq	(%r12), %r12
	.globl	foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
