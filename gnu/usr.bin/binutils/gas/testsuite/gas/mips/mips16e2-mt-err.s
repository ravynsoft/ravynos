# Verify that switching off either `mips16e2' or `mt' causes an assembly
# error with a MIPS16e2 MT ASE instruction, which requires both at a time.

	.text
foo:
	evpe
	.set	nomips16e2
	dvpe
	.set	mips16e2
	evpe
	.set	nomt
	dvpe
	.set	mt
	evpe
