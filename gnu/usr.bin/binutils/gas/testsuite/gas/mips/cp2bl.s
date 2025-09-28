	.text
	.set	noreorder
foo:
	bc2fl	0f
	 xor	$16, $16
0:
	bc2tl	0f
	 xor	$16, $16
0:
	.insn

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
