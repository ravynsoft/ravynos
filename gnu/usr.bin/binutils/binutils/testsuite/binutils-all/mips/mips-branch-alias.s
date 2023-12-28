	.text
	.set	noat
	.set	noreorder
	.set	mips2
foo:
	beq	$0, $0, . + 4
	bgez	$0, . + 4
	bgezal	$0, . + 4
	bltzal	$0, . + 4
	beqz	$1, . + 4
	bnez	$1, . + 4

	beqzl	$1, . + 4
	bnezl	$1, . + 4

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
