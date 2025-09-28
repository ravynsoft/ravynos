# Test the work around of the NOP issue of loongson2F
	.text
	.set noreorder

	.align 5	# Test _implicit_ nops
loongson2f_nop_insn:
	nop		# Test _explicit_ nops

# align section end to 16-byte boundary for easier testing on multiple targets
	.p2align 4
