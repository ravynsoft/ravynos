	svdot	za.d[w8, 0], { z0.h - z1.h }, z0.h[0]

	svdot	za.d[w8, 0], { z0.h - z3.h }, z0.h[-1]
	svdot	za.d[w8, 0], { z0.h - z3.h }, z0.h[2]
	svdot	za.d[w8, 0], { z1.h - z4.h }, z0.h[0]
	svdot	za.d[w8, 0], { z2.h - z5.h }, z0.h[0]
	svdot	za.d[w8, 0], { z3.h - z6.h }, z0.h[0]

	svdot	za.d[w8, 0], { z0.h - z1.h }, z0.h
	svdot	za.d[w8, 0], { z0.h - z3.h }, z0.h
	svdot	za.d[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	svdot	za.d[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
