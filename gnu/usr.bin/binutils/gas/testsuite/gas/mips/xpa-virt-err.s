# Verify that switching off either `xpa' or `virt' causes an assembly
# error with an XPA Virtualization ASE instruction, which requires
# both at a time.

	.text
foo:
	mfhgc0	$2, $1
	.set	noxpa
	mthgc0	$2, $1
	.set	xpa
	mfhgc0	$2, $1
	.set	novirt
	mthgc0	$2, $1
	.set	virt
	mfhgc0	$2, $1
