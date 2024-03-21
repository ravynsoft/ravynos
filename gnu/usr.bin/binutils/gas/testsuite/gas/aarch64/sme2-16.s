	sdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[0]
	sdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h[0]
	SDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	SDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	sdot	za.s[w11, 0], { z0.h - z1.h }, z0.h[0]
	sdot	za.s[w8, 7], { z0.h - z1.h }, z0.h[0]
	sdot	za.s[w8, 0], { z30.h - z31.h }, z0.h[0]
	sdot	za.s[w8, 0], { z0.h - z1.h }, z15.h[0]
	sdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[3]
	sdot	za.s[w10, 2], { z14.h - z15.h }, z13.h[1]

	sdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	sdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h[0]
	SDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	SDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	sdot	za.s[w11, 0], { z0.h - z3.h }, z0.h[0]
	sdot	za.s[w8, 7], { z0.h - z3.h }, z0.h[0]
	sdot	za.s[w8, 0], { z28.h - z31.h }, z0.h[0]
	sdot	za.s[w8, 0], { z0.h - z3.h }, z15.h[0]
	sdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[3]
	sdot	za.s[w9, 1], { z4.h - z7.h }, z10.h[2]

	sdot	za.s[w8, 0], { z0.h - z1.h }, z0.h
	sdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h
	SDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h
	SDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H
	sdot	za.s[w11, 0], { z0.h - z1.h }, z0.h
	sdot	za.s[w8, 7], { z0.h - z1.h }, z0.h
	sdot	za.s[w8, 0], { z30.h - z31.h }, z0.h
	sdot	za.s[w8, 0], { z31.h, z0.h }, z0.h
	sdot	za.s[w8, 0], { z31.h - z0.h }, z0.h
	sdot	za.s[w8, 0], { z0.h - z1.h }, z15.h
	sdot	za.s[w9, 3], { z21.h - z22.h }, z9.h

	sdot	za.s[w8, 0], { z0.h - z3.h }, z0.h
	sdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h
	SDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h
	SDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H
	sdot	za.s[w11, 0], { z0.h - z3.h }, z0.h
	sdot	za.s[w8, 7], { z0.h - z3.h }, z0.h
	sdot	za.s[w8, 0], { z28.h - z31.h }, z0.h
	sdot	za.s[w8, 0], { z30.h, z31.h, z0.h, z1.h }, z0.h
	sdot	za.s[w8, 0], { z30.h - z1.h }, z0.h
	sdot	za.s[w8, 0], { z31.h, z0.h, z1.h, z2.h }, z0.h
	sdot	za.s[w8, 0], { z31.h - z2.h }, z0.h
	sdot	za.s[w8, 0], { z0.h - z3.h }, z15.h
	sdot	za.s[w10, 5], { z17.h - z20.h }, z3.h

	sdot	za.s[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	sdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	SDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	SDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	sdot	za.s[w11, 0], { z0.h - z1.h }, { z0.h - z1.h }
	sdot	za.s[w8, 7], { z0.h - z1.h }, { z0.h - z1.h }
	sdot	za.s[w8, 0], { z30.h - z31.h }, { z0.h - z1.h }
	sdot	za.s[w8, 0], { z0.h - z1.h }, { z30.h - z31.h }
	sdot	za.s[w10, 1], { z22.h - z23.h }, { z18.h - z19.h }

	sdot	za.s[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
	sdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	SDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	SDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	sdot	za.s[w11, 0], { z0.h - z3.h }, { z0.h - z3.h }
	sdot	za.s[w8, 7], { z0.h - z3.h }, { z0.h - z3.h }
	sdot	za.s[w8, 0], { z28.h - z31.h }, { z0.h - z3.h }
	sdot	za.s[w8, 0], { z0.h - z3.h }, { z28.h - z31.h }
	sdot	za.s[w11, 3], { z16.h - z19.h }, { z24.h - z27.h }

	sdot	za.s[w8, 0], { z0.b - z1.b }, z0.b[0]
	sdot	za.s[w8, 0, vgx2], { z0.b - z1.b }, z0.b[0]
	SDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, Z0.b[0]
	SDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, Z0.B[0]
	sdot	za.s[w11, 0], { z0.b - z1.b }, z0.b[0]
	sdot	za.s[w8, 7], { z0.b - z1.b }, z0.b[0]
	sdot	za.s[w8, 0], { z30.b - z31.b }, z0.b[0]
	sdot	za.s[w8, 0], { z0.b - z1.b }, z15.b[0]
	sdot	za.s[w8, 0], { z0.b - z1.b }, z0.b[3]
	sdot	za.s[w10, 2], { z14.b - z15.b }, z13.b[1]

	sdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[0]
	sdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.b[0]
	SDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, Z0.b[0]
	SDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, Z0.B[0]
	sdot	za.s[w11, 0], { z0.b - z3.b }, z0.b[0]
	sdot	za.s[w8, 7], { z0.b - z3.b }, z0.b[0]
	sdot	za.s[w8, 0], { z28.b - z31.b }, z0.b[0]
	sdot	za.s[w8, 0], { z0.b - z3.b }, z15.b[0]
	sdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[3]
	sdot	za.s[w9, 1], { z4.b - z7.b }, z10.b[2]

	sdot	za.s[w8, 0], { z0.b - z1.b }, z0.b
	sdot	za.s[w8, 0, vgx2], { z0.b - z1.b }, z0.b
	SDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, Z0.b
	SDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, Z0.B
	sdot	za.s[w11, 0], { z0.b - z1.b }, z0.b
	sdot	za.s[w8, 7], { z0.b - z1.b }, z0.b
	sdot	za.s[w8, 0], { z30.b - z31.b }, z0.b
	sdot	za.s[w8, 0], { z31.b, z0.b }, z0.b
	sdot	za.s[w8, 0], { z31.b - z0.b }, z0.b
	sdot	za.s[w8, 0], { z0.b - z1.b }, z15.b
	sdot	za.s[w9, 3], { z21.b - z22.b }, z9.b

	sdot	za.s[w8, 0], { z0.b - z3.b }, z0.b
	sdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.b
	SDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, Z0.b
	SDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, Z0.B
	sdot	za.s[w11, 0], { z0.b - z3.b }, z0.b
	sdot	za.s[w8, 7], { z0.b - z3.b }, z0.b
	sdot	za.s[w8, 0], { z28.b - z31.b }, z0.b
	sdot	za.s[w8, 0], { z30.b, z31.b, z0.b, z1.b }, z0.b
	sdot	za.s[w8, 0], { z30.b - z1.b }, z0.b
	sdot	za.s[w8, 0], { z31.b, z0.b, z1.b, z2.b }, z0.b
	sdot	za.s[w8, 0], { z31.b - z2.b }, z0.b
	sdot	za.s[w8, 0], { z0.b - z3.b }, z15.b
	sdot	za.s[w10, 5], { z17.b - z20.b }, z3.b

	sdot	za.s[w8, 0], { z0.b - z1.b }, { z0.b - z1.b }
	sdot	za.s[w8, 0, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	SDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, { Z0.b - Z1.b }
	SDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, { Z0.B - Z1.B }
	sdot	za.s[w11, 0], { z0.b - z1.b }, { z0.b - z1.b }
	sdot	za.s[w8, 7], { z0.b - z1.b }, { z0.b - z1.b }
	sdot	za.s[w8, 0], { z30.b - z31.b }, { z0.b - z1.b }
	sdot	za.s[w8, 0], { z0.b - z1.b }, { z30.b - z31.b }
	sdot	za.s[w10, 1], { z22.b - z23.b }, { z18.b - z19.b }

	sdot	za.s[w8, 0], { z0.b - z3.b }, { z0.b - z3.b }
	sdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	SDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, { Z0.b - Z3.b }
	SDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, { Z0.B - Z3.B }
	sdot	za.s[w11, 0], { z0.b - z3.b }, { z0.b - z3.b }
	sdot	za.s[w8, 7], { z0.b - z3.b }, { z0.b - z3.b }
	sdot	za.s[w8, 0], { z28.b - z31.b }, { z0.b - z3.b }
	sdot	za.s[w8, 0], { z0.b - z3.b }, { z28.b - z31.b }
	sdot	za.s[w11, 3], { z16.b - z19.b }, { z24.b - z27.b }

	udot	za.s[w8, 0], { z0.h - z1.h }, z0.h[0]
	udot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h[0]
	UDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	UDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	udot	za.s[w11, 0], { z0.h - z1.h }, z0.h[0]
	udot	za.s[w8, 7], { z0.h - z1.h }, z0.h[0]
	udot	za.s[w8, 0], { z30.h - z31.h }, z0.h[0]
	udot	za.s[w8, 0], { z0.h - z1.h }, z15.h[0]
	udot	za.s[w8, 0], { z0.h - z1.h }, z0.h[3]
	udot	za.s[w10, 2], { z14.h - z15.h }, z13.h[1]

	udot	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	udot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h[0]
	UDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	UDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	udot	za.s[w11, 0], { z0.h - z3.h }, z0.h[0]
	udot	za.s[w8, 7], { z0.h - z3.h }, z0.h[0]
	udot	za.s[w8, 0], { z28.h - z31.h }, z0.h[0]
	udot	za.s[w8, 0], { z0.h - z3.h }, z15.h[0]
	udot	za.s[w8, 0], { z0.h - z3.h }, z0.h[3]
	udot	za.s[w9, 1], { z4.h - z7.h }, z10.h[2]

	udot	za.s[w8, 0], { z0.h - z1.h }, z0.h
	udot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h
	UDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h
	UDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H
	udot	za.s[w11, 0], { z0.h - z1.h }, z0.h
	udot	za.s[w8, 7], { z0.h - z1.h }, z0.h
	udot	za.s[w8, 0], { z30.h - z31.h }, z0.h
	udot	za.s[w8, 0], { z31.h, z0.h }, z0.h
	udot	za.s[w8, 0], { z31.h - z0.h }, z0.h
	udot	za.s[w8, 0], { z0.h - z1.h }, z15.h
	udot	za.s[w9, 3], { z21.h - z22.h }, z9.h

	udot	za.s[w8, 0], { z0.h - z3.h }, z0.h
	udot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h
	UDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h
	UDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H
	udot	za.s[w11, 0], { z0.h - z3.h }, z0.h
	udot	za.s[w8, 7], { z0.h - z3.h }, z0.h
	udot	za.s[w8, 0], { z28.h - z31.h }, z0.h
	udot	za.s[w8, 0], { z30.h, z31.h, z0.h, z1.h }, z0.h
	udot	za.s[w8, 0], { z30.h - z1.h }, z0.h
	udot	za.s[w8, 0], { z31.h, z0.h, z1.h, z2.h }, z0.h
	udot	za.s[w8, 0], { z31.h - z2.h }, z0.h
	udot	za.s[w8, 0], { z0.h - z3.h }, z15.h
	udot	za.s[w10, 5], { z17.h - z20.h }, z3.h

	udot	za.s[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	udot	za.s[w8, 0, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	UDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	UDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	udot	za.s[w11, 0], { z0.h - z1.h }, { z0.h - z1.h }
	udot	za.s[w8, 7], { z0.h - z1.h }, { z0.h - z1.h }
	udot	za.s[w8, 0], { z30.h - z31.h }, { z0.h - z1.h }
	udot	za.s[w8, 0], { z0.h - z1.h }, { z30.h - z31.h }
	udot	za.s[w10, 1], { z22.h - z23.h }, { z18.h - z19.h }

	udot	za.s[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
	udot	za.s[w8, 0, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	UDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	UDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	udot	za.s[w11, 0], { z0.h - z3.h }, { z0.h - z3.h }
	udot	za.s[w8, 7], { z0.h - z3.h }, { z0.h - z3.h }
	udot	za.s[w8, 0], { z28.h - z31.h }, { z0.h - z3.h }
	udot	za.s[w8, 0], { z0.h - z3.h }, { z28.h - z31.h }
	udot	za.s[w11, 3], { z16.h - z19.h }, { z24.h - z27.h }

	udot	za.s[w8, 0], { z0.b - z1.b }, z0.b[0]
	udot	za.s[w8, 0, vgx2], { z0.b - z1.b }, z0.b[0]
	UDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, Z0.b[0]
	UDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, Z0.B[0]
	udot	za.s[w11, 0], { z0.b - z1.b }, z0.b[0]
	udot	za.s[w8, 7], { z0.b - z1.b }, z0.b[0]
	udot	za.s[w8, 0], { z30.b - z31.b }, z0.b[0]
	udot	za.s[w8, 0], { z0.b - z1.b }, z15.b[0]
	udot	za.s[w8, 0], { z0.b - z1.b }, z0.b[3]
	udot	za.s[w10, 2], { z14.b - z15.b }, z13.b[1]

	udot	za.s[w8, 0], { z0.b - z3.b }, z0.b[0]
	udot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.b[0]
	UDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, Z0.b[0]
	UDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, Z0.B[0]
	udot	za.s[w11, 0], { z0.b - z3.b }, z0.b[0]
	udot	za.s[w8, 7], { z0.b - z3.b }, z0.b[0]
	udot	za.s[w8, 0], { z28.b - z31.b }, z0.b[0]
	udot	za.s[w8, 0], { z0.b - z3.b }, z15.b[0]
	udot	za.s[w8, 0], { z0.b - z3.b }, z0.b[3]
	udot	za.s[w9, 1], { z4.b - z7.b }, z10.b[2]

	udot	za.s[w8, 0], { z0.b - z1.b }, z0.b
	udot	za.s[w8, 0, vgx2], { z0.b - z1.b }, z0.b
	UDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, Z0.b
	UDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, Z0.B
	udot	za.s[w11, 0], { z0.b - z1.b }, z0.b
	udot	za.s[w8, 7], { z0.b - z1.b }, z0.b
	udot	za.s[w8, 0], { z30.b - z31.b }, z0.b
	udot	za.s[w8, 0], { z31.b, z0.b }, z0.b
	udot	za.s[w8, 0], { z31.b - z0.b }, z0.b
	udot	za.s[w8, 0], { z0.b - z1.b }, z15.b
	udot	za.s[w9, 3], { z21.b - z22.b }, z9.b

	udot	za.s[w8, 0], { z0.b - z3.b }, z0.b
	udot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.b
	UDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, Z0.b
	UDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, Z0.B
	udot	za.s[w11, 0], { z0.b - z3.b }, z0.b
	udot	za.s[w8, 7], { z0.b - z3.b }, z0.b
	udot	za.s[w8, 0], { z28.b - z31.b }, z0.b
	udot	za.s[w8, 0], { z30.b, z31.b, z0.b, z1.b }, z0.b
	udot	za.s[w8, 0], { z30.b - z1.b }, z0.b
	udot	za.s[w8, 0], { z31.b, z0.b, z1.b, z2.b }, z0.b
	udot	za.s[w8, 0], { z31.b - z2.b }, z0.b
	udot	za.s[w8, 0], { z0.b - z3.b }, z15.b
	udot	za.s[w10, 5], { z17.b - z20.b }, z3.b

	udot	za.s[w8, 0], { z0.b - z1.b }, { z0.b - z1.b }
	udot	za.s[w8, 0, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	UDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, { Z0.b - Z1.b }
	UDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, { Z0.B - Z1.B }
	udot	za.s[w11, 0], { z0.b - z1.b }, { z0.b - z1.b }
	udot	za.s[w8, 7], { z0.b - z1.b }, { z0.b - z1.b }
	udot	za.s[w8, 0], { z30.b - z31.b }, { z0.b - z1.b }
	udot	za.s[w8, 0], { z0.b - z1.b }, { z30.b - z31.b }
	udot	za.s[w10, 1], { z22.b - z23.b }, { z18.b - z19.b }

	udot	za.s[w8, 0], { z0.b - z3.b }, { z0.b - z3.b }
	udot	za.s[w8, 0, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	UDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, { Z0.b - Z3.b }
	UDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, { Z0.B - Z3.B }
	udot	za.s[w11, 0], { z0.b - z3.b }, { z0.b - z3.b }
	udot	za.s[w8, 7], { z0.b - z3.b }, { z0.b - z3.b }
	udot	za.s[w8, 0], { z28.b - z31.b }, { z0.b - z3.b }
	udot	za.s[w8, 0], { z0.b - z3.b }, { z28.b - z31.b }
	udot	za.s[w11, 3], { z16.b - z19.b }, { z24.b - z27.b }
