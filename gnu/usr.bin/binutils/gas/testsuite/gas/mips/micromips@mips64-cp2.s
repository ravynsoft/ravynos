# Source file to test assembly of MIPS64-derived microMIPS cop2 instructions

	.set noreorder
	.set noat

	.globl text_label .text
text_label:

	# Unprivileged coprocessor instructions.
	# These tests use cp2 to avoid other (cp0, fpu, prefetch) opcodes.

	# No sel with cp2 for microMIPS.
	dmfc2	$3, $4
	dmtc2	$6, $7

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
