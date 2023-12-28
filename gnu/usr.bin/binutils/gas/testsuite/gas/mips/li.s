# Source file used to test the li macro.

foo:
	li	$4,0
	li	$4,1
	li	$4,0x8000
	li	$4,-0x8000
	li	$4,0x10000
	li	$4,0x1a5a5

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
