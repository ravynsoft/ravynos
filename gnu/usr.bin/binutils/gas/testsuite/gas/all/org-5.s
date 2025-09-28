	.data
	.org	0x10
	.globl	foo
foo:
	.byte	0, 1, 2, 3
	.org	0x15
	.globl	bar
bar:
	.byte	0, 1, 2, 3
