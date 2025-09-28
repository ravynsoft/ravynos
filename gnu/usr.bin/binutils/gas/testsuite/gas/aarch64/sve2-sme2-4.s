	fdot	z0.s, z0.h, z0.h[0]
	FDOT	Z0.S, Z0.H, Z0.H[0]
	fdot	z31.s, z0.h, z0.h[0]
	fdot	z0.s, z31.h, z0.h[0]
	fdot	z0.s, z0.h, z7.h[0]
	fdot	z0.s, z0.h, z0.h[3]

	movprfx	z0, z1; fdot	z0.s, z1.h, z1.h[0]

	fdot	z0.s, z0.h, z0.h
	fdot	z31.s, z0.h, z0.h
	fdot	z0.s, z31.h, z0.h
	fdot	z0.s, z0.h, z31.h
	fdot	z14.s, z26.h, z9.h

	movprfx	z0, z1; fdot	z0.s, z1.h, z2.h

	sdot	z0.s, z0.h, z0.h[0]
	SDOT	Z0.S, Z0.H, Z0.H[0]
	sdot	z31.s, z0.h, z0.h[0]
	sdot	z0.s, z31.h, z0.h[0]
	sdot	z0.s, z0.h, z7.h[0]
	sdot	z0.s, z0.h, z0.h[3]

	movprfx	z0, z1; sdot	z0.s, z1.h, z1.h[0]

	sdot	z0.s, z0.h, z0.h
	sdot	z31.s, z0.h, z0.h
	sdot	z0.s, z31.h, z0.h
	sdot	z0.s, z0.h, z31.h
	sdot	z14.s, z26.h, z9.h

	movprfx	z0, z1; sdot	z0.s, z1.h, z2.h
	udot	z0.s, z0.h, z0.h[0]
	UDOT	Z0.S, Z0.H, Z0.H[0]
	udot	z31.s, z0.h, z0.h[0]
	udot	z0.s, z31.h, z0.h[0]
	udot	z0.s, z0.h, z7.h[0]
	udot	z0.s, z0.h, z0.h[3]

	movprfx	z0, z1; udot	z0.s, z1.h, z1.h[0]

	udot	z0.s, z0.h, z0.h
	udot	z31.s, z0.h, z0.h
	udot	z0.s, z31.h, z0.h
	udot	z0.s, z0.h, z31.h
	udot	z14.s, z26.h, z9.h

	movprfx	z0, z1; udot	z0.s, z1.h, z2.h
