	bfvdot	0, { z0.h - z1.h }, z0.h[0]
	bfvdot	za.s[w8, 0], 0, z0.h[0]
	bfvdot	za.s[w8, 0], { z0.h - z1.h }, 0

	fvdot	za.h[w8, 0], z0.h, z0.h
	fvdot	za.h[w8, 0], { z0.h - z1.h }, z0.h
	fvdot	za.s[w8, 0], { z0.b - z1.h }, z0.b[0]
	fvdot	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	fvdot	za.s[w8, 0, vgx4], { z0.h - z1.h }, z0.h[0]

	fvdot	za.s[w7, 0], { z0.h - z1.h }, z0.h[0]
	fvdot	za.s[w12, 0], { z0.h - z1.h }, z0.h[0]
	fvdot	za.s[w8, -1], { z0.h - z1.h }, z0.h[0]
	fvdot	za.s[w8, 8], { z0.h - z1.h }, z0.h[0]
	fvdot	za.s[w8, 0], { z0.h - z2.h }, z0.h[0]
	fvdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	fvdot	za.s[w8, 0], { z1.h - z2.h }, z0.h[0]
	fvdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[-1]
	fvdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[4]
	fvdot	za.s[w8, 0], { z0.h - z1.h }, z16.h[0]
