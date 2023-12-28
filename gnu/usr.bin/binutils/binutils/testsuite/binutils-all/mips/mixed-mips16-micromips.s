	.align	4, 0
	.set	mips16
	.globl	foo
	.ent	foo
foo:
	lw	$v0, .foo.data
	lw	$v1, 0($v0)
	move	$t9, $v1
	jr	$v1

	.type	.foo.data, @object
.foo.data:
	.word	0x40404040
	.size	.foo.data, . - .foo.data
	.end	foo
	.set	nomips16

	.align	4, 0
	.set	micromips
	.globl	bar
	.ent	bar
bar:
	lui	$v1, 0
	lw	$t9, 0($v1)
	jrc	$t9
	nop
	.end	bar
	.set	nomicromips

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
