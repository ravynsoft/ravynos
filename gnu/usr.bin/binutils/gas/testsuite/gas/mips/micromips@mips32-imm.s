# Source file to test wide immediates with MIPS32 WAIT and SDBBP instructions

	.set noreorder
	.set noat

	.text
text_label:

	# 10 bits accepted for microMIPS
	wait	0x3c3
	sdbbp	0x3c3

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
