	sdot	za.d[w8, 0], { z0.h - z1.h }, z0.h[0]
	sdot	za.d[w8, 0, vgx2], { z0.h - z1.h }, z0.h[0]
	SDOT	ZA.d[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	SDOT	ZA.D[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	sdot	za.d[w11, 0], { z0.h - z1.h }, z0.h[0]
	sdot	za.d[w8, 7], { z0.h - z1.h }, z0.h[0]
	sdot	za.d[w8, 0], { z30.h - z31.h }, z0.h[0]
	sdot	za.d[w8, 0], { z0.h - z1.h }, z15.h[0]
	sdot	za.d[w8, 0], { z0.h - z1.h }, z0.h[1]
	sdot	za.d[w10, 2], { z14.h - z15.h }, z13.h[1]

	sdot	za.d[w8, 0], { z0.h - z3.h }, z0.h[0]
	sdot	za.d[w8, 0, vgx4], { z0.h - z3.h }, z0.h[0]
	SDOT	ZA.d[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	SDOT	ZA.D[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	sdot	za.d[w11, 0], { z0.h - z3.h }, z0.h[0]
	sdot	za.d[w8, 7], { z0.h - z3.h }, z0.h[0]
	sdot	za.d[w8, 0], { z28.h - z31.h }, z0.h[0]
	sdot	za.d[w8, 0], { z0.h - z3.h }, z15.h[0]
	sdot	za.d[w8, 0], { z0.h - z3.h }, z0.h[1]
	sdot	za.d[w9, 1], { z4.h - z7.h }, z10.h[1]

	sdot	za.d[w8, 0], { z0.h - z1.h }, z0.h
	sdot	za.d[w8, 0, vgx2], { z0.h - z1.h }, z0.h
	SDOT	ZA.d[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h
	SDOT	ZA.D[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H
	sdot	za.d[w11, 0], { z0.h - z1.h }, z0.h
	sdot	za.d[w8, 7], { z0.h - z1.h }, z0.h
	sdot	za.d[w8, 0], { z30.h - z31.h }, z0.h
	sdot	za.d[w8, 0], { z31.h, z0.h }, z0.h
	sdot	za.d[w8, 0], { z31.h - z0.h }, z0.h
	sdot	za.d[w8, 0], { z0.h - z1.h }, z15.h
	sdot	za.d[w9, 3], { z21.h - z22.h }, z9.h

	sdot	za.d[w8, 0], { z0.h - z3.h }, z0.h
	sdot	za.d[w8, 0, vgx4], { z0.h - z3.h }, z0.h
	SDOT	ZA.d[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h
	SDOT	ZA.D[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H
	sdot	za.d[w11, 0], { z0.h - z3.h }, z0.h
	sdot	za.d[w8, 7], { z0.h - z3.h }, z0.h
	sdot	za.d[w8, 0], { z28.h - z31.h }, z0.h
	sdot	za.d[w8, 0], { z30.h, z31.h, z0.h, z1.h }, z0.h
	sdot	za.d[w8, 0], { z30.h - z1.h }, z0.h
	sdot	za.d[w8, 0], { z31.h, z0.h, z1.h, z2.h }, z0.h
	sdot	za.d[w8, 0], { z31.h - z2.h }, z0.h
	sdot	za.d[w8, 0], { z0.h - z3.h }, z15.h
	sdot	za.d[w10, 5], { z17.h - z20.h }, z3.h

	sdot	za.d[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	sdot	za.d[w8, 0, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	SDOT	ZA.d[W8, 0, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	SDOT	ZA.D[W8, 0, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	sdot	za.d[w11, 0], { z0.h - z1.h }, { z0.h - z1.h }
	sdot	za.d[w8, 7], { z0.h - z1.h }, { z0.h - z1.h }
	sdot	za.d[w8, 0], { z30.h - z31.h }, { z0.h - z1.h }
	sdot	za.d[w8, 0], { z0.h - z1.h }, { z30.h - z31.h }
	sdot	za.d[w10, 1], { z22.h - z23.h }, { z18.h - z19.h }

	sdot	za.d[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
	sdot	za.d[w8, 0, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	SDOT	ZA.d[W8, 0, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	SDOT	ZA.D[W8, 0, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	sdot	za.d[w11, 0], { z0.h - z3.h }, { z0.h - z3.h }
	sdot	za.d[w8, 7], { z0.h - z3.h }, { z0.h - z3.h }
	sdot	za.d[w8, 0], { z28.h - z31.h }, { z0.h - z3.h }
	sdot	za.d[w8, 0], { z0.h - z3.h }, { z28.h - z31.h }
	sdot	za.d[w11, 3], { z16.h - z19.h }, { z24.h - z27.h }

	udot	za.d[w8, 0], { z0.h - z1.h }, z0.h[0]
	udot	za.d[w8, 0, vgx2], { z0.h - z1.h }, z0.h[0]
	UDOT	ZA.d[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	UDOT	ZA.D[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	udot	za.d[w11, 0], { z0.h - z1.h }, z0.h[0]
	udot	za.d[w8, 7], { z0.h - z1.h }, z0.h[0]
	udot	za.d[w8, 0], { z30.h - z31.h }, z0.h[0]
	udot	za.d[w8, 0], { z0.h - z1.h }, z15.h[0]
	udot	za.d[w8, 0], { z0.h - z1.h }, z0.h[1]
	udot	za.d[w10, 2], { z14.h - z15.h }, z13.h[1]

	udot	za.d[w8, 0], { z0.h - z3.h }, z0.h[0]
	udot	za.d[w8, 0, vgx4], { z0.h - z3.h }, z0.h[0]
	UDOT	ZA.d[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	UDOT	ZA.D[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	udot	za.d[w11, 0], { z0.h - z3.h }, z0.h[0]
	udot	za.d[w8, 7], { z0.h - z3.h }, z0.h[0]
	udot	za.d[w8, 0], { z28.h - z31.h }, z0.h[0]
	udot	za.d[w8, 0], { z0.h - z3.h }, z15.h[0]
	udot	za.d[w8, 0], { z0.h - z3.h }, z0.h[1]
	udot	za.d[w9, 1], { z4.h - z7.h }, z10.h[0]

	udot	za.d[w8, 0], { z0.h - z1.h }, z0.h
	udot	za.d[w8, 0, vgx2], { z0.h - z1.h }, z0.h
	UDOT	ZA.d[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h
	UDOT	ZA.D[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H
	udot	za.d[w11, 0], { z0.h - z1.h }, z0.h
	udot	za.d[w8, 7], { z0.h - z1.h }, z0.h
	udot	za.d[w8, 0], { z30.h - z31.h }, z0.h
	udot	za.d[w8, 0], { z31.h, z0.h }, z0.h
	udot	za.d[w8, 0], { z31.h - z0.h }, z0.h
	udot	za.d[w8, 0], { z0.h - z1.h }, z15.h
	udot	za.d[w9, 3], { z21.h - z22.h }, z9.h

	udot	za.d[w8, 0], { z0.h - z3.h }, z0.h
	udot	za.d[w8, 0, vgx4], { z0.h - z3.h }, z0.h
	UDOT	ZA.d[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h
	UDOT	ZA.D[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H
	udot	za.d[w11, 0], { z0.h - z3.h }, z0.h
	udot	za.d[w8, 7], { z0.h - z3.h }, z0.h
	udot	za.d[w8, 0], { z28.h - z31.h }, z0.h
	udot	za.d[w8, 0], { z30.h, z31.h, z0.h, z1.h }, z0.h
	udot	za.d[w8, 0], { z30.h - z1.h }, z0.h
	udot	za.d[w8, 0], { z31.h, z0.h, z1.h, z2.h }, z0.h
	udot	za.d[w8, 0], { z31.h - z2.h }, z0.h
	udot	za.d[w8, 0], { z0.h - z3.h }, z15.h
	udot	za.d[w10, 5], { z17.h - z20.h }, z3.h

	udot	za.d[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	udot	za.d[w8, 0, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	UDOT	ZA.d[W8, 0, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	UDOT	ZA.D[W8, 0, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	udot	za.d[w11, 0], { z0.h - z1.h }, { z0.h - z1.h }
	udot	za.d[w8, 7], { z0.h - z1.h }, { z0.h - z1.h }
	udot	za.d[w8, 0], { z30.h - z31.h }, { z0.h - z1.h }
	udot	za.d[w8, 0], { z0.h - z1.h }, { z30.h - z31.h }
	udot	za.d[w10, 1], { z22.h - z23.h }, { z18.h - z19.h }

	udot	za.d[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
	udot	za.d[w8, 0, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	UDOT	ZA.d[W8, 0, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	UDOT	ZA.D[W8, 0, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	udot	za.d[w11, 0], { z0.h - z3.h }, { z0.h - z3.h }
	udot	za.d[w8, 7], { z0.h - z3.h }, { z0.h - z3.h }
	udot	za.d[w8, 0], { z28.h - z31.h }, { z0.h - z3.h }
	udot	za.d[w8, 0], { z0.h - z3.h }, { z28.h - z31.h }
	udot	za.d[w11, 3], { z16.h - z19.h }, { z24.h - z27.h }
