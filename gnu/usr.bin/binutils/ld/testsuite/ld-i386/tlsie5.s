	.text
	.globl _start
_start:
	leal	foo@indntpoff, %ecx
	movl	(%ecx), %ecx
	.globl foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
