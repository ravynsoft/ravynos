	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	micromips
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
	jalr	$0, $ra
	 nor	$0, $0
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
