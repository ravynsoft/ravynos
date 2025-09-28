	zero	0

	zero	zt0
	zero	{
	zero	{ foo }
	zero	{ zt }
	zero	{ x0 }
	zero	{ z0 }
	zero	{ zt0
	zero	{ zt0.b }
	zero	{ zt0, zt0 }

	movt	0, zt0[0]
	movt	x0, 0

	movt	zt0, x0
	movt	za[0], x0
	movt	za0[0], x0
	movt	zt0[#0], x0
	movt	zt0[-1], x0
	movt	zt0[1],x0
	movt	zt0[2],x0
	movt	zt0[4],x0
	movt	zt0[7],x0
	movt	zt0[49],x0
	movt	zt0[50],x0
	movt	zt0[52],x0
	movt	zt0[57],x0
	movt	zt0[64], x0
	movt	zt0[1<<32], x0
	movt	zt0.b[0], x0
	movt	zt0/z[0], x0
	movt	zt0[0], sp
	movt	zt0[0], w0
	movt	zt0[0], wsp
	movt	zt0[0], wzr
	movt	zt0[0], 0

	ldr	0, [x0]
	ldr	zt0, 0

	ldr	zt0, [x0, #0]
	ldr	Zt0, [x0]
	ldr	zT0, [x0]
	ldr	zt0, [x0, #0, mul vl]
	ldr	zt0, [w0]
	ldr	zt0, [x0]!
	ldr	zt0, [xzr]
	ldr	zt0, [wsp]
	ldr	zt0, [x0, xzr]
	ldr	zt0, [x1, x2]

	luti2	z0.b, zt0, z0[-1]
	luti2	z0.b, zt0, z0[16]
	luti2	z0.b, zt0, z0.b[0]
	luti2	z0, zt0, z0[0]
	luti2	z0.d, zt0, z0[0]
	luti2	z0.q, zt0, z0[0]
	luti2	z0.b, zt0, zt0

	luti2	0, zt0, z0[0]
	luti2	z0.b, 0, z0[0]
	luti2	z0.b, zt0, 0

	luti2	{ z1.b - z2.b }, zt0, z0[0]
	luti2	{ z0.b - z1.b }, z0, z0[0]
	luti2	{ z0.b - z1.b }, za, z0[0]
	luti2	{ z0.h - z1.h }, zt0, z0.h[0]
	luti2	{ z0.h - z1.h }, zt0, z0[-1]
	luti2	{ z0.h - z1.h }, zt0, z0[8]
	luti2	{ z0.d - z1.d }, zt0, z0[0]
	luti2	{ z0.q - z1.q }, zt0, z0[0]

	luti2	{ z1.s - z4.s }, zt0, z0[0]
	luti2	{ z2.s - z5.s }, zt0, z0[0]
	luti2	{ z3.s - z6.s }, zt0, z0[0]
	luti2	{ z0.s - z3.s }, z0, z0[0]
	luti2	{ z0.b - z3.b }, za, z0[0]
	luti2	{ z0.b - z3.b }, zt0, z0.b[0]
	luti2	{ z0.b - z3.b }, zt0, z0[-1]
	luti2	{ z0.b - z3.b }, zt0, z0[4]
	luti2	{ z0.d - z3.d }, zt0, z0[0]
	luti2	{ z0.q - z3.q }, zt0, z0[0]

	luti4	0, zt0, z0[0]
	luti4	z0.b, 0, z0[0]
	luti4	z0.b, zt0, 0

	luti4	z0.h, zt0, z0[-1]
	luti4	z0.h, zt0, z0[8]
	luti4	z0.h, zt0, z0.h[0]
	luti4	z0, zt0, z0[0]
	luti4	z0.d, zt0, z0[0]
	luti4	z0.q, zt0, z0[0]
	luti4	z0.h, zt0, zt0

	luti4	{ z1.h - z2.h }, zt0, z0[0]
	luti4	{ z0.h - z1.h }, z0, z0[0]
	luti4	{ z0.h - z1.h }, za, z0[0]
	luti4	{ z0.h - z1.h }, zt0, z0.h[0]
	luti4	{ z0.h - z1.h }, zt0, z0[-1]
	luti4	{ z0.h - z1.h }, zt0, z0[4]
	luti4	{ z0.d - z1.d }, zt0, z0[0]
	luti4	{ z0.q - z1.q }, zt0, z0[0]

	luti4	{ z1.s - z4.s }, zt0, z0[0]
	luti4	{ z2.s - z5.s }, zt0, z0[0]
	luti4	{ z3.s - z6.s }, zt0, z0[0]
	luti4	{ z0.s - z3.s }, z0, z0[0]
	luti4	{ z0.s - z3.s }, za, z0[0]
	luti4	{ z0.s - z3.s }, zt0, z0.s[0]
	luti4	{ z0.s - z3.s }, zt0, z0[-1]
	luti4	{ z0.s - z3.s }, zt0, z0[2]
	luti4	{ z0.b - z3.b }, zt0, z0[0]
	luti4	{ z0.d - z3.d }, zt0, z0[0]
	luti4	{ z0.q - z3.q }, zt0, z0[0]
