	.set	reorder
	daddi	$23,$24,1023
	ddiv $2, $4
	ddivu $2, $4
	dmult	$2,$3
	dmultu	$2,$3
	ldl	$2, 1($3)
	ldr	$2, 1($3)
	sdl	$2, 1($3)
	sdr	$2, 1($3)
