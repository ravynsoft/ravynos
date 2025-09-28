	zero	{ zt0 }
	ZERO	{ ZT0 }

	movt	x0, zt0[0]
	MOVT	X0, ZT0[0]
	movt	x30, zt0[0]
	movt	xzr, zt0[0]
	movt	x0, zt0[56]
	movt	x9, zt0[24]
	movt	x15, zt0[40]
	movt	x22, zt0[48]

	movt	zt0[0], x0
	MOVT	ZT0[0], X0
	movt	zt0[56], x0
	movt	zt0[0], x30
	movt	zt0[0], xzr
	movt	zt0[8], x20
	movt	zt0[16], x25
	movt	zt0[32], x27
	movt	zt0[24], x29

	ldr	zt0, [x0]
	LDR	ZT0, [X0]
	ldr	zt0, [x30]
	ldr	zt0, [sp]

	str	zt0, [x0]
	STR	ZT0, [X0]
	str	zt0, [x30]
	str	zt0, [sp]

	luti2	z0.b, zt0, z0[0]
	LUTI2	Z0.B, ZT0, Z0[0]
	luti2	z31.b, zt0, z0[0]
	luti2	z0.b, zt0, z31[0]
	luti2	z0.b, zt0, z0[15]

	luti2	z0.h, zt0, z0[0]
	luti2	z31.h, zt0, z0[0]
	luti2	z0.h, zt0, z31[0]
	luti2	z0.h, zt0, z0[15]

	luti2	z0.s, zt0, z0[0]
	luti2	z31.s, zt0, z0[0]
	luti2	z0.s, zt0, z31[0]
	luti2	z0.s, zt0, z0[15]

	luti2	{ z0.b - z1.b }, zt0, z0[0]
	LUTI2	{ Z0.B - Z1.B }, ZT0, Z0[0]
	luti2	{ z30.b - z31.b }, zt0, z0[0]
	luti2	{ z0.b - z1.b }, zt0, z31[0]
	luti2	{ z0.b - z1.b }, zt0, z0[7]

	luti2	{ z0.h - z1.h }, zt0, z0[0]
	luti2	{ z30.h - z31.h }, zt0, z0[0]
	luti2	{ z0.h - z1.h }, zt0, z31[0]
	luti2	{ z0.h - z1.h }, zt0, z0[7]

	luti2	{ z0.s - z1.s }, zt0, z0[0]
	luti2	{ z30.s - z31.s }, zt0, z0[0]
	luti2	{ z0.s - z1.s }, zt0, z31[0]
	luti2	{ z0.s - z1.s }, zt0, z0[7]

	luti2	{ z0.b - z3.b }, zt0, z0[0]
	LUTI2	{ Z0.B - Z3.B }, ZT0, Z0[0]
	luti2	{ z28.b - z31.b }, zt0, z0[0]
	luti2	{ z0.b - z3.b }, zt0, z31[0]
	luti2	{ z0.b - z3.b }, zt0, z0[3]

	luti2	{ z0.h - z3.h }, zt0, z0[0]
	luti2	{ z28.h - z31.h }, zt0, z0[0]
	luti2	{ z0.h - z3.h }, zt0, z31[0]
	luti2	{ z0.h - z3.h }, zt0, z0[3]

	luti2	{ z0.s - z3.s }, zt0, z0[0]
	luti2	{ z28.s - z31.s }, zt0, z0[0]
	luti2	{ z0.s - z3.s }, zt0, z31[0]
	luti2	{ z0.s - z3.s }, zt0, z0[3]

	luti4	z0.b, zt0, z0[0]
	LUTI4	Z0.b, ZT0, Z0[0]
	luti4	z31.b, zt0, z0[0]
	luti4	z0.b, zt0, z31[0]
	luti4	z0.b, zt0, z0[7]

	luti4	z0.h, zt0, z0[0]
	LUTI4	Z0.H, ZT0, Z0[0]
	luti4	z31.h, zt0, z0[0]
	luti4	z0.h, zt0, z31[0]
	luti4	z0.h, zt0, z0[7]

	luti4	z0.s, zt0, z0[0]
	luti4	z31.s, zt0, z0[0]
	luti4	z0.s, zt0, z31[0]
	luti4	z0.s, zt0, z0[7]

	luti4	{ z0.b - z1.b }, zt0, z0[0]
	LUTI4	{ Z0.b - Z1.b }, ZT0, Z0[0]
	luti4	{ z30.b - z31.b }, zt0, z0[0]
	luti4	{ z0.b - z1.b }, zt0, z31[0]
	luti4	{ z0.b - z1.b }, zt0, z0[3]

	luti4	{ z0.h - z1.h }, zt0, z0[0]
	LUTI4	{ Z0.H - Z1.H }, ZT0, Z0[0]
	luti4	{ z30.h - z31.h }, zt0, z0[0]
	luti4	{ z0.h - z1.h }, zt0, z31[0]
	luti4	{ z0.h - z1.h }, zt0, z0[3]

	luti4	{ z0.s - z1.s }, zt0, z0[0]
	luti4	{ z30.s - z31.s }, zt0, z0[0]
	luti4	{ z0.s - z1.s }, zt0, z31[0]
	luti4	{ z0.s - z1.s }, zt0, z0[3]

	luti4	{ z0.h - z3.h }, zt0, z0[0]
	LUTI4	{ Z0.H - Z3.H }, ZT0, Z0[0]
	luti4	{ z28.h - z31.h }, zt0, z0[0]
	luti4	{ z0.h - z3.h }, zt0, z31[0]
	luti4	{ z0.h - z3.h }, zt0, z0[1]

	luti4	{ z0.s - z3.s }, zt0, z0[0]
	luti4	{ z28.s - z31.s }, zt0, z0[0]
	luti4	{ z0.s - z3.s }, zt0, z31[0]
	luti4	{ z0.s - z3.s }, zt0, z0[1]
