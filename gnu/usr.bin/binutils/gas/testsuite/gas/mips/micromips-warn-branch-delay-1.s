# Source code used to test correct macro expansion in microMIPS fixed-size
# branch delay slots.

	.text
	.set	dspr2
	.set	noreorder
	.set	noat
test:
	bltzals	$0, .
	 nop
	nop

	bltzals	$0, .
	 bgt	$2, 0x7fffffff, .
	 nop

	bltzals	$0, .
	 jals	$2
	 nop

	bltzals	$0, .
	 balign	$2, $2, 0
	nop

	bltzal	$0, .
	 nop
	nop

	bltzal	$0, .
	 bgt	$2, 0x7fffffff, .
	 nop

	bltzal	$0, .
	 jals	$2
	 nop

	bltzal	$0, .
	 balign	$2, $2, 0
	nop

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
