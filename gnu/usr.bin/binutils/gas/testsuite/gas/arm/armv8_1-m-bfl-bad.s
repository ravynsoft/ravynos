	.syntax unified
	.text
	.thumb
foo:
	# OP0 : Unsigned, 5-bit, even
	bfl  0, 36
	bfl -2, 36
	bfl  3, 36
	bfl 32, 36
	# OP1 : signed, 19-bit, even
	bf  2, -5
	bf  2,  5
	bf  2,  262144
	bf  2, -262146
