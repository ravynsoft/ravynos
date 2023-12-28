# Source file to test wide immediates with MIPS32 WAIT and SDBBP instructions

	.set noreorder
	.set noat

	.text
text_label:

	# 20 bits accepted for MIPS32
	wait	0x56789
	sdbbp	0x56789

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
