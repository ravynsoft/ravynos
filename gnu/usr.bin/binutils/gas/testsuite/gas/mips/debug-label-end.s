	.text

	.globl	foo
	.globl	bar
	.align	4, 0
	.ent	foo
foo:
	nop
	.aent	bar
bar:
	.insn
	.end	foo
	.align	4, 0
	.space	16

	.globl	baz
	.ent	baz
baz:
	nop
	.end	baz
	.align	4, 0
	.space	16
