	sumlall	0, z0.b, z0.b[0]
	sumlall	za.s[w8, 0:3], 0, z0.b[0]
	sumlall	za.s[w8, 0:3], z0.b, 0

	sumlall	za.s[w8, 0:3], z0.b, z0.b
	sumlall	za.s[w8, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	sumlall	za.s[w8, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
