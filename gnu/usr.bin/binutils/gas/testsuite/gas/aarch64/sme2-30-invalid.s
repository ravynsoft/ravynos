	uzp	0, z0.b, z0.b
	uzp	{ z0.b - z1.b }, 0, z0.b
	uzp	{ z0.b - z1.b }, z0.b, 0

	uzp	{ z1.b - z2.b }, z0.b, z0.b
	uzp	{ z0.b - z2.b }, z0.b, z0.b
	uzp	{ z0.b - z3.b }, z0.b, z0.b
	uzp	{ z0.b - z1.b }, { z0.b - z1.b }, { z0.b, z1.b }
	uzp	{ z0.h - z1.h }, z0.b, z0.b
	uzp	{ z0.q - z3.q }, z0.b, z0.b

	uzp	{ z0.b - z3.b }, { z0.b - z1.b }, { z2.b - z3.b }
	uzp	{ z1.b - z4.b }, { z0.b - z3.b }
	uzp	{ z2.b - z5.b }, { z0.b - z3.b }
	uzp	{ z3.b - z6.b }, { z0.b - z3.b }
	uzp	{ z0.b - z3.b }, { z1.b - z4.b }
	uzp	{ z0.b - z3.b }, { z2.b - z5.b }
	uzp	{ z0.b - z3.b }, { z3.b - z6.b }
