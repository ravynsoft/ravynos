	.arch	armv9.3-a
	sb
	rshrnb	z0.h, z0.s, #1
	bfcvt	z0.h, p0/m, z0.s
	smmla	z0.s, z1.b, z2.b
	ld64b	x0, [x8]
	cpyfp	[x0]!, [x1]!, x30!
	cpyfm	[x0]!, [x1]!, x30!
	cpyfe	[x0]!, [x1]!, x30!
1:
	bc.eq	1b
