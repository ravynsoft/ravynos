	fmla	za.s[w8, 0], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, 0, vgx2], { z0.s - z1.s }, z0.s[0]
	FMLA	ZA.S[W8, 0, VGx2], { Z0.S - Z1.S }, Z0.S[0]
	fmla	za.s[w11, 0], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, 7], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, 0], { z30.s - z31.s }, z0.s[0]
	fmla	za.s[w8, 0], { z0.s - z1.s }, z15.s[0]
	fmla	za.s[w8, 0], { z0.s - z1.s }, z0.s[3]
	fmla	za.s[w9, 6], { z12.s - z13.s }, z1.s[2]

	fmla	za.s[w8, 0], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w8, 0, vgx4], { z0.s - z3.s }, z0.s[0]
	FMLA	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, Z0.S[0]
	fmla	za.s[w11, 0], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w8, 7], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w8, 0], { z28.s - z31.s }, z0.s[0]
	fmla	za.s[w8, 0], { z0.s - z3.s }, z15.s[0]
	fmla	za.s[w8, 0], { z0.s - z3.s }, z0.s[3]
	fmla	za.s[w10, 4], { z4.s - z7.s }, z9.s[1]

	fmla	za.s[w8, 0], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0, vgx2], { z0.s - z1.s }, z0.s
	FMLA	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, Z0.s
	FMLA	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, Z0.S
	fmla	za.s[w11, 0], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 7], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0], { z30.s - z31.s }, z0.s
	fmla	za.s[w8, 0], { z31.s, z0.s }, z0.s
	fmla	za.s[w8, 0], { z31.s - z0.s }, z0.s
	fmla	za.s[w8, 0], { z0.s - z1.s }, z15.s
	fmla	za.s[w9, 5], { z9.s - z10.s }, z6.s

	fmla	za.s[w8, 0], { z0.s - z3.s }, z0.s
	fmla	za.s[w8, 0, vgx4], { z0.s - z3.s }, z0.s
	FMLA	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, Z0.s
	FMLA	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, Z0.S
	fmla	za.s[w11, 0], { z0.s - z3.s }, z0.s
	fmla	za.s[w8, 7], { z0.s - z3.s }, z0.s
	fmla	za.s[w8, 0], { z28.s - z31.s }, z0.s
	fmla	za.s[w8, 0], { z31.s, z0.s, z1.s, z2.s }, z0.s
	fmla	za.s[w8, 0], { z31.s - z2.s }, z0.s
	fmla	za.s[w8, 0], { z0.s - z3.s }, z15.s
	fmla	za.s[w11, 2], { z23.s - z26.s }, z13.s

	fmla	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0, vgx2], { z0.s - z1.s }, { z0.s - z1.s }
	FMLA	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, { Z0.s - Z1.s }
	FMLA	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, { Z0.S - Z1.S }
	fmla	za.s[w11, 0], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za.s[w8, 7], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0], { z30.s - z31.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0], { z0.s - z1.s }, { z30.s - z31.s }
	fmla	za.s[w10, 1], { z22.s - z23.s }, { z18.s - z19.s }

	fmla	za.s[w8, 0], { z0.s - z3.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0, vgx4], { z0.s - z3.s }, { z0.s - z3.s }
	FMLA	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, { Z0.s - Z3.s }
	FMLA	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, { Z0.S - Z3.S }
	fmla	za.s[w11, 0], { z0.s - z3.s }, { z0.s - z3.s }
	fmla	za.s[w8, 7], { z0.s - z3.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0], { z28.s - z31.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0], { z0.s - z3.s }, { z28.s - z31.s }
	fmla	za.s[w11, 3], { z16.s - z19.s }, { z24.s - z27.s }

	fmls	za.s[w8, 0], { z0.s - z1.s }, z0.s[0]
	fmls	za.s[w8, 0, vgx2], { z0.s - z1.s }, z0.s[0]
	FMLS	ZA.S[W8, 0, VGx2], { Z0.S - Z1.S }, Z0.S[0]
	fmls	za.s[w11, 0], { z0.s - z1.s }, z0.s[0]
	fmls	za.s[w8, 7], { z0.s - z1.s }, z0.s[0]
	fmls	za.s[w8, 0], { z30.s - z31.s }, z0.s[0]
	fmls	za.s[w8, 0], { z0.s - z1.s }, z15.s[0]
	fmls	za.s[w8, 0], { z0.s - z1.s }, z0.s[3]
	fmls	za.s[w9, 6], { z12.s - z13.s }, z1.s[2]

	fmls	za.s[w8, 0], { z0.s - z3.s }, z0.s[0]
	fmls	za.s[w8, 0, vgx4], { z0.s - z3.s }, z0.s[0]
	FMLS	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, Z0.S[0]
	fmls	za.s[w11, 0], { z0.s - z3.s }, z0.s[0]
	fmls	za.s[w8, 7], { z0.s - z3.s }, z0.s[0]
	fmls	za.s[w8, 0], { z28.s - z31.s }, z0.s[0]
	fmls	za.s[w8, 0], { z0.s - z3.s }, z15.s[0]
	fmls	za.s[w8, 0], { z0.s - z3.s }, z0.s[3]
	fmls	za.s[w10, 4], { z4.s - z7.s }, z9.s[1]

	fmls	za.s[w8, 0], { z0.s - z1.s }, z0.s
	fmls	za.s[w8, 0, vgx2], { z0.s - z1.s }, z0.s
	FMLS	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, Z0.s
	FMLS	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, Z0.S
	fmls	za.s[w11, 0], { z0.s - z1.s }, z0.s
	fmls	za.s[w8, 7], { z0.s - z1.s }, z0.s
	fmls	za.s[w8, 0], { z30.s - z31.s }, z0.s
	fmls	za.s[w8, 0], { z31.s, z0.s }, z0.s
	fmls	za.s[w8, 0], { z31.s - z0.s }, z0.s
	fmls	za.s[w8, 0], { z0.s - z1.s }, z15.s
	fmls	za.s[w9, 5], { z9.s - z10.s }, z6.s

	fmls	za.s[w8, 0], { z0.s - z3.s }, z0.s
	fmls	za.s[w8, 0, vgx4], { z0.s - z3.s }, z0.s
	FMLS	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, Z0.s
	FMLS	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, Z0.S
	fmls	za.s[w11, 0], { z0.s - z3.s }, z0.s
	fmls	za.s[w8, 7], { z0.s - z3.s }, z0.s
	fmls	za.s[w8, 0], { z28.s - z31.s }, z0.s
	fmls	za.s[w8, 0], { z31.s, z0.s, z1.s, z2.s }, z0.s
	fmls	za.s[w8, 0], { z31.s - z2.s }, z0.s
	fmls	za.s[w8, 0], { z0.s - z3.s }, z15.s
	fmls	za.s[w11, 2], { z23.s - z26.s }, z13.s

	fmls	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z1.s }
	fmls	za.s[w8, 0, vgx2], { z0.s - z1.s }, { z0.s - z1.s }
	FMLS	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, { Z0.s - Z1.s }
	FMLS	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, { Z0.S - Z1.S }
	fmls	za.s[w11, 0], { z0.s - z1.s }, { z0.s - z1.s }
	fmls	za.s[w8, 7], { z0.s - z1.s }, { z0.s - z1.s }
	fmls	za.s[w8, 0], { z30.s - z31.s }, { z0.s - z1.s }
	fmls	za.s[w8, 0], { z0.s - z1.s }, { z30.s - z31.s }
	fmls	za.s[w10, 1], { z22.s - z23.s }, { z18.s - z19.s }

	fmls	za.s[w8, 0], { z0.s - z3.s }, { z0.s - z3.s }
	fmls	za.s[w8, 0, vgx4], { z0.s - z3.s }, { z0.s - z3.s }
	FMLS	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, { Z0.s - Z3.s }
	FMLS	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, { Z0.S - Z3.S }
	fmls	za.s[w11, 0], { z0.s - z3.s }, { z0.s - z3.s }
	fmls	za.s[w8, 7], { z0.s - z3.s }, { z0.s - z3.s }
	fmls	za.s[w8, 0], { z28.s - z31.s }, { z0.s - z3.s }
	fmls	za.s[w8, 0], { z0.s - z3.s }, { z28.s - z31.s }
	fmls	za.s[w11, 3], { z16.s - z19.s }, { z24.s - z27.s }
