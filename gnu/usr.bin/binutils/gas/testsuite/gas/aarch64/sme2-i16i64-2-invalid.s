	smlall	za0.d[w8, 0:3], z0.h, z0.h[0]
	smlall	za0h.d[w8, 0:3], z0.h, z0.h[0]
	smlall	za.d[w7, 0:3], z0.h, z0.h[0]
	smlall	za.d[w12, 0:3], z0.h, z0.h[0]
	smlall	za.d[w8, 0], z0.h, z0.h[0]
	smlall	za.d[w8, 0:1], z0.h, z0.h[0]
	smlall	za.d[w8, 0:2], z0.h, z0.h[0]
	smlall	za.d[w8, 0, vgx4], z0.h, z0.h[0]
	smlall	za.d[w8, 0:3, vgx2], z0.h, z0.h[0]
	smlall	za.d[w8, 0:3, vgx4], z0.h, z0.h[0]
	smlall	za.d[w8, 1:4], z0.h, z0.h[0]
	smlall	za.d[w8, 2:5], z0.h, z0.h[0]
	smlall	za.d[w8, 3:6], z0.h, z0.h[0]
	smlall	za.d[w8, 16:19], z0.h, z0.h[0]
	smlall	za.d[w8, 0:3], z0.h, z16.h[0]
	smlall	za.d[w8, 0:3], z0.h, z0.h[-1]
	smlall	za.d[w8, 0:3], z0.h, z0.h[8]
	smlall	za.d[w8, 0:3], z0.b, z0.b[0]
	smlall	za.d[w8, 0:3], z0.d, z0.d[0]

	smlall	za.d[w7, 0:3], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w12, 0:3], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 0], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 0:1], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 0:2], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 0:3, vgx4], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 1:4], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 2:5], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 3:6], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 8:11], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z1.h - z2.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z16.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[-1]
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[8]

	smlall	za.d[w7, 0:3], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w12, 0:3], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 0], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 0:1], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 0:2], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 0:3, vgx2], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 1:4], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 2:5], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 3:6], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 8:11], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z1.h - z4.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z2.h - z5.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z3.h - z6.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z16.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[-1]
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[8]

	smlall	za.d[w8, 0:3, vgx2], z0.h, z0.h
	smlall	za.d[w8, 0:3, vgx4], z0.h, z0.h
	smlall	za.d[w8, 16:19], z0.h, z0.h
	smlall	za.d[w8, 0:3], z0.h, z16.h

	smlall	za.d[w8, 0:3, vgx4], { z0.h - z1.h }, z0.h
	smlall	za.d[w8, 8:11], { z0.h - z1.h }, z0.h
	smlall	za.d[w8, 0:3], { z0.h - z2.h }, z0.h
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z16.h

	smlall	za.d[w8, 0:3, vgx2], { z0.h - z3.h }, z0.h
	smlall	za.d[w8, 8:11], { z0.h - z3.h }, z0.h
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z16.h

	smlall	za.d[w8, 0:3, vgx4], { z0.h - z1.h }, { z0.h - z1.h }
	smlall	za.d[w8, 8:11], { z0.h - z1.h }, { z0.h - z1.h }
	smlall	za.d[w8, 0:3], { z1.h - z2.h }, { z0.h - z1.h }
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, { z1.h - z2.h }

	smlall	za.d[w8, 0:3, vgx2], { z0.h - z3.h }, { z0.h - z3.h }
	smlall	za.d[w8, 8:11], { z0.h - z3.h }, { z0.h - z3.h }
	smlall	za.d[w8, 0:3], { z1.h - z4.h }, { z0.h - z3.h }
	smlall	za.d[w8, 0:3], { z2.h - z5.h }, { z0.h - z3.h }
	smlall	za.d[w8, 0:3], { z3.h - z6.h }, { z0.h - z3.h }
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, { z1.h - z4.h }
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, { z2.h - z5.h }
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, { z3.h - z6.h }

	sumlall	za.d[w8, 0:3], z0.h, z0.h[0]
	sumlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[0]
	sumlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[0]
	sumlall	za.d[w8, 0:3], z0.h, z0.h
	sumlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h
	sumlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h
	sumlall	za.d[w8, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	sumlall	za.d[w8, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
