	fmla	za.d[w8, 0], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w8, 0, vgx2], { z0.d - z1.d }, z0.d[0]
	FMLA	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }, Z0.d[0]
	fmla	za.d[w11, 0], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w8, 7], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w8, 0], { z30.d - z31.d }, z0.d[0]
	fmla	za.d[w8, 0], { z0.d - z1.d }, z15.d[0]
	fmla	za.d[w8, 0], { z0.d - z1.d }, z0.d[1]
	fmla	za.d[w10, 2], { z6.d - z7.d }, z5.d[1]

	fmla	za.d[w8, 0], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w8, 0, vgx4], { z0.d - z3.d }, z0.d[0]
	FMLA	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }, Z0.D[0]
	fmla	za.d[w11, 0], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w8, 7], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w8, 0], { z28.d - z31.d }, z0.d[0]
	fmla	za.d[w8, 0], { z0.d - z3.d }, z15.d[0]
	fmla	za.d[w8, 0], { z0.d - z3.d }, z0.d[1]
	fmla	za.d[w9, 3], { z8.d - z11.d }, z14.d[1]

	fmla	za.d[w8, 0], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0, vgx2], { z0.d - z1.d }, z0.d
	FMLA	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }, Z0.d
	FMLA	ZA.D[W8, 0, VGX2], { Z0.D - Z1.D }, Z0.D
	fmla	za.d[w11, 0], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 7], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0], { z30.d - z31.d }, z0.d
	fmla	za.d[w8, 0], { z31.d, z0.d }, z0.d
	fmla	za.d[w8, 0], { z31.d - z0.d }, z0.d
	fmla	za.d[w8, 0], { z0.d - z1.d }, z15.d
	fmla	za.d[w9, 5], { z9.d - z10.d }, z6.d

	fmla	za.d[w8, 0], { z0.d - z3.d }, z0.d
	fmla	za.d[w8, 0, vgx4], { z0.d - z3.d }, z0.d
	FMLA	ZA.d[W8, 0, VGx4], { Z0.d - Z3.d }, Z0.d
	FMLA	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }, Z0.D
	fmla	za.d[w11, 0], { z0.d - z3.d }, z0.d
	fmla	za.d[w8, 7], { z0.d - z3.d }, z0.d
	fmla	za.d[w8, 0], { z28.d - z31.d }, z0.d
	fmla	za.d[w8, 0], { z31.d, z0.d, z1.d, z2.d }, z0.d
	fmla	za.d[w8, 0], { z31.d - z2.d }, z0.d
	fmla	za.d[w8, 0], { z0.d - z3.d }, z15.d
	fmla	za.d[w11, 2], { z23.d - z26.d }, z13.d

	fmla	za.d[w8, 0], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0, vgx2], { z0.d - z1.d }, { z0.d - z1.d }
	FMLA	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }, { Z0.d - Z1.d }
	FMLA	ZA.D[W8, 0, VGX2], { Z0.D - Z1.D }, { Z0.D - Z1.D }
	fmla	za.d[w11, 0], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za.d[w8, 7], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0], { z30.d - z31.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0], { z0.d - z1.d }, { z30.d - z31.d }
	fmla	za.d[w10, 1], { z22.d - z23.d }, { z18.d - z19.d }

	fmla	za.d[w8, 0], { z0.d - z3.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0, vgx4], { z0.d - z3.d }, { z0.d - z3.d }
	FMLA	ZA.d[W8, 0, VGx4], { Z0.d - Z3.d }, { Z0.d - Z3.d }
	FMLA	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }, { Z0.D - Z3.D }
	fmla	za.d[w11, 0], { z0.d - z3.d }, { z0.d - z3.d }
	fmla	za.d[w8, 7], { z0.d - z3.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0], { z28.d - z31.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0], { z0.d - z3.d }, { z28.d - z31.d }
	fmla	za.d[w11, 3], { z16.d - z19.d }, { z24.d - z27.d }

	fmls	za.d[w8, 0], { z0.d - z1.d }, z0.d[0]
	fmls	za.d[w8, 0, vgx2], { z0.d - z1.d }, z0.d[0]
	FMLS	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }, Z0.d[0]
	fmls	za.d[w11, 0], { z0.d - z1.d }, z0.d[0]
	fmls	za.d[w8, 7], { z0.d - z1.d }, z0.d[0]
	fmls	za.d[w8, 0], { z30.d - z31.d }, z0.d[0]
	fmls	za.d[w8, 0], { z0.d - z1.d }, z15.d[0]
	fmls	za.d[w8, 0], { z0.d - z1.d }, z0.d[1]
	fmls	za.d[w10, 2], { z6.d - z7.d }, z5.d[1]

	fmls	za.d[w8, 0], { z0.d - z3.d }, z0.d[0]
	fmls	za.d[w8, 0, vgx4], { z0.d - z3.d }, z0.d[0]
	FMLS	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }, Z0.D[0]
	fmls	za.d[w11, 0], { z0.d - z3.d }, z0.d[0]
	fmls	za.d[w8, 7], { z0.d - z3.d }, z0.d[0]
	fmls	za.d[w8, 0], { z28.d - z31.d }, z0.d[0]
	fmls	za.d[w8, 0], { z0.d - z3.d }, z15.d[0]
	fmls	za.d[w8, 0], { z0.d - z3.d }, z0.d[1]
	fmls	za.d[w9, 3], { z8.d - z11.d }, z14.d[1]

	fmls	za.d[w8, 0], { z0.d - z1.d }, z0.d
	fmls	za.d[w8, 0, vgx2], { z0.d - z1.d }, z0.d
	FMLS	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }, Z0.d
	FMLS	ZA.D[W8, 0, VGX2], { Z0.D - Z1.D }, Z0.D
	fmls	za.d[w11, 0], { z0.d - z1.d }, z0.d
	fmls	za.d[w8, 7], { z0.d - z1.d }, z0.d
	fmls	za.d[w8, 0], { z30.d - z31.d }, z0.d
	fmls	za.d[w8, 0], { z31.d, z0.d }, z0.d
	fmls	za.d[w8, 0], { z31.d - z0.d }, z0.d
	fmls	za.d[w8, 0], { z0.d - z1.d }, z15.d
	fmls	za.d[w9, 5], { z9.d - z10.d }, z6.d

	fmls	za.d[w8, 0], { z0.d - z3.d }, z0.d
	fmls	za.d[w8, 0, vgx4], { z0.d - z3.d }, z0.d
	FMLS	ZA.d[W8, 0, VGx4], { Z0.d - Z3.d }, Z0.d
	FMLS	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }, Z0.D
	fmls	za.d[w11, 0], { z0.d - z3.d }, z0.d
	fmls	za.d[w8, 7], { z0.d - z3.d }, z0.d
	fmls	za.d[w8, 0], { z28.d - z31.d }, z0.d
	fmls	za.d[w8, 0], { z31.d, z0.d, z1.d, z2.d }, z0.d
	fmls	za.d[w8, 0], { z31.d - z2.d }, z0.d
	fmls	za.d[w8, 0], { z0.d - z3.d }, z15.d
	fmls	za.d[w11, 2], { z23.d - z26.d }, z13.d

	fmls	za.d[w8, 0], { z0.d - z1.d }, { z0.d - z1.d }
	fmls	za.d[w8, 0, vgx2], { z0.d - z1.d }, { z0.d - z1.d }
	FMLS	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }, { Z0.d - Z1.d }
	FMLS	ZA.D[W8, 0, VGX2], { Z0.D - Z1.D }, { Z0.D - Z1.D }
	fmls	za.d[w11, 0], { z0.d - z1.d }, { z0.d - z1.d }
	fmls	za.d[w8, 7], { z0.d - z1.d }, { z0.d - z1.d }
	fmls	za.d[w8, 0], { z30.d - z31.d }, { z0.d - z1.d }
	fmls	za.d[w8, 0], { z0.d - z1.d }, { z30.d - z31.d }
	fmls	za.d[w10, 1], { z22.d - z23.d }, { z18.d - z19.d }

	fmls	za.d[w8, 0], { z0.d - z3.d }, { z0.d - z3.d }
	fmls	za.d[w8, 0, vgx4], { z0.d - z3.d }, { z0.d - z3.d }
	FMLS	ZA.d[W8, 0, VGx4], { Z0.d - Z3.d }, { Z0.d - Z3.d }
	FMLS	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }, { Z0.D - Z3.D }
	fmls	za.d[w11, 0], { z0.d - z3.d }, { z0.d - z3.d }
	fmls	za.d[w8, 7], { z0.d - z3.d }, { z0.d - z3.d }
	fmls	za.d[w8, 0], { z28.d - z31.d }, { z0.d - z3.d }
	fmls	za.d[w8, 0], { z0.d - z3.d }, { z28.d - z31.d }
	fmls	za.d[w11, 3], { z16.d - z19.d }, { z24.d - z27.d }
