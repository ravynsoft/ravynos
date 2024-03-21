	fmla	za.d[w7, 0], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w12, 0], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w8, -1], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w8, 8], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w8, 0, vgx4], { z0.d - z1.d }, z0.d[0]
	fmla	za.d[w8, 0], { z0.d - z2.d }, z0.d[0]
	fmla	za.d[w8, 0], { z1.d - z2.d }, z0.d[0]
	fmla	za.d[w8, 0], { z0.d - z1.d }, z16.d[0]
	fmla	za.d[w8, 0], { z0.d - z1.d }, z0.d[-1]
	fmla	za.d[w8, 0], { z0.d - z1.d }, z0.d[2]

	fmla	za.d[w7, 0], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w12, 0], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w8, -1], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w8, 8], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w8, 0, vgx2], { z0.d - z3.d }, z0.d[0]
	fmla	za.d[w8, 0], { z0.d - z4.d }, z0.d[0]
	fmla	za.d[w8, 0], { z1.d - z4.d }, z0.d[0]
	fmla	za.d[w8, 0], { z2.d - z5.d }, z0.d[0]
	fmla	za.d[w8, 0], { z3.d - z6.d }, z0.d[0]
	fmla	za.d[w8, 0], { z0.d - z3.d }, z16.d[0]
	fmla	za.d[w8, 0], { z0.d - z3.d }, z0.d[-1]
	fmla	za.d[w8, 0], { z0.d - z3.d }, z0.d[2]

	fmla	za.d[w0, 0], { z0.d - z1.d }, z0.d
	fmla	za.d[w31, 0], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 1<<63], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0], { z0.d - z1.d }, z31.d
	fmla	za.d[w8, 0:0], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0:-1], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0:1], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0:100], { z0.d - z1.d }, z0.d

	fmla	za.d[w7, 0], { z0.d - z1.d }, z0.d
	fmla	za.d[w12, 0], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, -1], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 8], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0], { z0.d - z1.d }, z16.d

	fmla	za.d[w7, 0], { z0.d - z3.d }, z0.d
	fmla	za.d[w12, 0], { z0.d - z3.d }, z0.d
	fmla	za.d[w8, -1], { z0.d - z3.d }, z0.d
	fmla	za.d[w8, 8], { z0.d - z3.d }, z0.d
	fmla	za.d[w8, 0], { z0.d - z3.d }, z16.d

	fmla	za.d[w8, 0], { z0.d - z2.d }, z0.d
	fmla	za.d[w8, 0], { z0.d - z4.d }, z0.d
	fmla	za.d[w8, 0], { z0.d, z1.d, z2.d }, z0.d
	fmla	za.d[w8, 0], { z0.d, z1.d, z5.d }, z0.d

	fmla	za.d[w8, 0, vgx4], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0, vgx2], { z0.d - z3.d }, z0.d
	fmla	za[w8, 0], { z0.d - z1.d }, z0.d
	fmla	za.d[w8, 0], { z0 - z1 }, z0.d
	fmla	za.d[w8, 0], { z0.d - z1.d }, z0
	fmla	za[w8, 0], { z0.d - z1.d }, z0

	fmla	za.d[w7, 0], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za.d[w12, 0], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za.d[w8, -1], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za.d[w8, 8], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0], { z1.d - z2.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0], { z0.d - z1.d }, { z15.d - z16.d }
	fmla	za.d[w8, 0], { z0.d - z1.d }, { z31.d, z0.d }

	fmla	za.d[w7, 0], { z0.d - z3.d }, { z0.d - z3.d }
	fmla	za.d[w12, 0], { z0.d - z3.d }, { z0.d - z3.d }
	fmla	za.d[w8, -1], { z0.d - z3.d }, { z0.d - z3.d }
	fmla	za.d[w8, 8], { z0.d - z3.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0], { z1.d - z4.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0], { z2.d - z5.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0], { z3.d - z6.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0], { z0.d - z3.d }, { z15.d - z18.d }
	fmla	za.d[w8, 0], { z0.d - z3.d }, { z29.d, z30.d, z31.d, z0.d }

	fmla	za.d[w8, 0], { z0.d - z2.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0], { z0.d - z3.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0], { z0.d - z1.d }, { z0.d - z2.d }
	fmla	za.d[w8, 0], { z0.d - z1.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0], { z0.d - z1.d }, { z0.d - z4.d }

	fmla	za.d[w8, 0, vgx4], { z0.d - z1.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0, vgx4], { z0.d - z3.d }, { z0.d - z1.d }
	fmla	za.d[w8, 0, vgx2], { z0.d - z1.d }, { z0.d - z3.d }
	fmla	za.d[w8, 0, vgx2], { z0.d - z3.d }, { z0.d - z1.d }
	fmla	za[w8, 0], { z0.d - z1.d }, { z0.d - z1.d }
	fmla	za[w8, 0], { z0.d - z3.d }, { z0.d - z3.d }
