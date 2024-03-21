	.globl	strings
	.section	.rodata
.LC0:
	.string	"test"
	.section	.data.rel.local,"aw",@progbits
	.align 8
	.type	strings, @object
	.size	strings, 8
strings:
	.quad	.LC0
