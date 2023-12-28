	bfdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[0]
	bfdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h[0]
	BFDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	BFDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	bfdot	za.s[w11, 0], { z0.h - z1.h }, z0.h[0]
	bfdot	za.s[w8, 7], { z0.h - z1.h }, z0.h[0]
	bfdot	za.s[w8, 0], { z30.h - z31.h }, z0.h[0]
	bfdot	za.s[w8, 0], { z0.h - z1.h }, z15.h[0]
	bfdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[3]
	bfdot	za.s[w10, 2], { z14.h - z15.h }, z13.h[1]

	bfdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	bfdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h[0]
	BFDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	BFDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	bfdot	za.s[w11, 0], { z0.h - z3.h }, z0.h[0]
	bfdot	za.s[w8, 7], { z0.h - z3.h }, z0.h[0]
	bfdot	za.s[w8, 0], { z28.h - z31.h }, z0.h[0]
	bfdot	za.s[w8, 0], { z0.h - z3.h }, z15.h[0]
	bfdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[3]
	bfdot	za.s[w9, 1], { z4.h - z7.h }, z10.h[2]

	bfdot	za.s[w8, 0], { z0.h - z1.h }, z0.h
	bfdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h
	BFDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h
	BFDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H
	bfdot	za.s[w11, 0], { z0.h - z1.h }, z0.h
	bfdot	za.s[w8, 7], { z0.h - z1.h }, z0.h
	bfdot	za.s[w8, 0], { z30.h - z31.h }, z0.h
	bfdot	za.s[w8, 0], { z31.h, z0.h }, z0.h
	bfdot	za.s[w8, 0], { z31.h - z0.h }, z0.h
	bfdot	za.s[w8, 0], { z0.h - z1.h }, z15.h
	bfdot	za.s[w9, 3], { z21.h - z22.h }, z9.h

	bfdot	za.s[w8, 0], { z0.h - z3.h }, z0.h
	bfdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h
	BFDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h
	BFDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H
	bfdot	za.s[w11, 0], { z0.h - z3.h }, z0.h
	bfdot	za.s[w8, 7], { z0.h - z3.h }, z0.h
	bfdot	za.s[w8, 0], { z28.h - z31.h }, z0.h
	bfdot	za.s[w8, 0], { z30.h, z31.h, z0.h, z1.h }, z0.h
	bfdot	za.s[w8, 0], { z30.h - z1.h }, z0.h
	bfdot	za.s[w8, 0], { z31.h, z0.h, z1.h, z2.h }, z0.h
	bfdot	za.s[w8, 0], { z31.h - z2.h }, z0.h
	bfdot	za.s[w8, 0], { z0.h - z3.h }, z15.h
	bfdot	za.s[w10, 5], { z17.h - z20.h }, z3.h

	bfdot	za.s[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	bfdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	BFDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	BFDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	bfdot	za.s[w11, 0], { z0.h - z1.h }, { z0.h - z1.h }
	bfdot	za.s[w8, 7], { z0.h - z1.h }, { z0.h - z1.h }
	bfdot	za.s[w8, 0], { z30.h - z31.h }, { z0.h - z1.h }
	bfdot	za.s[w8, 0], { z0.h - z1.h }, { z30.h - z31.h }
	bfdot	za.s[w10, 1], { z22.h - z23.h }, { z18.h - z19.h }

	bfdot	za.s[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
	bfdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	BFDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	BFDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	bfdot	za.s[w11, 0], { z0.h - z3.h }, { z0.h - z3.h }
	bfdot	za.s[w8, 7], { z0.h - z3.h }, { z0.h - z3.h }
	bfdot	za.s[w8, 0], { z28.h - z31.h }, { z0.h - z3.h }
	bfdot	za.s[w8, 0], { z0.h - z3.h }, { z28.h - z31.h }
	bfdot	za.s[w11, 3], { z16.h - z19.h }, { z24.h - z27.h }

	fdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[0]
	fdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h[0]
	FDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	FDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	fdot	za.s[w11, 0], { z0.h - z1.h }, z0.h[0]
	fdot	za.s[w8, 7], { z0.h - z1.h }, z0.h[0]
	fdot	za.s[w8, 0], { z30.h - z31.h }, z0.h[0]
	fdot	za.s[w8, 0], { z0.h - z1.h }, z15.h[0]
	fdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[3]
	fdot	za.s[w10, 2], { z14.h - z15.h }, z13.h[1]

	fdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	fdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h[0]
	FDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	FDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	fdot	za.s[w11, 0], { z0.h - z3.h }, z0.h[0]
	fdot	za.s[w8, 7], { z0.h - z3.h }, z0.h[0]
	fdot	za.s[w8, 0], { z28.h - z31.h }, z0.h[0]
	fdot	za.s[w8, 0], { z0.h - z3.h }, z15.h[0]
	fdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[3]
	fdot	za.s[w9, 1], { z4.h - z7.h }, z10.h[2]

	fdot	za.s[w8, 0], { z0.h - z1.h }, z0.h
	fdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, z0.h
	FDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, Z0.h
	FDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, Z0.H
	fdot	za.s[w11, 0], { z0.h - z1.h }, z0.h
	fdot	za.s[w8, 7], { z0.h - z1.h }, z0.h
	fdot	za.s[w8, 0], { z30.h - z31.h }, z0.h
	fdot	za.s[w8, 0], { z31.h, z0.h }, z0.h
	fdot	za.s[w8, 0], { z31.h - z0.h }, z0.h
	fdot	za.s[w8, 0], { z0.h - z1.h }, z15.h
	fdot	za.s[w9, 3], { z21.h - z22.h }, z9.h

	fdot	za.s[w8, 0], { z0.h - z3.h }, z0.h
	fdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, z0.h
	FDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, Z0.h
	FDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, Z0.H
	fdot	za.s[w11, 0], { z0.h - z3.h }, z0.h
	fdot	za.s[w8, 7], { z0.h - z3.h }, z0.h
	fdot	za.s[w8, 0], { z28.h - z31.h }, z0.h
	fdot	za.s[w8, 0], { z30.h, z31.h, z0.h, z1.h }, z0.h
	fdot	za.s[w8, 0], { z30.h - z1.h }, z0.h
	fdot	za.s[w8, 0], { z31.h, z0.h, z1.h, z2.h }, z0.h
	fdot	za.s[w8, 0], { z31.h - z2.h }, z0.h
	fdot	za.s[w8, 0], { z0.h - z3.h }, z15.h
	fdot	za.s[w10, 5], { z17.h - z20.h }, z3.h

	fdot	za.s[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	fdot	za.s[w8, 0, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	FDOT	ZA.s[W8, 0, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	FDOT	ZA.S[W8, 0, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	fdot	za.s[w11, 0], { z0.h - z1.h }, { z0.h - z1.h }
	fdot	za.s[w8, 7], { z0.h - z1.h }, { z0.h - z1.h }
	fdot	za.s[w8, 0], { z30.h - z31.h }, { z0.h - z1.h }
	fdot	za.s[w8, 0], { z0.h - z1.h }, { z30.h - z31.h }
	fdot	za.s[w10, 1], { z22.h - z23.h }, { z18.h - z19.h }

	fdot	za.s[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
	fdot	za.s[w8, 0, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	FDOT	ZA.s[W8, 0, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	FDOT	ZA.S[W8, 0, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	fdot	za.s[w11, 0], { z0.h - z3.h }, { z0.h - z3.h }
	fdot	za.s[w8, 7], { z0.h - z3.h }, { z0.h - z3.h }
	fdot	za.s[w8, 0], { z28.h - z31.h }, { z0.h - z3.h }
	fdot	za.s[w8, 0], { z0.h - z3.h }, { z28.h - z31.h }
	fdot	za.s[w11, 3], { z16.h - z19.h }, { z24.h - z27.h }

	usdot	za.s[w8, 0], { z0.b - z1.b }, z0.b[0]
	usdot	za.s[w8, 0, vgx2], { z0.b - z1.b }, z0.b[0]
	USDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, Z0.b[0]
	USDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, Z0.B[0]
	usdot	za.s[w11, 0], { z0.b - z1.b }, z0.b[0]
	usdot	za.s[w8, 7], { z0.b - z1.b }, z0.b[0]
	usdot	za.s[w8, 0], { z30.b - z31.b }, z0.b[0]
	usdot	za.s[w8, 0], { z0.b - z1.b }, z15.b[0]
	usdot	za.s[w8, 0], { z0.b - z1.b }, z0.b[3]
	usdot	za.s[w10, 2], { z14.b - z15.b }, z13.b[1]

	usdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[0]
	usdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.b[0]
	USDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, Z0.b[0]
	USDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, Z0.B[0]
	usdot	za.s[w11, 0], { z0.b - z3.b }, z0.b[0]
	usdot	za.s[w8, 7], { z0.b - z3.b }, z0.b[0]
	usdot	za.s[w8, 0], { z28.b - z31.b }, z0.b[0]
	usdot	za.s[w8, 0], { z0.b - z3.b }, z15.b[0]
	usdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[3]
	usdot	za.s[w9, 1], { z4.b - z7.b }, z10.b[2]

	usdot	za.s[w8, 0], { z0.b - z1.b }, z0.b
	usdot	za.s[w8, 0, vgx2], { z0.b - z1.b }, z0.b
	USDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, Z0.b
	USDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, Z0.B
	usdot	za.s[w11, 0], { z0.b - z1.b }, z0.b
	usdot	za.s[w8, 7], { z0.b - z1.b }, z0.b
	usdot	za.s[w8, 0], { z30.b - z31.b }, z0.b
	usdot	za.s[w8, 0], { z31.b, z0.b }, z0.b
	usdot	za.s[w8, 0], { z31.b - z0.b }, z0.b
	usdot	za.s[w8, 0], { z0.b - z1.b }, z15.b
	usdot	za.s[w9, 3], { z21.b - z22.b }, z9.b

	usdot	za.s[w8, 0], { z0.b - z3.b }, z0.b
	usdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.b
	USDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, Z0.b
	USDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, Z0.B
	usdot	za.s[w11, 0], { z0.b - z3.b }, z0.b
	usdot	za.s[w8, 7], { z0.b - z3.b }, z0.b
	usdot	za.s[w8, 0], { z28.b - z31.b }, z0.b
	usdot	za.s[w8, 0], { z30.b, z31.b, z0.b, z1.b }, z0.b
	usdot	za.s[w8, 0], { z30.b - z1.b }, z0.b
	usdot	za.s[w8, 0], { z31.b, z0.b, z1.b, z2.b }, z0.b
	usdot	za.s[w8, 0], { z31.b - z2.b }, z0.b
	usdot	za.s[w8, 0], { z0.b - z3.b }, z15.b
	usdot	za.s[w10, 5], { z17.b - z20.b }, z3.b

	usdot	za.s[w8, 0], { z0.b - z1.b }, { z0.b - z1.b }
	usdot	za.s[w8, 0, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	USDOT	ZA.s[W8, 0, VGx2], { Z0.b - Z1.b }, { Z0.b - Z1.b }
	USDOT	ZA.S[W8, 0, VGX2], { Z0.B - Z1.B }, { Z0.B - Z1.B }
	usdot	za.s[w11, 0], { z0.b - z1.b }, { z0.b - z1.b }
	usdot	za.s[w8, 7], { z0.b - z1.b }, { z0.b - z1.b }
	usdot	za.s[w8, 0], { z30.b - z31.b }, { z0.b - z1.b }
	usdot	za.s[w8, 0], { z0.b - z1.b }, { z30.b - z31.b }
	usdot	za.s[w10, 1], { z22.b - z23.b }, { z18.b - z19.b }

	usdot	za.s[w8, 0], { z0.b - z3.b }, { z0.b - z3.b }
	usdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	USDOT	ZA.s[W8, 0, VGx4], { Z0.b - Z3.b }, { Z0.b - Z3.b }
	USDOT	ZA.S[W8, 0, VGX4], { Z0.B - Z3.B }, { Z0.B - Z3.B }
	usdot	za.s[w11, 0], { z0.b - z3.b }, { z0.b - z3.b }
	usdot	za.s[w8, 7], { z0.b - z3.b }, { z0.b - z3.b }
	usdot	za.s[w8, 0], { z28.b - z31.b }, { z0.b - z3.b }
	usdot	za.s[w8, 0], { z0.b - z3.b }, { z28.b - z31.b }
	usdot	za.s[w11, 3], { z16.b - z19.b }, { z24.b - z27.b }
