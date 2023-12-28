	.text
	.set	mips32r3
	.set	noat
	.set	noreorder
	.set	micromips
foo:
	beq	$0, $0, . + 4
	bgez	$0, . + 4
	beqzc	$0, . + 4
	beqz	$1, . + 4
	bnez	$1, . + 4
	bgezal	$0, . + 4
	bgezals	$0, . + 4

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
