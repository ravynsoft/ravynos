# source file to test assembly of MIPS DSP ASE Rev3 for MIPS32 instructions

	.set noreorder
	.set noat

	.text
text_label:
	bposge32c text_label

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align 2
	.space 8
