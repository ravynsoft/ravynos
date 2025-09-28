	lui	$27, %hi(kernelsp)
	lw	$27, %lo(kernelsp)($27)
	.set	noreorder
	mfc0	$28, $14
	addu	$28, 4
