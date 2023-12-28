	.text
	.set	noreorder
foo:
	bc0fl	0f
	 xor	$16, $16
0:
	bc0tl	0f
	 xor	$16, $16
0:
	.insn

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
