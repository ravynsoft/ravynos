	.set	mips16
foo:
	ehb

	dmt
	dmt	$0
	dmt	$2
	emt
	emt	$0
	emt	$2

	dvpe
	dvpe	$0
	dvpe	$2
	evpe
	evpe	$0
	evpe	$2

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
