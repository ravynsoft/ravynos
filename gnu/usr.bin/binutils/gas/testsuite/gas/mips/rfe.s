	.text
	.set	noreorder
foo:
	rfe

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
