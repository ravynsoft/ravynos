	sdot	za.d[w8, 0], { z0.h - z1.h }, z0.h[-1]
	sdot	za.d[w8, 0], { z0.h - z1.h }, z0.h[2]

	sdot	za.d[w8, 0], { z0.h - z3.h }, z0.h[-1]
	sdot	za.d[w8, 0], { z0.h - z3.h }, z0.h[2]

	sudot	za.d[w8, 0], { z0.h - z1.h }, z0.h[0]
	sudot	za.d[w8, 0], { z0.h - z3.h }, z0.h[0]
	sudot	za.d[w8, 0], { z0.h - z1.h }, z0.h
	sudot	za.d[w8, 0], { z0.h - z3.h }, z0.h
	sudot	za.d[w8, 0], { z0.h - z1.h }, { z0.h - z1.h }
	sudot	za.d[w8, 0], { z0.h - z3.h }, { z0.h - z3.h }
