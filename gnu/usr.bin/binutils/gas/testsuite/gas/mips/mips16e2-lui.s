	.text
foo:
	lui	$2, %hi(bar)
	lui	$3, %hi(0f)
	lui	$4, %hi(baz)
	lui	$5, %hi(0x12345678)
	lui	$6, %lo(bar)
	lui	$7, %lo(0f)
	lui	$16, %lo(baz)
	lui	$17, %lo(0x12345678)
	lui	$2, 0x1234
	lui	$3, 1
0:
	.set	baz, 0x87654321

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
