# Source file used to test the rdprs instruction

.set noat

foo:
	rdprs zero, zero, 0
	rdprs zero, zero, 1
	rdprs zero, zero, 2
	rdprs zero, zero, 4
	rdprs zero, zero, 8
	rdprs zero, zero, 16
	rdprs zero, zero, 32
	rdprs zero, zero, 64
	rdprs zero, zero, 128
	rdprs zero, zero, 256
	rdprs zero, zero, 512
	rdprs zero, zero, 1024
	rdprs zero, zero, -2048
	rdprs at, zero, 0
	rdprs r2, zero, 0
	rdprs r4, zero, 0
	rdprs r8, zero, 0
	rdprs r16, zero, 0
	rdprs zero, at, 0
	rdprs zero, r2, 0
	rdprs zero, r4, 0
	rdprs zero, r8, 0
	rdprs zero, r16, 0
