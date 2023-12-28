	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	nomicromips
	.ent	foo
foo:
	nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
	.end	foo

	.align	4
	.set	micromips
	.ent	bar
bar:
	nor	$0, $0
	beqzc	$2, foo
	nor	$0, $0
	b	foo
	 nor	$0, $0
	beqz	$2, foo
	 nor	$0, $0
	bgezal	$2, foo
	 nor	$0, $0
	bgezals	$2, foo
	 not	$16, $16
	bltzal	$2, foo
	 nor	$0, $0
	bltzals	$2, foo
	 not	$16, $16
	bals	foo
	 not	$16, $16
	jalr	$0, $ra
	 nor	$0, $0
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
