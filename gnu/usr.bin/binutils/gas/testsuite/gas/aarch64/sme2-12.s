	bfmlal	za.s[w8, 0:1], z0.h, z0.h[0]
	BFMLAL	ZA.s[W8, 0:1], Z0.h, Z0.h[0]
	BFMLAL	ZA.S[W8, 0:1], Z0.H, Z0.H[0]
	bfmlal	za.s[w11, 0:1], z0.h, z0.h[0]
	bfmlal	za.s[w8, 14:15], z0.h, z0.h[0]
	bfmlal	za.s[w8, 0:1], z31.h, z0.h[0]
	bfmlal	za.s[w8, 0:1], z0.h, z15.h[0]
	bfmlal	za.s[w8, 0:1], z0.h, z0.h[7]
	bfmlal	za.s[w9, 10:11], z21.h, z9.h[2]

	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	BFMLAL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	BFMLAL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	bfmlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	bfmlal	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	BFMLAL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	BFMLAL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	bfmlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	bfmlal	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	bfmlal	za.s[w8, 0:1], z0.h, z0.h
	BFMLAL	ZA.s[W8, 0:1], Z0.h, Z0.h
	BFMLAL	ZA.S[W8, 0:1], Z0.H, Z0.H
	bfmlal	za.s[w11, 0:1], z0.h, z0.h
	bfmlal	za.s[w8, 14:15], z0.h, z0.h
	bfmlal	za.s[w8, 0:1], z31.h, z0.h
	bfmlal	za.s[w8, 0:1], z0.h, z15.h
	bfmlal	za.s[w10, 2:3], z25.h, z7.h

	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	BFMLAL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h
	BFMLAL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H
	bfmlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z1.h - z2.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z31.h, z0.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z31.h - z0.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	bfmlal	za.s[w9, 4:5], { z18.h - z19.h }, z13.h

	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	BFMLAL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h
	BFMLAL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H
	bfmlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w11, 0:1], { z1.h - z4.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z29.h, z30.h, z31.h, z0.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z29.h - z0.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z30.h, z31.h, z0.h, z1.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z30.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z31.h, z0.h, z1.h, z2.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z31.h - z2.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	bfmlal	za.s[w9, 4:5], { z24.h - z27.h }, z14.h

	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	BFMLAL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	BFMLAL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	bfmlal	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	bfmlal	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	BFMLAL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	BFMLAL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	bfmlal	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	bfmlal	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }

	bfmlsl	za.s[w8, 0:1], z0.h, z0.h[0]
	BFMLSL	ZA.s[W8, 0:1], Z0.h, Z0.h[0]
	BFMLSL	ZA.S[W8, 0:1], Z0.H, Z0.H[0]
	bfmlsl	za.s[w11, 0:1], z0.h, z0.h[0]
	bfmlsl	za.s[w8, 14:15], z0.h, z0.h[0]
	bfmlsl	za.s[w8, 0:1], z31.h, z0.h[0]
	bfmlsl	za.s[w8, 0:1], z0.h, z15.h[0]
	bfmlsl	za.s[w8, 0:1], z0.h, z0.h[7]
	bfmlsl	za.s[w9, 10:11], z21.h, z9.h[2]

	bfmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	bfmlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	BFMLSL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	BFMLSL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	bfmlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	bfmlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	bfmlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	bfmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	bfmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	bfmlsl	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	bfmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	bfmlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	BFMLSL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	BFMLSL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	bfmlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	bfmlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	bfmlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	bfmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	bfmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	bfmlsl	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	bfmlsl	za.s[w8, 0:1], z0.h, z0.h
	BFMLSL	ZA.s[W8, 0:1], Z0.h, Z0.h
	BFMLSL	ZA.S[W8, 0:1], Z0.H, Z0.H
	bfmlsl	za.s[w11, 0:1], z0.h, z0.h
	bfmlsl	za.s[w8, 14:15], z0.h, z0.h
	bfmlsl	za.s[w8, 0:1], z31.h, z0.h
	bfmlsl	za.s[w8, 0:1], z0.h, z15.h
	bfmlsl	za.s[w10, 2:3], z25.h, z7.h

	bfmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	bfmlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	BFMLSL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h
	BFMLSL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H
	bfmlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	bfmlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	bfmlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	bfmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	bfmlsl	za.s[w9, 4:5], { z18.h - z19.h }, z13.h

	bfmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	bfmlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	BFMLSL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h
	BFMLSL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H
	bfmlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	bfmlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	bfmlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	bfmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	bfmlsl	za.s[w9, 4:5], { z24.h - z27.h }, z14.h

	bfmlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	BFMLSL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	BFMLSL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	bfmlsl	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlsl	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlsl	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	bfmlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	bfmlsl	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	bfmlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	BFMLSL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	BFMLSL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	bfmlsl	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlsl	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlsl	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	bfmlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	bfmlsl	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }

	fmlal	za.s[w8, 0:1], z0.h, z0.h[0]
	FMLAL	ZA.s[W8, 0:1], Z0.h, Z0.h[0]
	FMLAL	ZA.S[W8, 0:1], Z0.H, Z0.H[0]
	fmlal	za.s[w11, 0:1], z0.h, z0.h[0]
	fmlal	za.s[w8, 14:15], z0.h, z0.h[0]
	fmlal	za.s[w8, 0:1], z31.h, z0.h[0]
	fmlal	za.s[w8, 0:1], z0.h, z15.h[0]
	fmlal	za.s[w8, 0:1], z0.h, z0.h[7]
	fmlal	za.s[w9, 10:11], z21.h, z9.h[2]

	fmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	fmlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	FMLAL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	FMLAL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	fmlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	fmlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	fmlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	fmlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	fmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	fmlal	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	fmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	fmlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	FMLAL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	FMLAL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	fmlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	fmlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	fmlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	fmlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	fmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	fmlal	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	fmlal	za.s[w8, 0:1], z0.h, z0.h
	FMLAL	ZA.s[W8, 0:1], Z0.h, Z0.h
	FMLAL	ZA.S[W8, 0:1], Z0.H, Z0.H
	fmlal	za.s[w11, 0:1], z0.h, z0.h
	fmlal	za.s[w8, 14:15], z0.h, z0.h
	fmlal	za.s[w8, 0:1], z31.h, z0.h
	fmlal	za.s[w8, 0:1], z0.h, z15.h
	fmlal	za.s[w10, 2:3], z25.h, z7.h

	fmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	fmlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	FMLAL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h
	FMLAL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H
	fmlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	fmlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	fmlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	fmlal	za.s[w8, 0:1], { z31.h, z0.h }, z0.h
	fmlal	za.s[w8, 0:1], { z31.h - z0.h }, z0.h
	fmlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	fmlal	za.s[w9, 4:5], { z19.h - z20.h }, z13.h

	fmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	fmlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	FMLAL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h
	FMLAL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H
	fmlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	fmlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	fmlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	fmlal	za.s[w8, 0:1], { z29.h - z0.h }, z0.h
	fmlal	za.s[w8, 0:1], { z30.h, z31.h, z0.h, z1.h }, z0.h
	fmlal	za.s[w8, 0:1], { z30.h - z1.h }, z0.h
	fmlal	za.s[w8, 0:1], { z31.h - z2.h }, z0.h
	fmlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	fmlal	za.s[w9, 4:5], { z25.h - z28.h }, z14.h

	fmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	fmlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	FMLAL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	FMLAL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	fmlal	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	fmlal	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	fmlal	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	fmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	fmlal	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	fmlal	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	fmlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	FMLAL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	FMLAL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	fmlal	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	fmlal	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	fmlal	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	fmlal	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	fmlal	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }

	fmlsl	za.s[w8, 0:1], z0.h, z0.h[0]
	FMLSL	ZA.s[W8, 0:1], Z0.h, Z0.h[0]
	FMLSL	ZA.S[W8, 0:1], Z0.H, Z0.H[0]
	fmlsl	za.s[w11, 0:1], z0.h, z0.h[0]
	fmlsl	za.s[w8, 14:15], z0.h, z0.h[0]
	fmlsl	za.s[w8, 0:1], z31.h, z0.h[0]
	fmlsl	za.s[w8, 0:1], z0.h, z15.h[0]
	fmlsl	za.s[w8, 0:1], z0.h, z0.h[7]
	fmlsl	za.s[w9, 10:11], z21.h, z9.h[2]

	fmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	fmlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	FMLSL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h[0]
	FMLSL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H[0]
	fmlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	fmlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	fmlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	fmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	fmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	fmlsl	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	fmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	fmlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	FMLSL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h[0]
	FMLSL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H[0]
	fmlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	fmlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	fmlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	fmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	fmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	fmlsl	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	fmlsl	za.s[w8, 0:1], z0.h, z0.h
	FMLSL	ZA.s[W8, 0:1], Z0.h, Z0.h
	FMLSL	ZA.S[W8, 0:1], Z0.H, Z0.H
	fmlsl	za.s[w11, 0:1], z0.h, z0.h
	fmlsl	za.s[w8, 14:15], z0.h, z0.h
	fmlsl	za.s[w8, 0:1], z31.h, z0.h
	fmlsl	za.s[w8, 0:1], z0.h, z15.h
	fmlsl	za.s[w10, 2:3], z25.h, z7.h

	fmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	fmlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	FMLSL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, Z0.h
	FMLSL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, Z0.H
	fmlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	fmlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z31.h, z0.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z31.h - z0.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	fmlsl	za.s[w9, 4:5], { z19.h - z20.h }, z13.h

	fmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	fmlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	FMLSL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, Z0.h
	FMLSL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, Z0.H
	fmlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	fmlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z29.h - z0.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z30.h, z31.h, z0.h, z1.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z30.h - z1.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z31.h - z2.h }, z0.h
	fmlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	fmlsl	za.s[w9, 4:5], { z25.h - z28.h }, z14.h

	fmlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	fmlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	FMLSL	ZA.s[W8, 0:1, VGx2], { Z0.h - Z1.h }, { Z0.h - Z1.h }
	FMLSL	ZA.S[W8, 0:1, VGX2], { Z0.H - Z1.H }, { Z0.H - Z1.H }
	fmlsl	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	fmlsl	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	fmlsl	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	fmlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	fmlsl	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	fmlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	fmlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	FMLSL	ZA.s[W8, 0:1, VGx4], { Z0.h - Z3.h }, { Z0.h - Z3.h }
	FMLSL	ZA.S[W8, 0:1, VGX4], { Z0.H - Z3.H }, { Z0.H - Z3.H }
	fmlsl	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	fmlsl	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	fmlsl	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	fmlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	fmlsl	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }

	smlal	za.s[w8, 0:1], z0.h, z0.h[0]
	smlal	za.s[w11, 0:1], z0.h, z0.h[0]
	smlal	za.s[w8, 14:15], z0.h, z0.h[0]
	smlal	za.s[w8, 0:1], z31.h, z0.h[0]
	smlal	za.s[w8, 0:1], z0.h, z15.h[0]
	smlal	za.s[w8, 0:1], z0.h, z0.h[7]
	smlal	za.s[w9, 10:11], z21.h, z9.h[2]

	smlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	smlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	smlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	smlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	smlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	smlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	smlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	smlal	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	smlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	smlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	smlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	smlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	smlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	smlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	smlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	smlal	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	smlal	za.s[w8, 0:1], z0.h, z0.h
	smlal	za.s[w11, 0:1], z0.h, z0.h
	smlal	za.s[w8, 14:15], z0.h, z0.h
	smlal	za.s[w8, 0:1], z31.h, z0.h
	smlal	za.s[w8, 0:1], z0.h, z15.h
	smlal	za.s[w10, 2:3], z25.h, z7.h

	smlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	smlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	smlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	smlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	smlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	smlal	za.s[w8, 0:1], { z31.h, z0.h }, z0.h
	smlal	za.s[w8, 0:1], { z31.h - z0.h }, z0.h
	smlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	smlal	za.s[w9, 4:5], { z19.h - z20.h }, z13.h

	smlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	smlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	smlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	smlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	smlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	smlal	za.s[w8, 0:1], { z29.h - z0.h }, z0.h
	smlal	za.s[w8, 0:1], { z30.h, z31.h, z0.h, z1.h }, z0.h
	smlal	za.s[w8, 0:1], { z30.h - z1.h }, z0.h
	smlal	za.s[w8, 0:1], { z31.h, z0.h, z1.h, z2.h }, z0.h
	smlal	za.s[w8, 0:1], { z31.h - z2.h }, z0.h
	smlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	smlal	za.s[w9, 4:5], { z25.h - z28.h }, z14.h

	smlal	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	smlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	smlal	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	smlal	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	smlal	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	smlal	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	smlal	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	smlal	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	smlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	smlal	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	smlal	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	smlal	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	smlal	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	smlal	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }

	smlsl	za.s[w8, 0:1], z0.h, z0.h[0]
	smlsl	za.s[w11, 0:1], z0.h, z0.h[0]
	smlsl	za.s[w8, 14:15], z0.h, z0.h[0]
	smlsl	za.s[w8, 0:1], z31.h, z0.h[0]
	smlsl	za.s[w8, 0:1], z0.h, z15.h[0]
	smlsl	za.s[w8, 0:1], z0.h, z0.h[7]
	smlsl	za.s[w9, 10:11], z21.h, z9.h[2]

	smlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	smlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	smlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	smlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	smlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	smlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	smlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	smlsl	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	smlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	smlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	smlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	smlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	smlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	smlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	smlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	smlsl	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	smlsl	za.s[w8, 0:1], z0.h, z0.h
	smlsl	za.s[w11, 0:1], z0.h, z0.h
	smlsl	za.s[w8, 14:15], z0.h, z0.h
	smlsl	za.s[w8, 0:1], z31.h, z0.h
	smlsl	za.s[w8, 0:1], z0.h, z15.h
	smlsl	za.s[w10, 2:3], z25.h, z7.h

	smlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	smlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	smlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	smlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	smlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	smlsl	za.s[w8, 0:1], { z31.h, z0.h }, z0.h
	smlsl	za.s[w8, 0:1], { z31.h - z0.h }, z0.h
	smlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	smlsl	za.s[w9, 4:5], { z19.h - z20.h }, z13.h

	smlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	smlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	smlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	smlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	smlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	smlsl	za.s[w8, 0:1], { z29.h - z0.h }, z0.h
	smlsl	za.s[w8, 0:1], { z30.h, z31.h, z0.h, z1.h }, z0.h
	smlsl	za.s[w8, 0:1], { z30.h - z1.h }, z0.h
	smlsl	za.s[w8, 0:1], { z31.h, z0.h, z1.h, z2.h }, z0.h
	smlsl	za.s[w8, 0:1], { z31.h - z2.h }, z0.h
	smlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	smlsl	za.s[w9, 4:5], { z25.h - z28.h }, z14.h

	smlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	smlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	smlsl	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	smlsl	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	smlsl	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	smlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	smlsl	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	smlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	smlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	smlsl	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	smlsl	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	smlsl	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	smlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	smlsl	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }

	umlal	za.s[w8, 0:1], z0.h, z0.h[0]
	umlal	za.s[w11, 0:1], z0.h, z0.h[0]
	umlal	za.s[w8, 14:15], z0.h, z0.h[0]
	umlal	za.s[w8, 0:1], z31.h, z0.h[0]
	umlal	za.s[w8, 0:1], z0.h, z15.h[0]
	umlal	za.s[w8, 0:1], z0.h, z0.h[7]
	umlal	za.s[w9, 10:11], z21.h, z9.h[2]

	umlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	umlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	umlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	umlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	umlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	umlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	umlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	umlal	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	umlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	umlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	umlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	umlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	umlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	umlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	umlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	umlal	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	umlal	za.s[w8, 0:1], z0.h, z0.h
	umlal	za.s[w11, 0:1], z0.h, z0.h
	umlal	za.s[w8, 14:15], z0.h, z0.h
	umlal	za.s[w8, 0:1], z31.h, z0.h
	umlal	za.s[w8, 0:1], z0.h, z15.h
	umlal	za.s[w10, 2:3], z25.h, z7.h

	umlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	umlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	umlal	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	umlal	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	umlal	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	umlal	za.s[w8, 0:1], { z31.h, z0.h }, z0.h
	umlal	za.s[w8, 0:1], { z31.h - z0.h }, z0.h
	umlal	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	umlal	za.s[w9, 4:5], { z19.h - z20.h }, z13.h

	umlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	umlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	umlal	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	umlal	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	umlal	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	umlal	za.s[w8, 0:1], { z29.h - z0.h }, z0.h
	umlal	za.s[w8, 0:1], { z30.h, z31.h, z0.h, z1.h }, z0.h
	umlal	za.s[w8, 0:1], { z30.h - z1.h }, z0.h
	umlal	za.s[w8, 0:1], { z31.h, z0.h, z1.h, z2.h }, z0.h
	umlal	za.s[w8, 0:1], { z31.h - z2.h }, z0.h
	umlal	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	umlal	za.s[w9, 4:5], { z25.h - z28.h }, z14.h

	umlal	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	umlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	umlal	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	umlal	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	umlal	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	umlal	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	umlal	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	umlal	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	umlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	umlal	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	umlal	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	umlal	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	umlal	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	umlal	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }

	umlsl	za.s[w8, 0:1], z0.h, z0.h[0]
	umlsl	za.s[w11, 0:1], z0.h, z0.h[0]
	umlsl	za.s[w8, 14:15], z0.h, z0.h[0]
	umlsl	za.s[w8, 0:1], z31.h, z0.h[0]
	umlsl	za.s[w8, 0:1], z0.h, z15.h[0]
	umlsl	za.s[w8, 0:1], z0.h, z0.h[7]
	umlsl	za.s[w9, 10:11], z21.h, z9.h[2]

	umlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	umlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h[0]
	umlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h[0]
	umlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h[0]
	umlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h[0]
	umlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h[0]
	umlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[7]
	umlsl	za.s[w9, 4:5], { z18.h - z19.h }, z9.h[3]

	umlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	umlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h[0]
	umlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h[0]
	umlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h[0]
	umlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h[0]
	umlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h[0]
	umlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[7]
	umlsl	za.s[w9, 4:5], { z24.h - z27.h }, z14.h[5]

	umlsl	za.s[w8, 0:1], z0.h, z0.h
	umlsl	za.s[w11, 0:1], z0.h, z0.h
	umlsl	za.s[w8, 14:15], z0.h, z0.h
	umlsl	za.s[w8, 0:1], z31.h, z0.h
	umlsl	za.s[w8, 0:1], z0.h, z15.h
	umlsl	za.s[w10, 2:3], z25.h, z7.h

	umlsl	za.s[w8, 0:1], { z0.h - z1.h }, z0.h
	umlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, z0.h
	umlsl	za.s[w11, 0:1], { z0.h - z1.h }, z0.h
	umlsl	za.s[w8, 6:7], { z0.h - z1.h }, z0.h
	umlsl	za.s[w8, 0:1], { z30.h - z31.h }, z0.h
	umlsl	za.s[w8, 0:1], { z31.h, z0.h }, z0.h
	umlsl	za.s[w8, 0:1], { z31.h - z0.h }, z0.h
	umlsl	za.s[w8, 0:1], { z0.h - z1.h }, z15.h
	umlsl	za.s[w9, 4:5], { z19.h - z20.h }, z13.h

	umlsl	za.s[w8, 0:1], { z0.h - z3.h }, z0.h
	umlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, z0.h
	umlsl	za.s[w11, 0:1], { z0.h - z3.h }, z0.h
	umlsl	za.s[w8, 6:7], { z0.h - z3.h }, z0.h
	umlsl	za.s[w8, 0:1], { z28.h - z31.h }, z0.h
	umlsl	za.s[w8, 0:1], { z29.h - z0.h }, z0.h
	umlsl	za.s[w8, 0:1], { z30.h, z31.h, z0.h, z1.h }, z0.h
	umlsl	za.s[w8, 0:1], { z30.h - z1.h }, z0.h
	umlsl	za.s[w8, 0:1], { z31.h, z0.h, z1.h, z2.h }, z0.h
	umlsl	za.s[w8, 0:1], { z31.h - z2.h }, z0.h
	umlsl	za.s[w8, 0:1], { z0.h - z3.h }, z15.h
	umlsl	za.s[w9, 4:5], { z25.h - z28.h }, z14.h

	umlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	umlsl	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	umlsl	za.s[w11, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	umlsl	za.s[w8, 6:7], { z0.h - z1.h }, { z0.h - z1.h }
	umlsl	za.s[w8, 0:1], { z30.h - z31.h }, { z0.h - z1.h }
	umlsl	za.s[w8, 0:1], { z0.h - z1.h }, { z30.h - z31.h }
	umlsl	za.s[w10, 2:3], { z22.h - z23.h }, { z18.h - z19.h }

	umlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	umlsl	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	umlsl	za.s[w11, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	umlsl	za.s[w8, 6:7], { z0.h - z3.h }, { z0.h - z3.h }
	umlsl	za.s[w8, 0:1], { z28.h - z31.h }, { z0.h - z3.h }
	umlsl	za.s[w8, 0:1], { z0.h - z3.h }, { z28.h - z31.h }
	umlsl	za.s[w11, 4:5], { z16.h - z19.h }, { z24.h - z27.h }
