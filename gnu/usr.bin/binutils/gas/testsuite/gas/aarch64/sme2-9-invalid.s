	add	0, { z0.s - z1.s }
	add	za.s[w8, 0], 0

	add	za.s[w7, 0], { z0.s - z1.s }
	add	za.s[w12, 0], { z0.s - z1.s }
	add	za.s[w8, -1], { z0.s - z1.s }
	add	za.s[w8, 8], { z0.s - z1.s }
	add	za.s[w8, 0], { z0.s - z2.s }
	add	za.s[w8, 0], { z1.s - z2.s }

	add	za.s[w7, 0], { z0.s - z3.s }
	add	za.s[w12, 0], { z0.s - z3.s }
	add	za.s[w8, -1], { z0.s - z3.s }
	add	za.s[w8, 8], { z1.s - z3.s }
	add	za.s[w8, 0], { z1.s - z4.s }
	add	za.s[w8, 0], { z2.s - z5.s }
	add	za.s[w8, 0], { z3.s - z6.s }

	add	za.s[w8, 0, vgx4], { z0.s - z1.s }
	add	za.s[w8, 0, vgx2], { z0.s - z3.s }
	add	za[w8, 0], { z0.s - z1.s }
	add	za.s[w8, 0], { z0 - z1 }

	add	0, { z0.s - z1.s }, z0.s
	add	za.s[w8, 0], 0, z0.s
	add	za.s[w8, 0], { z0.s - z1.s }, 0

	add	za.s[w0, 0], { z0.s - z1.s }, z0.s
	add	za.s[w31, 0], { z0.s - z1.s }, z0.s
	add	za.s[w8, 1<<63], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0], { z0.s - z1.s }, z31.s
	add	za.s[w8, 0:0], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0:-1], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0:1], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0:100], { z0.s - z1.s }, z0.s

	add	za.s[w7, 0], { z0.s - z1.s }, z0.s
	add	za.s[w12, 0], { z0.s - z1.s }, z0.s
	add	za.s[w8, -1], { z0.s - z1.s }, z0.s
	add	za.s[w8, 8], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0], { z0.s - z1.s }, z16.s

	add	za.s[w7, 0], { z0.s - z3.s }, z0.s
	add	za.s[w12, 0], { z0.s - z3.s }, z0.s
	add	za.s[w8, -1], { z0.s - z3.s }, z0.s
	add	za.s[w8, 8], { z0.s - z3.s }, z0.s
	add	za.s[w8, 0], { z0.s - z3.s }, z16.s

	add	za.s[w8, 0], { z0.s - z2.s }, z0.s
	add	za.s[w8, 0], { z0.s - z4.s }, z0.s
	add	za.s[w8, 0], { z0.s, z1.s, z2.s }, z0.s
	add	za.s[w8, 0], { z0.s, z1.s, z5.s }, z0.s

	add	za.s[w8, 0, vgx4], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0, vgx2], { z0.s - z3.s }, z0.s
	add	za[w8, 0], { z0.s - z1.s }, z0.s
	add	za.s[w8, 0], { z0 - z1 }, z0.s
	add	za.s[w8, 0], { z0.s - z1.s }, z0
	add	za[w8, 0], { z0.s - z1.s }, z0

	add	za.s[w7, 0], { z0.s - z1.s }, { z0.s - z1.s }
	add	za.s[w12, 0], { z0.s - z1.s }, { z0.s - z1.s }
	add	za.s[w8, -1], { z0.s - z1.s }, { z0.s - z1.s }
	add	za.s[w8, 8], { z0.s - z1.s }, { z0.s - z1.s }
	add	za.s[w8, 0], { z1.s - z2.s }, { z0.s - z1.s }
	add	za.s[w8, 0], { z0.s - z1.s }, { z15.s - z16.s }
	add	za.s[w8, 0], { z0.s - z1.s }, { z31.s, z0.s }

	add	za.s[w7, 0], { z0.s - z3.s }, { z0.s - z3.s }
	add	za.s[w12, 0], { z0.s - z3.s }, { z0.s - z3.s }
	add	za.s[w8, -1], { z0.s - z3.s }, { z0.s - z3.s }
	add	za.s[w8, 8], { z0.s - z3.s }, { z0.s - z3.s }
	add	za.s[w8, 0], { z1.s - z4.s }, { z0.s - z3.s }
	add	za.s[w8, 0], { z2.s - z5.s }, { z0.s - z3.s }
	add	za.s[w8, 0], { z3.s - z6.s }, { z0.s - z3.s }
	add	za.s[w8, 0], { z0.s - z3.s }, { z15.s - z18.s }
	add	za.s[w8, 0], { z0.s - z3.s }, { z29.s, z30.s, z31.s, z0.s }

	add	za.s[w8, 0], { z0.s - z2.s }, { z0.s - z1.s }
	add	za.s[w8, 0], { z0.s - z3.s }, { z0.s - z1.s }
	add	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z2.s }
	add	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z3.s }
	add	za.s[w8, 0], { z0.s - z1.s }, { z0.s - z4.s }

	add	za.s[w8, 0, vgx4], { z0.s - z1.s }, { z0.s - z3.s }
	add	za.s[w8, 0, vgx4], { z0.s - z3.s }, { z0.s - z1.s }
	add	za.s[w8, 0, vgx2], { z0.s - z1.s }, { z0.s - z3.s }
	add	za.s[w8, 0, vgx2], { z0.s - z3.s }, { z0.s - z1.s }
	add	za[w8, 0], { z0.s - z1.s }, { z0.s - z1.s }
	add	za[w8, 0], { z0.s - z3.s }, { z0.s - z3.s }

	add	{ z0.b - z1.b }, { z0.b - z2.b }, z0.b
	add	{ z0.b - z1.b }, { z0.b - z3.b }, z0.b
	add	{ z0.b - z2.b }, { z0.b - z2.b }, z0.b
	add	{ z0.b - z1.b }, { z2.b - z3.b }, z0.b
	add	{ z1.b - z2.b }, { z1.b - z2.b }, z0.b
	add	{ z31.b, z0.b }, { z31.b, z0.b }, z0.b
	add	{ z0.b - z1.b }, { z0.b - z1.b }, z16.b
	add	{ z0.b - z1.b }, { z0.b - z1.b }, z31.b
	add	{ z0.b - z1.b }, { z0.h - z1.h }, z0.b
	add	{ z0.b - z1.b }, { z0.b - z1.b }, z0.h
	add	{ z0.q - z1.q }, { z0.q - z1.q }, z0.q

	add	{ z0.b - z3.b }, { z0.b - z2.b }, z0.b
	add	{ z0.b - z3.b }, { z0.b - z1.b }, z0.b
	add	{ z0.b - z3.b }, { z2.b - z5.b }, z0.b
	add	{ z1.b - z4.b }, { z1.b - z4.b }, z0.b
	add	{ z2.b - z5.b }, { z2.b - z5.b }, z0.b
	add	{ z3.b - z6.b }, { z3.b - z6.b }, z0.b
	add	{ z31.b, z0.b, z1.b, z2.b }, { z31.b, z0.b, z1.b, z2.b }, z0.b
	add	{ z0.b - z3.b }, { z0.b - z3.b }, z16.b
	add	{ z0.b - z3.b }, { z0.b - z3.b }, z31.b
	add	{ z0.b - z3.b }, { z0.h - z3.h }, z0.b
	add	{ z0.b - z3.b }, { z0.b - z3.b }, z0.h
	add	{ z0.q - z3.q }, { z0.q - z3.q }, z0.q

	sub	{ z0.b - z1.b }, { z0.b - z1.b }, z0.b
	sub	{ z0.h - z1.h }, { z0.h - z1.h }, z0.h
	sub	{ z0.s - z1.s }, { z0.s - z1.s }, z0.s
	sub	{ z0.d - z1.d }, { z0.d - z1.d }, z0.d

	sub	{ z0.b - z3.b }, { z0.b - z3.b }, z0.b
	sub	{ z0.h - z3.h }, { z0.h - z3.h }, z0.h
	sub	{ z0.s - z3.s }, { z0.s - z3.s }, z0.s
	sub	{ z0.d - z3.d }, { z0.d - z3.d }, z0.d

	fadd	za.b[w8, 0], { z0.b - z1.b }
	fadd	za.h[w8, 0], { z0.h - z1.h }
