	.text
	.set	noat
	.set	noreorder
	.set	nomacro
test_r5:
	eretnc

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  2
	.space  8
