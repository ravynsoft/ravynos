	.file	"tmpdir/aix-lineno-1.txt"
	.csect	.foo[PR]
	.function .foo,.foo
.foo:
	.bf	1
	nop
	.line	2
	nop
	.line	3
	nop
	.line	4
	nop
	.line	5
	nop
	.ef	1

	.globl	.main
	.csect	.main[PR]
	.function .main,.main
.main:
	.bf	7
	bl	.foo
	.line	2
	nop
	.ef	7
