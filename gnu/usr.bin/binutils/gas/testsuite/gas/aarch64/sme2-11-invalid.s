	fmla	0, { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, 0], 0, z0.s[0]
	fmla	za.s[w8, 0], { z0.s - z1.s }, 0

	fmla	za.s[w7, 0], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w12, 0], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, -1], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, 8], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, 0, vgx4], { z0.s - z1.s }, z0.s[0]
	fmla	za.s[w8, 0], { z0.s - z2.s }, z0.s[0]
	fmla	za.s[w8, 0], { z1.s - z2.s }, z0.s[0]
	fmla	za.s[w8, 0], { z0.s - z1.s }, z16.s[0]
	fmla	za.s[w8, 0], { z0.s - z1.s }, z0.s[-1]
	fmla	za.s[w8, 0], { z0.s - z1.s }, z0.s[4]

	fmla	za.s[w7, 0], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w12, 0], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w8, -1], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w8, 8], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w8, 0, vgx2], { z0.s - z3.s }, z0.s[0]
	fmla	za.s[w8, 0], { z0.s - z4.s }, z0.s[0]
	fmla	za.s[w8, 0], { z1.s - z4.s }, z0.s[0]
	fmla	za.s[w8, 0], { z2.s - z5.s }, z0.s[0]
	fmla	za.s[w8, 0], { z3.s - z6.s }, z0.s[0]
	fmla	za.s[w8, 0], { z0.s - z3.s }, z16.s[0]
	fmla	za.s[w8, 0], { z0.s - z3.s }, z0.s[-1]
	fmla	za.s[w8, 0], { z0.s - z3.s }, z0.s[4]

	fmla	za.s[w0, 0], { z0.s - z1.s }, z0.s
	fmla	za.s[w31, 0], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 1<<63], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0], { z0.s - z1.s }, z31.s
	fmla	za.s[w8, 0:0], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0:-1], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0:1], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0:100], { z0.s - z1.s }, z0.s

	fmla	za.s[w7, 0], { z0.s - z1.s }, z0.s
	fmla	za.s[w12, 0], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, -1], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 8], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0], { z0.s - z1.s }, z16.s

	fmla	za.s[w7, 0], { z0.s - z3.s }, z0.s
	fmla	za.s[w12, 0], { z0.s - z3.s }, z0.s
	fmla	za.s[w8, -1], { z0.s - z3.s }, z0.s
	fmla	za.s[w8, 8], { z0.s - z3.s }, z0.s
	fmla	za.s[w8, 0], { z0.s - z3.s }, z16.s

	fmla	za.s[w8, 0], { z0.s - z2.s }, z0.s
	fmla	za.s[w8, 0], { z0.s - z4.s }, z0.s
	fmla	za.s[w8, 0], { z0.s, z1.s, z2.s }, z0.s
	fmla	za.s[w8, 0], { z0.s, z1.s, z5.s }, z0.s

	fmla	za.s[w8, 0, vgx4], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0, vgx2], { z0.s - z3.s }, z0.s
	fmla	za[w8, 0], { z0.s - z1.s }, z0.s
	fmla	za.s[w8, 0], { z0 - z1 }, z0.s
	fmla	za.s[w8, 0], { z0.s - z1.s }, z0
	fmla	za[w8, 0], { z0.s - z1.s }, z0

	fmla	za.s[w7, 0], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za.s[w12, 0], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za.s[w8, -1], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za.s[w8, 8], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0], { z1.s - z2.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0], { z0.s - z1.s }, { z15.s - z16.s }
	fmla	za.s[w8, 0], { z0.s - z1.s }, { z31.s, z0.s }

	fmla	za.s[w7, 0], { z0.s - z3.s }, { z0.s - z3.s }
	fmla	za.s[w12, 0], { z0.s - z3.s }, { z0.s - z3.s }
	fmla	za.s[w8, -1], { z0.s - z3.s }, { z0.s - z3.s }
	fmla	za.s[w8, 8], { z0.s - z3.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0], { z1.s - z4.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0], { z2.s - z5.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0], { z3.s - z6.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0], { z0.s - z3.s }, { z15.s - z18.s }
	fmla	za.s[w8, 0], { z0.s - z3.s }, { z29.s, z30.s, z31.s, z0.s }

	fmla	za.s[w8, 0], { z0.s - z2.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0], { z0.s - z3.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z2.s }
	fmla	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z4.s }

	fmla	za.s[w8, 0, vgx4], { z0.s - z1.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0, vgx4], { z0.s - z3.s }, { z0.s - z1.s }
	fmla	za.s[w8, 0, vgx2], { z0.s - z1.s }, { z0.s - z3.s }
	fmla	za.s[w8, 0, vgx2], { z0.s - z3.s }, { z0.s - z1.s }
	fmla	za[w8, 0], { z0.s - z1.s }, { z0.s - z1.s }
	fmla	za[w8, 0], { z0.s - z3.s }, { z0.s - z3.s }
