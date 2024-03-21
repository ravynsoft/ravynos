	.text
	.globl	foo
	.type	foo, %function
foo:
	.size	foo, . - foo

	.data
	.dc.a	bar
