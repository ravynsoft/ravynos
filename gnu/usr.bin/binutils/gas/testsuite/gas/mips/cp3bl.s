	.text
	.set	noreorder
foo:
	bc3fl	0f
	 xor	$16, $16
0:
	bc3tl	0f
	 xor	$16, $16
0:
	.insn

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
