	sumlall	za.s[w8, 0:3], z0.b, z0.b[0]
	sumlall	za.s[w11, 0:3], z0.b, z0.b[0]
	sumlall	za.s[w8, 12:15], z0.b, z0.b[0]
	sumlall	za.s[w8, 0:3], z31.b, z0.b[0]
	sumlall	za.s[w8, 0:3], z0.b, z15.b[0]
	sumlall	za.s[w8, 0:3], z0.b, z0.b[15]
	sumlall	za.s[w9, 8:11], z21.b, z9.b[9]

	sumlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[0]
	sumlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b[0]
	sumlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b[0]
	sumlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b[0]
	sumlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b[0]
	sumlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b[0]
	sumlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[15]
	sumlall	za.s[w9, 4:7], { z18.b - z19.b }, z9.b[12]

	sumlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[0]
	sumlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b[0]
	sumlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b[0]
	sumlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b[0]
	sumlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b[0]
	sumlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b[0]
	sumlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[15]
	sumlall	za.s[w10, 0:3], { z24.b - z27.b }, z14.b[6]

	sumlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b
	sumlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b
	sumlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b
	sumlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b
	sumlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b
	sumlall	za.s[w8, 0:3], { z31.b, z0.b }, z0.b
	sumlall	za.s[w8, 0:3], { z31.b - z0.b }, z0.b
	sumlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b
	sumlall	za.s[w9, 4:7], { z19.b - z20.b }, z13.b

	sumlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b
	sumlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b
	sumlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b
	sumlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b
	sumlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b
	sumlall	za.s[w8, 0:3], { z29.b - z0.b }, z0.b
	sumlall	za.s[w8, 0:3], { z30.b, z31.b, z0.b, z1.b }, z0.b
	sumlall	za.s[w8, 0:3], { z30.b - z1.b }, z0.b
	sumlall	za.s[w8, 0:3], { z31.b - z2.b }, z0.b
	sumlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b
	sumlall	za.s[w9, 0:3], { z25.b - z28.b }, z14.b

	usmlall	za.s[w8, 0:3], z0.b, z0.b[0]
	usmlall	za.s[w11, 0:3], z0.b, z0.b[0]
	usmlall	za.s[w8, 12:15], z0.b, z0.b[0]
	usmlall	za.s[w8, 0:3], z31.b, z0.b[0]
	usmlall	za.s[w8, 0:3], z0.b, z15.b[0]
	usmlall	za.s[w8, 0:3], z0.b, z0.b[15]
	usmlall	za.s[w9, 8:11], z21.b, z9.b[9]

	usmlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[0]
	usmlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b[0]
	usmlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b[0]
	usmlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b[0]
	usmlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b[0]
	usmlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b[0]
	usmlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[15]
	usmlall	za.s[w9, 4:7], { z18.b - z19.b }, z9.b[12]

	usmlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[0]
	usmlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b[0]
	usmlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b[0]
	usmlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b[0]
	usmlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b[0]
	usmlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b[0]
	usmlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[15]
	usmlall	za.s[w10, 0:3], { z24.b - z27.b }, z14.b[6]

	usmlall	za.s[w8, 0:3], z0.b, z0.b
	usmlall	za.s[w11, 0:3], z0.b, z0.b
	usmlall	za.s[w8, 12:15], z0.b, z0.b
	usmlall	za.s[w8, 0:3], z31.b, z0.b
	usmlall	za.s[w8, 0:3], z0.b, z15.b
	usmlall	za.s[w10, 4:7], z25.b, z7.b

	usmlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b
	usmlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b
	usmlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b
	usmlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b
	usmlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b
	usmlall	za.s[w8, 0:3], { z31.b, z0.b }, z0.b
	usmlall	za.s[w8, 0:3], { z31.b - z0.b }, z0.b
	usmlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b
	usmlall	za.s[w9, 4:7], { z19.b - z20.b }, z13.b

	usmlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b
	usmlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b
	usmlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b
	usmlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b
	usmlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b
	usmlall	za.s[w8, 0:3], { z29.b - z0.b }, z0.b
	usmlall	za.s[w8, 0:3], { z30.b, z31.b, z0.b, z1.b }, z0.b
	usmlall	za.s[w8, 0:3], { z30.b - z1.b }, z0.b
	usmlall	za.s[w8, 0:3], { z31.b - z2.b }, z0.b
	usmlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b
	usmlall	za.s[w9, 0:3], { z25.b - z28.b }, z14.b

	usmlall	za.s[w8, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	usmlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	usmlall	za.s[w11, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	usmlall	za.s[w8, 4:7], { z0.b - z1.b }, { z0.b - z1.b }
	usmlall	za.s[w8, 0:3], { z30.b - z31.b }, { z0.b - z1.b }
	usmlall	za.s[w8, 0:3], { z0.b - z1.b }, { z30.b - z31.b }
	usmlall	za.s[w10, 4:7], { z22.b - z23.b }, { z18.b - z19.b }

	usmlall	za.s[w8, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	usmlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	usmlall	za.s[w11, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	usmlall	za.s[w8, 4:7], { z0.b - z3.b }, { z0.b - z3.b }
	usmlall	za.s[w8, 0:3], { z28.b - z31.b }, { z0.b - z3.b }
	usmlall	za.s[w8, 0:3], { z0.b - z3.b }, { z28.b - z31.b }
	usmlall	za.s[w11, 0:3], { z16.b - z19.b }, { z24.b - z27.b }
