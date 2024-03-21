# Source file to test branch relaxation with the BC1ANY2F, BC1ANY2T,
# BC1ANY4F and BC1ANY4T instructions.

	.text
foo:
	b	bar
	bc1any2f $cc0, bar
	bc1any2t $cc0, bar
	bc1any4f $cc0, bar
	bc1any4t $cc0, bar
	bal	bar

	.space	0x20000
bar:
