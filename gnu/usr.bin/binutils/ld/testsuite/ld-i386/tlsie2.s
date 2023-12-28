	.text
	.globl _start
_start:
	leal	foo@gotntpoff(%ebx), %eax
	movl	(%eax), %eax
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
