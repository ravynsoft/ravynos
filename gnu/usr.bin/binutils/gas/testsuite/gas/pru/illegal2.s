# Source file used to test illegal operands.

foo:
# Out-of-bounds immediate value
	add r2, r2, 256
