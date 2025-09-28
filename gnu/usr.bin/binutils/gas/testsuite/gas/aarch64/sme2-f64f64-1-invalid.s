	fadd	za.d[w7, 0], { z0.d - z1.d }
	fadd	za.d[w12, 0], { z0.d - z1.d }
	fadd	za.d[w8, -1], { z0.d - z1.d }
	fadd	za.d[w8, 8], { z0.d - z1.d }
	fadd	za.d[w8, 0], { z0.d - z2.d }
	fadd	za.d[w8, 0], { z1.d - z2.d }

	fadd	za.d[w7, 0], { z0.d - z3.d }
	fadd	za.d[w12, 0], { z0.d - z3.d }
	fadd	za.d[w8, -1], { z0.d - z3.d }
	fadd	za.d[w8, 8], { z1.d - z3.d }
	fadd	za.d[w8, 0], { z1.d - z4.d }
	fadd	za.d[w8, 0], { z2.d - z5.d }
	fadd	za.d[w8, 0], { z3.d - z6.d }

	fadd	za.d[w8, 0, vgx4], { z0.d - z1.d }
	fadd	za.d[w8, 0, vgx2], { z0.d - z3.d }
	fadd	za[w8, 0], { z0.d - z1.d }
	fadd	za.d[w8, 0], { z0 - z1 }
	fadd	za.d[w8, 0], { z0.s - z1.s }
