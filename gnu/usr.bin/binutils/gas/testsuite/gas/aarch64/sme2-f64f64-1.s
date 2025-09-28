	fadd	za.d[w8, 0], { z0.d - z1.d }
	fadd	za.d[w8, 0, vgx2], { z0.d - z1.d }
	FADD	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }
	FADD	ZA.D[W8, 0, VGX2], { Z0.D - Z1.D }
	fadd	za.d[w11, 0], { z0.d - z1.d }
	fadd	za.d[w8, 7], { z0.d - z1.d }
	fadd	za.d[w8, 0], { z30.d - z31.d }
	fadd	za.d[w10, 3], { z10.d - z11.d }

	fadd	za.d[w8, 0], { z0.d - z3.d }
	fadd	za.d[w8, 0, vgx4], { z0.d - z3.d }
	FADD	ZA.d[W8, 0, VGx4], { Z0.d - Z3.d }
	FADD	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }
	fadd	za.d[w11, 0], { z0.d - z3.d }
	fadd	za.d[w8, 7], { z0.d - z3.d }
	fadd	za.d[w8, 0], { z28.d - z31.d }
	fadd	za.d[w11, 1], { z12.d - z15.d }

	fsub	za.d[w8, 0], { z0.d - z1.d }
	fsub	za.d[w8, 0, vgx2], { z0.d - z1.d }
	FSUB	ZA.d[W8, 0, VGx2], { Z0.d - Z1.d }
	FSUB	ZA.D[W8, 0, VGX2], { Z0.D - Z1.D }
	fsub	za.d[w11, 0], { z0.d - z1.d }
	fsub	za.d[w8, 7], { z0.d - z1.d }
	fsub	za.d[w8, 0], { z30.d - z31.d }
	fsub	za.d[w10, 3], { z10.d - z11.d }

	fsub	za.d[w8, 0], { z0.d - z3.d }
	fsub	za.d[w8, 0, vgx4], { z0.d - z3.d }
	FSUB	ZA.d[W8, 0, VGx4], { Z0.d - Z3.d }
	FSUB	ZA.D[W8, 0, VGX4], { Z0.D - Z3.D }
	fsub	za.d[w11, 0], { z0.d - z3.d }
	fsub	za.d[w8, 7], { z0.d - z3.d }
	fsub	za.d[w8, 0], { z28.d - z31.d }
	fsub	za.d[w11, 1], { z12.d - z15.d }
