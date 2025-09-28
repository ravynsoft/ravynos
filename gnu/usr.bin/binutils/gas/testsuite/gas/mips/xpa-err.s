# Verify that switching off either `xpa' causes an assembly error
# with an XPA instruction.

	.text
foo:
	mfhc0	$2, $1
	.set	noxpa
	mthc0	$2, $1
