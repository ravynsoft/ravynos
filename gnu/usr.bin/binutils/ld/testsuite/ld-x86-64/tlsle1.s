	.text
	.globl _start
_start:
	movl	$0, %fs:foo@TPOFF
	.globl	foo
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	100
