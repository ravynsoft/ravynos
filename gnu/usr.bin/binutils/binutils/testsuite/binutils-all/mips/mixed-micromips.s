	.section .text.foo, "ax", @progbits
	.set	nomicromips
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
	.set	micromips
	.globl	bar
	.ent	bar
bar:
	addiu	$sp, $sp, -32
	sw	$ra, 28($sp)
	jals	baz
	lw	$ra, 28($sp)
	jraddiusp 32
	.end	bar

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
