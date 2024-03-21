// Test file for AArch64 bti.

	.text

	bti
	bti c
	bti j
	bti jc

	bti C
	bti J
	bti JC
