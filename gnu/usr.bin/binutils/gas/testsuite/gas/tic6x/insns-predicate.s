# Test predicates.
.text
.nocmp
.globl f
f:
	[a0] abs .L1 a2,a3
||	[A0] abs .L2 b4,b5
	[!a0] abs .L1 a6,a7
	[!A0] abs .L2 b8,b9
	[b0] abs .L1 a10,a11
	[B0] abs .L2 b12,b13
	[!b0] abs .L1 a14,a15
	[!B0] abs .L2 b16,b17
	[a1] abs .L1 a18,a19
	[!A1] abs .L2 b20,b21
	[A2] abs .L1 a22,a23
	[!a2] abs .L2 b24,b25
	[b1] abs .L1 a26,a27
	[!B1] abs .L2 b28,b29
	[B2] abs .L1 a30,a31
	[!b2] abs .L2 b3,b7
