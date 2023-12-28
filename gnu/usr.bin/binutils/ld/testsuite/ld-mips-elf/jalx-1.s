	.set	noreorder
	.set	micromips
	.ent	test
	.globl	test
test:
	jalx	test1
	nop

	.set	nomicromips
test1:
	addu	$3, $4, $5
	.end	test

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
