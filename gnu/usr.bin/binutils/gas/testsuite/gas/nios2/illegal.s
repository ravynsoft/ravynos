# Source file used to test illegal operands.

foo:
# Illegal registers
	add r3,r4,r56
	add r4,r0,r2
# Illegal syntax
	ldw r4,0x1000
	ldw r4,0x1000(r5
	ldw r4,0x1000r5)
	ldw r4,0x1000,r5
	ldw r4,(0x1000)r5
# Illegal opcodes
	fop r3,r4,r5
# Extra operands
	nop Crapola
	add r2, r2, r2, r4
