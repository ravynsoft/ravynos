	.data
	.globl	foo
	.type	foo, @object
foo:
	.dc.a	bar
	.dc.a	_gp
	.size	foo, . - foo
