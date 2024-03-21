	suvdot	0, { z0.b - z3.b }, z0.b[0]
	suvdot	za.s[w8, 0], 0, z0.b[0]
	suvdot	za.s[w8, 0], { z0.b - z3.b }, 0

	suvdot	za.h[w8, 0], z0.h, z0.h
	suvdot	za.h[w8, 0], { z0.h - z1.h }, z0.h
	suvdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[0]

	suvdot	za.s[w8, 0:1], { z0.b - z3.b }, z0.h[0]
	suvdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.h[0]

	suvdot	za.s[w7, 0], { z0.b - z3.b }, z0.b[0]
	suvdot	za.s[w12, 0], { z0.b - z3.b }, z0.b[0]
	suvdot	za.s[w8, -1], { z0.b - z3.b }, z0.b[0]
	suvdot	za.s[w8, 8], { z0.b - z3.b }, z0.b[0]
	suvdot	za.s[w8, 0], { z0.b - z1.b }, z0.b[0]
	suvdot	za.s[w8, 0], { z0.b - z2.b }, z0.b[0]
	suvdot	za.s[w8, 0], { z1.b - z4.b }, z0.b[0]
	suvdot	za.s[w8, 0], { z2.b - z5.b }, z0.b[0]
	suvdot	za.s[w8, 0], { z3.b - z6.b }, z0.b[0]
	suvdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[-1]
	suvdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[4]
	suvdot	za.s[w8, 0], { z0.b - z3.b }, z16.b[0]
