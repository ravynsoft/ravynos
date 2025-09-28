	.data
	.globl	foo
	.type	foo, @object
foo:
	.dc.l	0
	.size	foo, . - foo
	.type	bar, @object
bar:
	.dc.l	0
	.size	bar, . - bar
