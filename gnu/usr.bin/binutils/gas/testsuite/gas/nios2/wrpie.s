# Source file used to test the WRPIE instruction

.set noat

foo:
	wrpie zero, zero
	wrpie at, zero
	wrpie r2, zero
	wrpie r4, zero
	wrpie r8, zero
	wrpie r16, zero
	wrpie zero, at
	wrpie zero, r2
	wrpie zero, r4
	wrpie zero, r8
	wrpie zero, r16
