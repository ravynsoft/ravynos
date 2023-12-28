	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	nomips16
	.ent	foo
foo:
	nor	$0, $0
	jalr	$0, $ra
	 nor	$0, $0
	.end	foo

	.align	4
	.set	mips16
	.ent	bar
bar:
	not	$16, $16
	b	foo
	not	$16, $16
	beqz	$2, foo
	not	$16, $16
	bteqz	foo
	not	$16, $16
	jr	$ra
	 not	$16, $16
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
