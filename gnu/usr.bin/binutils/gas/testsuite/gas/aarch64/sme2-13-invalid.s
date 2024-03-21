	smlall	0, z0.b, z0.b[0]
	smlall	za.s[w8, 0:3], 0, z0.b[0]
	smlall	za.s[w8, 0:3], z0.b, 0

	smlall	za0.s[w8, 0:3], z0.b, z0.b[0]
	smlall	za0h.s[w8, 0:3], z0.b, z0.b[0]
	smlall	za.s[w7, 0:3], z0.b, z0.b[0]
	smlall	za.s[w12, 0:3], z0.b, z0.b[0]
	smlall	za.s[w8, 0], z0.b, z0.b[0]
	smlall	za.s[w8, 0:1], z0.b, z0.b[0]
	smlall	za.s[w8, 0:2], z0.b, z0.b[0]
	smlall	za.s[w8, 0, vgx4], z0.b, z0.b[0]
	smlall	za.s[w8, 0:3, vgx2], z0.b, z0.b[0]
	smlall	za.s[w8, 0:3, vgx4], z0.b, z0.b[0]
	smlall	za.s[w8, 1:4], z0.b, z0.b[0]
	smlall	za.s[w8, 2:5], z0.b, z0.b[0]
	smlall	za.s[w8, 3:6], z0.b, z0.b[0]
	smlall	za.s[w8, 16:19], z0.b, z0.b[0]
	smlall	za.s[w8, 0:3], z0.b, z16.b[0]
	smlall	za.s[w8, 0:3], z0.b, z0.b[-1]
	smlall	za.s[w8, 0:3], z0.b, z0.b[16]
	smlall	za.s[w8, 0:3], z0.h, z0.h[0]
	smlall	za.s[w8, 0:3], z0.s, z0.s[0]

	smlall	za.s[w7, 0:3], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w12, 0:3], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 0], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 0:1], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 0:2], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 0:3, vgx4], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 1:4], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 2:5], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 3:6], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 8:11], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z1.b - z2.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z16.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[-1]
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[16]

	smlall	za.s[w7, 0:3], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w12, 0:3], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 0], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 0:1], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 0:2], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 0:3, vgx2], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 1:4], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 2:5], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 3:6], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 8:11], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z1.b - z4.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z2.b - z5.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z3.b - z6.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z16.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[-1]
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[16]

	smlall	za.s[w8, 0:3, vgx2], z0.b, z0.b
	smlall	za.s[w8, 0:3, vgx4], z0.b, z0.b
	smlall	za.s[w8, 16:19], z0.b, z0.b
	smlall	za.s[w8, 0:3], z0.b, z16.b

	smlall	za.s[w8, 0:3, vgx4], { z0.b - z1.b }, z0.b
	smlall	za.s[w8, 8:11], { z0.b - z1.b }, z0.b
	smlall	za.s[w8, 0:3], { z0.b - z2.b }, z0.b
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z16.b

	smlall	za.s[w8, 0:3, vgx2], { z0.b - z3.b }, z0.b
	smlall	za.s[w8, 8:11], { z0.b - z3.b }, z0.b
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z16.b

	smlall	za.s[w8, 0:3, vgx4], { z0.b - z1.b }, { z0.b - z1.b }
	smlall	za.s[w8, 8:11], { z0.b - z1.b }, { z0.b - z1.b }
	smlall	za.s[w8, 0:3], { z1.b - z2.b }, { z0.b - z1.b }
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, { z1.b - z2.b }

	smlall	za.s[w8, 0:3, vgx2], { z0.b - z3.b }, { z0.b - z3.b }
	smlall	za.s[w8, 8:11], { z0.b - z3.b }, { z0.b - z3.b }
	smlall	za.s[w8, 0:3], { z1.b - z4.b }, { z0.b - z3.b }
	smlall	za.s[w8, 0:3], { z2.b - z5.b }, { z0.b - z3.b }
	smlall	za.s[w8, 0:3], { z3.b - z6.b }, { z0.b - z3.b }
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, { z1.b - z4.b }
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, { z2.b - z5.b }
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, { z3.b - z6.b }
