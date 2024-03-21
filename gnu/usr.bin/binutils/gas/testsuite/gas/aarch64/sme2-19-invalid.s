	svdot	0, { z0.h - z1.h }, z0.h[0]
	svdot	za.s[w8, 0], 0, z0.h[0]
	svdot	za.s[w8, 0], { z0.h - z1.h }, 0

	svdot	za.h[w8, 0], z0.h, z0.h
	svdot	za.h[w8, 0], { z0.h - z1.h }, z0.h

	svdot	za.s[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	svdot	za.s[w8, 0, vgx4], { z0.h - z1.h }, z0.h[0]

	svdot	za.s[w7, 0], { z0.h - z1.h }, z0.h[0]
	svdot	za.s[w12, 0], { z0.h - z1.h }, z0.h[0]
	svdot	za.s[w8, -1], { z0.h - z1.h }, z0.h[0]
	svdot	za.s[w8, 8], { z0.h - z1.h }, z0.h[0]
	svdot	za.s[w8, 0], { z0.h - z2.h }, z0.h[0]
	svdot	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	svdot	za.s[w8, 0], { z1.h - z2.h }, z0.h[0]
	svdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[-1]
	svdot	za.s[w8, 0], { z0.h - z1.h }, z0.h[4]
	svdot	za.s[w8, 0], { z0.h - z1.h }, z16.h[0]

	svdot	za.s[w8, 0:1], { z0.b - z3.b }, z0.h[0]
	svdot	za.s[w8, 0, vgx4], { z0.b - z3.b }, z0.h[0]

	svdot	za.s[w7, 0], { z0.b - z3.b }, z0.b[0]
	svdot	za.s[w12, 0], { z0.b - z3.b }, z0.b[0]
	svdot	za.s[w8, -1], { z0.b - z3.b }, z0.b[0]
	svdot	za.s[w8, 8], { z0.b - z3.b }, z0.b[0]
	svdot	za.s[w8, 0], { z0.b - z1.b }, z0.b[0]
	svdot	za.s[w8, 0], { z0.b - z2.b }, z0.b[0]
	svdot	za.s[w8, 0], { z1.b - z4.b }, z0.b[0]
	svdot	za.s[w8, 0], { z2.b - z5.b }, z0.b[0]
	svdot	za.s[w8, 0], { z3.b - z6.b }, z0.b[0]
	svdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[-1]
	svdot	za.s[w8, 0], { z0.b - z3.b }, z0.b[4]
	svdot	za.s[w8, 0], { z0.b - z3.b }, z16.b[0]
