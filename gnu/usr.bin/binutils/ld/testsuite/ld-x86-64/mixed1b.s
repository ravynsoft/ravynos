.globl foo
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"Hello"
	.data
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	.LC0
