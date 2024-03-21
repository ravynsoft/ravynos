	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	micromips
	.globl	foo
	.ent	foo
foo:
	nor	$0, $0
	jal	foo - 0x3fffffe
	 nor	$0, $0
	jalx	bar - 0x7fffffc
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
	jalx	foo - 0x7fffffc
	 nor	$0, $0
	jal	bar - 0x7fffffc
	 nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
