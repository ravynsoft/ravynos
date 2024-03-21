	add	za.s[w8, 0], { z0.s - z1.s }
	add	za.s[w8, 0, vgx2], { z0.s - z1.s }
	ADD	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }
	ADD	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }
	add	za.s[w11, 0], { z0.s - z1.s }
	add	za.s[w8, 7], { z0.s - z1.s }
	add	za.s[w8, 0], { z30.s - z31.s }
	add	za.s[w10, 3], { z10.s - z11.s }

	add	za.s[w8, 0], { z0.s - z3.s }
	add	za.s[w8, 0, vgx4], { z0.s - z3.s }
	ADD	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }
	ADD	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }
	add	za.s[w11, 0], { z0.s - z3.s }
	add	za.s[w8, 7], { z0.s - z3.s }
	add	za.s[w8, 0], { z28.s - z31.s }
	add	za.s[w11, 1], { z12.s - z15.s }

	add	za.s[w8, 0], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0, vgx2], { z0.s - z1.s }, z0.s
	ADD	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, Z0.s
	ADD	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, Z0.S
	add	za.s[w11, 0], { z0.s - z1.s }, z0.s
	add	za.s[w8, 7], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0], { z30.s - z31.s }, z0.s
	add	za.s[w8, 0], { z31.s, z0.s }, z0.s
	add	za.s[w8, 0], { z31.s - z0.s }, z0.s
	add	za.s[w8, 0], { z0.s - z1.s }, z15.s
	add	za.s[w9, 5], { z9.s - z10.s }, z6.s

	add	za.s[w8, 0], { z0.s - z3.s }, z0.s
	add	za.s[w8, 0, vgx4], { z0.s - z3.s }, z0.s
	ADD	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, Z0.s
	ADD	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, Z0.S
	add	za.s[w11, 0], { z0.s - z3.s }, z0.s
	add	za.s[w8, 7], { z0.s - z3.s }, z0.s
	add	za.s[w8, 0], { z28.s - z31.s }, z0.s
	add	za.s[w8, 0], { z31.s, z0.s, z1.s, z2.s }, z0.s
	add	za.s[w8, 0], { z31.s - z2.s }, z0.s
	add	za.s[w8, 0], { z0.s - z3.s }, z15.s
	add	za.s[w11, 2], { z23.s - z26.s }, z13.s

	add	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z1.s }
	add	za.s[w8, 0, vgx2], { z0.s - z1.s }, { z0.s - z1.s }
	ADD	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, { Z0.s - Z1.s }
	ADD	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, { Z0.S - Z1.S }
	add	za.s[w11, 0], { z0.s - z1.s }, { z0.s - z1.s }
	add	za.s[w8, 7], { z0.s - z1.s }, { z0.s - z1.s }
	add	za.s[w8, 0], { z30.s - z31.s }, { z0.s - z1.s }
	add	za.s[w8, 0], { z0.s - z1.s }, { z30.s - z31.s }
	add	za.s[w10, 1], { z22.s - z23.s }, { z18.s - z19.s }

	add	za.s[w8, 0], { z0.s - z3.s }, { z0.s - z3.s }
	add	za.s[w8, 0, vgx4], { z0.s - z3.s }, { z0.s - z3.s }
	ADD	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, { Z0.s - Z3.s }
	ADD	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, { Z0.S - Z3.S }
	add	za.s[w11, 0], { z0.s - z3.s }, { z0.s - z3.s }
	add	za.s[w8, 7], { z0.s - z3.s }, { z0.s - z3.s }
	add	za.s[w8, 0], { z28.s - z31.s }, { z0.s - z3.s }
	add	za.s[w8, 0], { z0.s - z3.s }, { z28.s - z31.s }
	add	za.s[w11, 3], { z16.s - z19.s }, { z24.s - z27.s }

	add	{ z0.b - z1.b }, { z0.b - z1.b }, z0.b
	add	{ z30.b - z31.b }, { z30.b - z31.b }, z0.b
	add	{ z0.b - z1.b }, { z0.b - z1.b }, z15.b
	add	{ z14.b - z15.b }, { z14.b - z15.b }, z5.b

	add	{ z0.h - z1.h }, { z0.h - z1.h }, z0.h
	add	{ z30.h - z31.h }, { z30.h - z31.h }, z0.h
	add	{ z0.h - z1.h }, { z0.h - z1.h }, z15.h
	add	{ z20.h - z21.h }, { z20.h - z21.h }, z11.h

	add	{ z0.s - z1.s }, { z0.s - z1.s }, z0.s
	add	{ z30.s - z31.s }, { z30.s - z31.s }, z0.s
	add	{ z0.s - z1.s }, { z0.s - z1.s }, z15.s
	add	{ z2.s - z3.s }, { z2.s - z3.s }, z9.s

	add	{ z0.d - z1.d }, { z0.d - z1.d }, z0.d
	add	{ z30.d - z31.d }, { z30.d - z31.d }, z0.d
	add	{ z0.d - z1.d }, { z0.d - z1.d }, z15.d
	add	{ z28.d - z29.d }, { z28.d - z29.d }, z1.d

	add	{ z0.b - z3.b }, { z0.b - z3.b }, z0.b
	add	{ z28.b - z31.b }, { z28.b - z31.b }, z0.b
	add	{ z0.b - z3.b }, { z0.b - z3.b }, z15.b
	add	{ z24.b - z27.b }, { z24.b - z27.b }, z5.b

	add	{ z0.h - z3.h }, { z0.h - z3.h }, z0.h
	add	{ z28.h - z31.h }, { z28.h - z31.h }, z0.h
	add	{ z0.h - z3.h }, { z0.h - z3.h }, z15.h
	add	{ z20.h - z23.h }, { z20.h - z23.h }, z11.h

	add	{ z0.s - z3.s }, { z0.s - z3.s }, z0.s
	add	{ z28.s - z31.s }, { z28.s - z31.s }, z0.s
	add	{ z0.s - z3.s }, { z0.s - z3.s }, z15.s
	add	{ z4.s - z7.s }, { z4.s - z7.s }, z9.s

	add	{ z0.d - z3.d }, { z0.d - z3.d }, z0.d
	add	{ z28.d - z31.d }, { z28.d - z31.d }, z0.d
	add	{ z0.d - z3.d }, { z0.d - z3.d }, z15.d
	add	{ z16.d - z19.d }, { z16.d - z19.d }, z3.d

	sub	za.s[w8, 0], { z0.s - z1.s }
	sub	za.s[w8, 0, vgx2], { z0.s - z1.s }
	SUB	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }
	SUB	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }
	sub	za.s[w11, 0], { z0.s - z1.s }
	sub	za.s[w8, 7], { z0.s - z1.s }
	sub	za.s[w8, 0], { z30.s - z31.s }
	sub	za.s[w10, 3], { z10.s - z11.s }

	sub	za.s[w8, 0], { z0.s - z3.s }
	sub	za.s[w8, 0, vgx4], { z0.s - z3.s }
	SUB	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }
	SUB	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }
	sub	za.s[w11, 0], { z0.s - z3.s }
	sub	za.s[w8, 7], { z0.s - z3.s }
	sub	za.s[w8, 0], { z28.s - z31.s }
	sub	za.s[w11, 1], { z12.s - z15.s }

	sub	za.s[w8, 0], { z0.s - z1.s }, z0.s
	sub	za.s[w8, 0, vgx2], { z0.s - z1.s }, z0.s
	SUB	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, Z0.s
	SUB	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, Z0.S
	sub	za.s[w11, 0], { z0.s - z1.s }, z0.s
	sub	za.s[w8, 7], { z0.s - z1.s }, z0.s
	sub	za.s[w8, 0], { z30.s - z31.s }, z0.s
	sub	za.s[w8, 0], { z31.s, z0.s }, z0.s
	sub	za.s[w8, 0], { z31.s - z0.s }, z0.s
	sub	za.s[w8, 0], { z0.s - z1.s }, z15.s
	sub	za.s[w9, 5], { z9.s - z10.s }, z6.s

	sub	za.s[w8, 0], { z0.s - z3.s }, z0.s
	sub	za.s[w8, 0, vgx4], { z0.s - z3.s }, z0.s
	SUB	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, Z0.s
	SUB	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, Z0.S
	sub	za.s[w11, 0], { z0.s - z3.s }, z0.s
	sub	za.s[w8, 7], { z0.s - z3.s }, z0.s
	sub	za.s[w8, 0], { z28.s - z31.s }, z0.s
	sub	za.s[w8, 0], { z31.s, z0.s, z1.s, z2.s }, z0.s
	sub	za.s[w8, 0], { z31.s - z2.s }, z0.s
	sub	za.s[w8, 0], { z0.s - z3.s }, z15.s
	sub	za.s[w11, 2], { z23.s - z26.s }, z13.s

	sub	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z1.s }
	sub	za.s[w8, 0, vgx2], { z0.s - z1.s }, { z0.s - z1.s }
	SUB	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }, { Z0.s - Z1.s }
	SUB	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }, { Z0.S - Z1.S }
	sub	za.s[w11, 0], { z0.s - z1.s }, { z0.s - z1.s }
	sub	za.s[w8, 7], { z0.s - z1.s }, { z0.s - z1.s }
	sub	za.s[w8, 0], { z30.s - z31.s }, { z0.s - z1.s }
	sub	za.s[w8, 0], { z0.s - z1.s }, { z30.s - z31.s }
	sub	za.s[w10, 1], { z22.s - z23.s }, { z18.s - z19.s }

	sub	za.s[w8, 0], { z0.s - z3.s }, { z0.s - z3.s }
	sub	za.s[w8, 0, vgx4], { z0.s - z3.s }, { z0.s - z3.s }
	SUB	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }, { Z0.s - Z3.s }
	SUB	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }, { Z0.S - Z3.S }
	sub	za.s[w11, 0], { z0.s - z3.s }, { z0.s - z3.s }
	sub	za.s[w8, 7], { z0.s - z3.s }, { z0.s - z3.s }
	sub	za.s[w8, 0], { z28.s - z31.s }, { z0.s - z3.s }
	sub	za.s[w8, 0], { z0.s - z3.s }, { z28.s - z31.s }
	sub	za.s[w11, 3], { z16.s - z19.s }, { z24.s - z27.s }

	fadd	za.s[w8, 0], { z0.s - z1.s }
	fadd	za.s[w8, 0, vgx2], { z0.s - z1.s }
	FADD	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }
	FADD	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }
	fadd	za.s[w11, 0], { z0.s - z1.s }
	fadd	za.s[w8, 7], { z0.s - z1.s }
	fadd	za.s[w8, 0], { z30.s - z31.s }
	fadd	za.s[w10, 3], { z10.s - z11.s }

	fadd	za.s[w8, 0], { z0.s - z3.s }
	fadd	za.s[w8, 0, vgx4], { z0.s - z3.s }
	FADD	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }
	FADD	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }
	fadd	za.s[w11, 0], { z0.s - z3.s }
	fadd	za.s[w8, 7], { z0.s - z3.s }
	fadd	za.s[w8, 0], { z28.s - z31.s }
	fadd	za.s[w11, 1], { z12.s - z15.s }

	fsub	za.s[w8, 0], { z0.s - z1.s }
	fsub	za.s[w8, 0, vgx2], { z0.s - z1.s }
	FSUB	ZA.s[W8, 0, VGx2], { Z0.s - Z1.s }
	FSUB	ZA.S[W8, 0, VGX2], { Z0.S - Z1.S }
	fsub	za.s[w11, 0], { z0.s - z1.s }
	fsub	za.s[w8, 7], { z0.s - z1.s }
	fsub	za.s[w8, 0], { z30.s - z31.s }
	fsub	za.s[w10, 3], { z10.s - z11.s }

	fsub	za.s[w8, 0], { z0.s - z3.s }
	fsub	za.s[w8, 0, vgx4], { z0.s - z3.s }
	FSUB	ZA.s[W8, 0, VGx4], { Z0.s - Z3.s }
	FSUB	ZA.S[W8, 0, VGX4], { Z0.S - Z3.S }
	fsub	za.s[w11, 0], { z0.s - z3.s }
	fsub	za.s[w8, 7], { z0.s - z3.s }
	fsub	za.s[w8, 0], { z28.s - z31.s }
	fsub	za.s[w11, 1], { z12.s - z15.s }
