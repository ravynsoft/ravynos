	.text

	.globl	foo
	.type	foo, @function
foo:
	.insn
	.dc.l	0
	.size	foo, . - foo

	.set	micromips

	.globl	bar
	.type	bar, @function
bar:
	.insn
	.dc.l	0
	.size	bar, . - bar

	.globl	baz
	.type	baz, @object
baz:
	.dc.l	0
	.size	baz, . - baz
