	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	micromips
	.globl	foo
	.ent	foo
foo:
	nor	$0, $0
	jal	0x4000002
	 nor	$0, $0
	jalx	0x8000004
	 nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
	.end	foo

	.align	4
	.set	nomicromips
	.globl	bar
	.ent	bar
bar:
	nor	$0, $0
	jalx	0x8000004
	 nor	$0, $0
	jal	0x8000004
	 nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
