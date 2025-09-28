	bfmlal	0, z0.h, z0.h[0]
	bfmlal	za.s[w8, 0:1], 0, z0.h[0]
	bfmlal	za.s[w8, 0:1], z0.h, 0

	bfmlal	za.s[w7, 0:1], z0.h, z0.h[0]
	bfmlal	za.s[w12, 0:1], z0.h, z0.h[0]
	bfmlal	za.s[w8, 0], z0.h, z0.h[0]
	bfmlal	za.s[w8, 0:0], z0.h, z0.h[0]
	bfmlal	za.s[w8, 0:2], z0.h, z0.h[0]
	bfmlal	za.s[w8, 1:2], z0.h, z0.h[0]
	bfmlal	za.s[w8, 1:0], z0.h, z0.h[0]
	bfmlal	za.s[w8, -2:-1], z0.h, z0.h[0]
	bfmlal	za.s[w8, 16:17], z0.h, z0.h[0]
	bfmlal	za.s[w8, 0:1, vgx2], z0.h, z0.h[0]
	bfmlal	za.s[w8, 0:1], z0.h, z16.h[0]
	bfmlal	za.s[w8, 0:1], z0.h, z0.h[-1]
	bfmlal	za.s[w8, 0:1], z0.h, z0.h[8]
	bfmlal	za.s[w8, 0:1], z0.s, z0.s[0]
	bfmlal	za.h[w8, 0:1], z0.h, z0.h[0]

	bfmlal	za.s[w7, 0:1], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w12, 0:1], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 0], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 0:0], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 0:2], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 1:2], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 1:0], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, -2:-1], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 8:9], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 16:17], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1, vgx4], { z0.h - z1.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z1.h - z2.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z1.h, z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z16.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[-1]
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[8]
	bfmlal	za.s[w8, 0:1], { z0.s - z1.s }, z0.s[0]
	bfmlal	za.h[w8, 0:1], { z0.h - z1.h }, z0.h[0]

	bfmlal	za.s[w7, 0:1], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w12, 0:1], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0:0], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0:2], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 1:2], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 1:0], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, -2:-1], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 8:9], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 16:17], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1, vgx2], { z0.h - z3.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z1.h - z4.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z1.h, z3.h, z5.h, z7.h }, z0.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z16.h[0]
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[-1]
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z0.h[8]
	bfmlal	za.s[w8, 0:1], { z0.s - z3.s }, z0.s[0]
	bfmlal	za.h[w8, 0:1], { z0.h - z3.h }, z0.h[0]

	bfmlal	za.s[w7, 0:1], z0.h, z0.h
	bfmlal	za.s[w12, 0:1], z0.h, z0.h
	bfmlal	za.s[w8, 0], z0.h, z0.h
	bfmlal	za.s[w8, 0:0], z0.h, z0.h
	bfmlal	za.s[w8, 0:2], z0.h, z0.h
	bfmlal	za.s[w8, 1:2], z0.h, z0.h
	bfmlal	za.s[w8, 1:0], z0.h, z0.h
	bfmlal	za.s[w8, -2:-1], z0.h, z0.h
	bfmlal	za.s[w8, 16:17], z0.h, z0.h
	bfmlal	za.s[w8, 0:1, vgx2], z0.h, z0.h
	bfmlal	za.s[w8, 0:1], z0.h, z16.h
	bfmlal	za.s[w8, 0:1], z0.s, z0.s
	bfmlal	za.h[w8, 0:1], z0.h, z0.h

	bfmlal	za.s[w7, 0:1], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w12, 0:1], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0:0], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0:2], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 1:2], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 1:0], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, -2:-1], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 8:9], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 16:17], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0:1, vgx4], { z0.h - z1.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z1.h, z3.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, z16.h
	bfmlal	za.s[w8, 0:1], { z0.s - z1.s }, z0.s
	bfmlal	za.h[w8, 0:1], { z0.h - z1.h }, z0.h

	bfmlal	za.s[w7, 0:1], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w12, 0:1], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 0], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 0:0], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 0:2], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 1:2], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 1:0], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, -2:-1], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 8:9], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 16:17], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 0:1, vgx2], { z0.h - z3.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z1.h, z3.h, z5.h, z7.h }, z0.h
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, z16.h
	bfmlal	za.s[w8, 0:1], { z0.s - z3.s }, z0.s
	bfmlal	za.h[w8, 0:1], { z0.h - z3.h }, z0.h

	bfmlal	za.s[w7, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w12, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, -2:-1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 1:2], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 8:9], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1], { z1.h - z2.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z15.h - z16.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z31.h, z0.h }

	bfmlal	za.s[w7, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w12, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, -2:-1], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 1:2], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 8:9], { z0.h - z3.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1], { z1.h - z4.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1], { z2.h - z5.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1], { z3.h - z6.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, { z15.h - z18.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, { z29.h, z30.h, z31.h, z0.h }

	bfmlal	za.s[w8, 0:1], { z0.h - z2.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z3.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z2.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1], { z0.h - z1.h }, { z0.h - z4.h }

	bfmlal	za.s[w8, 0:1, vgx4], { z0.h - z1.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1, vgx4], { z0.h - z3.h }, { z0.h - z1.h }
	bfmlal	za.s[w8, 0:1, vgx2], { z0.h - z1.h }, { z0.h - z3.h }
	bfmlal	za.s[w8, 0:1, vgx2], { z0.h - z3.h }, { z0.h - z1.h }
	bfmlal	za[w8, 0:1], { z0.h - z1.h }, { z0.h - z1.h }
	bfmlal	za[w8, 0:1], { z0.h - z3.h }, { z0.h - z3.h }
