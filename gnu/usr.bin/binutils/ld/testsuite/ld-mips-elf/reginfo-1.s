	.section .foo, "aw", @progbits
	.globl	foo
	.type	foo, @object
foo:
	.set	.Li, 0
	.rept	16
	.set	.Li, .Li + 1
	.byte	.Li
	.endr
	.size	foo, . - foo
