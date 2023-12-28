	.text
foo:
	xor	$16, $16
	bc0f	0f
0:
	xor	$16, $16
	bc0t	0f
0:
	.insn

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
