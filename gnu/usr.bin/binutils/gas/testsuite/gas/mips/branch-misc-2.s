# Source file used to test branches to globals in this file.

	.globl g1 .text
	.globl g2 .text
	.globl g3 .text
	.globl g4 .text
	.globl g5 .text
	.globl g6 .text

	.globl x1 .text

	.text
g1:
	.space 20
g2:
	.space 20
g3:
	.space 20

x:
	bal	g1
	bal	g2
	bal	g3
	bal	g4
	bal	g5
	bal	g6

	.space 20
g4:
	.space 20
g5:
	.space 20
g6:

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
