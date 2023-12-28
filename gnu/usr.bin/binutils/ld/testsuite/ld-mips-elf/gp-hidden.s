	.data
	.globl	blah
	.type	blah, @object
blah:
	.dc.a	foo
	.dc.a	_gp
	.size	blah, . - blah
