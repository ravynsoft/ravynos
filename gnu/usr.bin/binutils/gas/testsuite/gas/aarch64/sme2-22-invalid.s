	fclamp	0, z0.h, z0.h
	fclamp	{ z0.h - z1.h }, 0, z0.h
	fclamp	{ z0.h - z1.h }, z0.h, 0

	fclamp	{ z0.b - z1.b }, z0.b, z0.b
	fclamp	{ z0.b - z3.b }, z0.b, z0.b
	fclamp	{ z0.q - z1.q }, z0.q, z0.q

	fclamp	{ z0.h - z2.h }, z0.h, z0.h
	fclamp	{ z1.h - z2.h }, z0.h, z0.h
	fclamp	{ z1.h - z4.h }, z0.h, z0.h
	fclamp	{ z2.h - z5.h }, z0.h, z0.h
	fclamp	{ z3.h - z6.h }, z0.h, z0.h
