# Source file used to test the wrprs instruction

.set noat

foo:
	wrprs zero, zero
	wrprs at, zero
	wrprs r2, zero
	wrprs r4, zero
	wrprs r8, zero
	wrprs r16, zero
	wrprs zero, at
	wrprs zero, r2
	wrprs zero, r4
	wrprs zero, r8
	wrprs zero, r16
