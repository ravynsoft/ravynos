	sudot	0, { z0.b - z1.b }, z0.b[0]
	sudot	za.s[w8, 0], 0, z0.b[0]
	sudot	za.s[w8, 0], { z0.b - z1.b }, 0

	sudot	za.s[w8, 0], { z0.h - z1.h }, z0.h[0]
	sudot	za.s[w8, 0], { z0.h - z3.h }, z0.h[0]
	sudot	za.s[w8, 0], { z0.h - z1.h }, z0.h
	sudot	za.s[w8, 0], { z0.h - z3.h }, z0.h
	sudot	za.s[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	sudot	za.s[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
	sudot	za.s[w8, 0], { z0.b - z1.b }, { z0.b - z1.b }
	sudot	za.s[w8, 0], { z0.b - z3.b }, { z0.b - z3.b }
