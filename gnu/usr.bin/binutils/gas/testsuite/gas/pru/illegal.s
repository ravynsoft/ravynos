# Source file used to test illegal operands.

foo:
# Illegal registers
	add r56,r4,r5
	add r4,r0,r2
# Illegal opcodes
	fop r3,r4,r5
# Extra operands
	nop Crapola
	add r2, r2, r2, r4
