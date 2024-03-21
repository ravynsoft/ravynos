	.align	2
	.set	mips16
	.globl	foo
	.ent	foo
foo:
	.stabs	"foo:F(0,49)",36,0,0,foo
	jr	$31
	.end	foo
