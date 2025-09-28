	.data
	.type foo,%object
	.globl foo
foo:
	.dc.a 0
	.size	foo, . - foo
	.type foo,%object
	.globl bar
bar:
	.dc.a 0
	.size	bar, . - bar
