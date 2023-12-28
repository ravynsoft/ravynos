	.text
	.globl _start
_start:
	movq	foo@GOTTPOFF(%rip), %rax
	addq	foo@GOTTPOFF(%rip), %rax
	movq	foo@GOTTPOFF(%rip), %r12
	addq	foo@GOTTPOFF(%rip), %r12
	.globl	foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
