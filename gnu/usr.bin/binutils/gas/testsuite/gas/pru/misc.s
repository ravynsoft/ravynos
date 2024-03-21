# Source file used to test the miscellaneous instructions.

foo:
	halt
	slp	1
	slp	0
	lmbd r0, r1, 0x1
	lmbd r0.b0, r1, 0x0
	lmbd r0, r1, r2.b2
