	.syntax unified
	.text
	.thumb
foo:
	# OP0 : Unsigned, 5-bit, even
	bf  0, 36
	bf -2, 36
	bf  3, 36
	bf 32, 36
	# OP1 : signed, 17-bit, even
	bf  2, -5
	bf  2,  5
	bf  2,  65536
	bf  2, -65538
