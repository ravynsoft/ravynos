# Source file to test assembly of MIPS32r2-derived microMIPS cop2 instructions.

	.set noreorder
	.set noat

	.text
text_label:
	# cp2 instructions.

	# Only register syntax with cp2 for microMIPS (and no sel).
	mfhc2	$17, $15
	mthc2	$17, $15

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
