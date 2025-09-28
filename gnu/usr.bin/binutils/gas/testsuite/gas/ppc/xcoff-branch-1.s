	.globl	.foo
	.globl	foo1
	.globl	foo2
	.globl	.bar
	.globl	.frob

	.csect	.foo[PR]
.foo:
	bl	foo2 + 0x4
	bl	foo1 + 0xc
	bl	foo1
	bl	foo2
	bl	.bar
foo1:
	bl	.foo
	bl	.frob
	bl	.foo + 0x10
	bl	.bar + 0x8
foo2:
	bl	.frob + 0x10
	blr

	.csect	.bar[PR]
.bar:	bl	foo1
	bl	foo2
	bl	foo1 + 0x8
	bl	foo2 + 0x4
	bl	.foo
	bl	.bar
	bl	.frob
	bl	.foo + 0x1c
	bl	.bar + 0xc
	bl	.frob + 0x4

	.csect	.frob[PR]
.frob:	bl	.foo
	bl	.bar
	bl	.frob
	bl	foo1
	bl	foo2
