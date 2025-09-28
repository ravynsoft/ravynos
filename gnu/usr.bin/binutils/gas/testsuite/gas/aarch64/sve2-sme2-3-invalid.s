	bfmlslb	0, z0.h, z0.h[0]
	bfmlslb	z0.s, 0, z0.h[0]
	bfmlslb	z0.s, z0.h, 0

	bfmlslb	z0.s, z0.h, z8.h[0]
	bfmlslb	z0.s, z0.h, z0.h[-1]
	bfmlslb	z0.s, z0.h, z0.h[8]
	bfmlslb	z0.h, z0.h, z0.h[0]
	bfmlslb	z0.d, z0.h, z0.h[0]

	movprfx	z0, z1; bfmlslb z0.s, z0.h, z1.h[0]
	movprfx	z0, z1; bfmlslb z0.s, z1.h, z0.h[0]
	movprfx	z3, z4; bfmlslb z0.s, z1.h, z2.h[0]
	movprfx	z0.s, p0/m, z1.s; bfmlslb z0.s, z1.h, z2.h[0]
	movprfx	z0.s, p0/z, z1.s; bfmlslb z0.s, z1.h, z2.h[0]
