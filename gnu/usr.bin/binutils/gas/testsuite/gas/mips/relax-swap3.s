# Source file used to check the lack of branch swapping with a relaxed macro.

	.text
foo:
	la	$2, bar
	jr	$3

	la	$2, bar
	beqz	$3, 0f
0:
	.insn

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
