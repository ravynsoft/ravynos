	.section .text.foo, "ax", @progbits
	.set	nomips16
	.globl	foo
	.ent	foo
foo:
	addiu	$sp, $sp, -32
	sw	$ra, 28($sp)
	jal	baz
	lw	$ra, 28($sp)
	addiu	$sp, $sp, 32
	jr	$ra
	.end	foo

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8


	.section .text.bar, "ax", @progbits
	.set	mips16
	.globl	bar
	.ent	bar
bar:
	addiu	$sp, -32
	sw	$ra, 28($sp)
	jal	baz
	lw	$a3, 28($sp)
	addiu	$sp, 32
	jr	$a3
	.end	bar

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
