# Source file used to test -mips4 branch-likely instructions.

	.text
text_label:
	bc1fl	$fcc1,text_label
	bc1tl	$fcc2,text_label

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
