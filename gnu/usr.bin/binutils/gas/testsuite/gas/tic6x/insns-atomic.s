# Test atomic instructions.
.text
.nocmp
.globl f
f:
	[a0] cmtl .D2T2 *b0,b1
	[!b1] ll .D2T2 *b2,b3
	[a2] sl .D2T2 b30,*b29
