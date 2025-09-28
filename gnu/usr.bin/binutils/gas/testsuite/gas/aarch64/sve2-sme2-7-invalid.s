	fclamp	0, z0.h, z0.h
	fclamp	z0.h, 0, z0.h
	fclamp	z0.h, z0.h, 0

	fclamp	z0.b, z0.b, z0.b
	fclamp	z0.h, { z0.h, z0.h }
	fclamp	z0.s, z0.h, z0.h
	fclamp	z0.h, z0.s, z0.h
	fclamp	z0.h, z0.h, z0.s
