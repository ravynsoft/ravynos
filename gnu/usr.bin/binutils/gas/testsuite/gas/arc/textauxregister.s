	.extAuxRegister mlx, 0x30, r|w
	.extAuxRegister mly, 0x31, r|w

	aex	r0,[mlx]
	lr	r1,[mly]
	sr	0x12,[mlx]
