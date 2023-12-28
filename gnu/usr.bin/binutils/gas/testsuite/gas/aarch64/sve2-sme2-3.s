	bfmlslb	z0.s, z0.h, z0.h[0]
	BFMLSLB	Z0.S, Z0.H, Z0.H[0]
	bfmlslb	z31.s, z0.h, z0.h[0]
	bfmlslb	z0.s, z31.h, z0.h[0]
	bfmlslb	z0.s, z0.h, z7.h[0]
	bfmlslb	z0.s, z0.h, z0.h[7]
	bfmlslb	z5.s, z22.h, z4.h[3]

	movprfx	z0, z1; bfmlslb	z0.s, z1.h, z1.h[0]

	bfmlslb	z0.s, z0.h, z0.h
	bfmlslb	z31.s, z0.h, z0.h
	bfmlslb	z0.s, z31.h, z0.h
	bfmlslb	z0.s, z0.h, z31.h
	bfmlslb	z25.s, z13.h, z6.h

	movprfx	z0, z1; bfmlslb	z0.s, z1.h, z2.h

	bfmlslt	z0.s, z0.h, z0.h[0]
	BFMLSLT	Z0.S, Z0.H, Z0.H[0]
	bfmlslt	z31.s, z0.h, z0.h[0]
	bfmlslt	z0.s, z31.h, z0.h[0]
	bfmlslt	z0.s, z0.h, z7.h[0]
	bfmlslt	z0.s, z0.h, z0.h[7]
	bfmlslt	z5.s, z22.h, z4.h[3]

	movprfx	z0, z1; bfmlslt	z0.s, z1.h, z1.h[0]

	bfmlslt	z0.s, z0.h, z0.h
	bfmlslt	z31.s, z0.h, z0.h
	bfmlslt	z0.s, z31.h, z0.h
	bfmlslt	z0.s, z0.h, z31.h
	bfmlslt	z25.s, z13.h, z6.h

	movprfx	z0, z1; bfmlslt	z0.s, z1.h, z2.h
