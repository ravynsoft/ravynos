# Source file used to test -mips4 fp instructions.

text_label:
	recip.d	$f4,$f6
	recip.s	$f4,$f6
	rsqrt.d	$f4,$f6
	rsqrt.s	$f4,$f6

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
