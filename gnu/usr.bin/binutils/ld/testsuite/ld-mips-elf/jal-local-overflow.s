	.text
	.set	noreorder
	.org	0x2000

	.align	4
	.globl	foo
	.ent	foo
foo:
	jal	abar - 8
	 nor	$0, $0

	.aent	afoo
afoo:
	jal	afoo - 8
	 nor	$0, $0
	.end	foo

	.org	0x4000

	.align	4
	.globl	bar
	.ent	bar
bar:
	jal	afoo - 8
	 nor	$0, $0

	.aent	abar
abar:
	jal	abar - 8
	 nor	$0, $0
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
