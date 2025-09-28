# Source file used to test branches to undefined symbols and a defined
# local symbol in another section.

	.globl g6 .text

	.globl x1 .text

	.text
g6:

	b	x1
	b	x2
	b	.Ldata

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8

	.data
.Ldata:
	.insn
