	smlall	za.s[w8, 0:3], z0.b, z0.b[0]
	smlall	za.s[w11, 0:3], z0.b, z0.b[0]
	smlall	za.s[w8, 12:15], z0.b, z0.b[0]
	smlall	za.s[w8, 0:3], z31.b, z0.b[0]
	smlall	za.s[w8, 0:3], z0.b, z15.b[0]
	smlall	za.s[w8, 0:3], z0.b, z0.b[15]
	smlall	za.s[w9, 8:11], z21.b, z9.b[9]

	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[15]
	smlall	za.s[w9, 4:7], { z18.b - z19.b }, z9.b[12]

	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b[0]
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[15]
	smlall	za.s[w10, 0:3], { z24.b - z27.b }, z14.b[6]

	smlall	za.s[w8, 0:3], z0.b, z0.b
	smlall	za.s[w11, 0:3], z0.b, z0.b
	smlall	za.s[w8, 12:15], z0.b, z0.b
	smlall	za.s[w8, 0:3], z31.b, z0.b
	smlall	za.s[w8, 0:3], z0.b, z15.b
	smlall	za.s[w10, 4:7], z25.b, z7.b

	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b
	smlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b
	smlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b
	smlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b
	smlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b
	smlall	za.s[w8, 0:3], { z31.b, z0.b }, z0.b
	smlall	za.s[w8, 0:3], { z31.b - z0.b }, z0.b
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b
	smlall	za.s[w9, 4:7], { z19.b - z20.b }, z13.b

	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b
	smlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b
	smlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b
	smlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b
	smlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b
	smlall	za.s[w8, 0:3], { z29.b - z0.b }, z0.b
	smlall	za.s[w8, 0:3], { z30.b, z31.b, z0.b, z1.b }, z0.b
	smlall	za.s[w8, 0:3], { z30.b - z1.b }, z0.b
	smlall	za.s[w8, 0:3], { z31.b - z2.b }, z0.b
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b
	smlall	za.s[w9, 0:3], { z25.b - z28.b }, z14.b

	smlall	za.s[w8, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	smlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	smlall	za.s[w11, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	smlall	za.s[w8, 4:7], { z0.b - z1.b }, { z0.b - z1.b }
	smlall	za.s[w8, 0:3], { z30.b - z31.b }, { z0.b - z1.b }
	smlall	za.s[w8, 0:3], { z0.b - z1.b }, { z30.b - z31.b }
	smlall	za.s[w10, 4:7], { z22.b - z23.b }, { z18.b - z19.b }

	smlall	za.s[w8, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	smlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	smlall	za.s[w11, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	smlall	za.s[w8, 4:7], { z0.b - z3.b }, { z0.b - z3.b }
	smlall	za.s[w8, 0:3], { z28.b - z31.b }, { z0.b - z3.b }
	smlall	za.s[w8, 0:3], { z0.b - z3.b }, { z28.b - z31.b }
	smlall	za.s[w11, 0:3], { z16.b - z19.b }, { z24.b - z27.b }

	smlsll	za.s[w8, 0:3], z0.b, z0.b[0]
	smlsll	za.s[w11, 0:3], z0.b, z0.b[0]
	smlsll	za.s[w8, 12:15], z0.b, z0.b[0]
	smlsll	za.s[w8, 0:3], z31.b, z0.b[0]
	smlsll	za.s[w8, 0:3], z0.b, z15.b[0]
	smlsll	za.s[w8, 0:3], z0.b, z0.b[15]
	smlsll	za.s[w9, 8:11], z21.b, z9.b[9]

	smlsll	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[0]
	smlsll	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b[0]
	smlsll	za.s[w11, 0:3], { z0.b - z1.b }, z0.b[0]
	smlsll	za.s[w8, 4:7], { z0.b - z1.b }, z0.b[0]
	smlsll	za.s[w8, 0:3], { z30.b - z31.b }, z0.b[0]
	smlsll	za.s[w8, 0:3], { z0.b - z1.b }, z15.b[0]
	smlsll	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[15]
	smlsll	za.s[w9, 4:7], { z18.b - z19.b }, z9.b[12]

	smlsll	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[0]
	smlsll	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b[0]
	smlsll	za.s[w11, 0:3], { z0.b - z3.b }, z0.b[0]
	smlsll	za.s[w8, 4:7], { z0.b - z3.b }, z0.b[0]
	smlsll	za.s[w8, 0:3], { z28.b - z31.b }, z0.b[0]
	smlsll	za.s[w8, 0:3], { z0.b - z3.b }, z15.b[0]
	smlsll	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[15]
	smlsll	za.s[w10, 0:3], { z24.b - z27.b }, z14.b[6]

	smlsll	za.s[w8, 0:3], z0.b, z0.b
	smlsll	za.s[w11, 0:3], z0.b, z0.b
	smlsll	za.s[w8, 12:15], z0.b, z0.b
	smlsll	za.s[w8, 0:3], z31.b, z0.b
	smlsll	za.s[w8, 0:3], z0.b, z15.b
	smlsll	za.s[w10, 4:7], z25.b, z7.b

	smlsll	za.s[w8, 0:3], { z0.b - z1.b }, z0.b
	smlsll	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b
	smlsll	za.s[w11, 0:3], { z0.b - z1.b }, z0.b
	smlsll	za.s[w8, 4:7], { z0.b - z1.b }, z0.b
	smlsll	za.s[w8, 0:3], { z30.b - z31.b }, z0.b
	smlsll	za.s[w8, 0:3], { z31.b, z0.b }, z0.b
	smlsll	za.s[w8, 0:3], { z31.b - z0.b }, z0.b
	smlsll	za.s[w8, 0:3], { z0.b - z1.b }, z15.b
	smlsll	za.s[w9, 4:7], { z19.b - z20.b }, z13.b

	smlsll	za.s[w8, 0:3], { z0.b - z3.b }, z0.b
	smlsll	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b
	smlsll	za.s[w11, 0:3], { z0.b - z3.b }, z0.b
	smlsll	za.s[w8, 4:7], { z0.b - z3.b }, z0.b
	smlsll	za.s[w8, 0:3], { z28.b - z31.b }, z0.b
	smlsll	za.s[w8, 0:3], { z29.b - z0.b }, z0.b
	smlsll	za.s[w8, 0:3], { z30.b, z31.b, z0.b, z1.b }, z0.b
	smlsll	za.s[w8, 0:3], { z30.b - z1.b }, z0.b
	smlsll	za.s[w8, 0:3], { z31.b - z2.b }, z0.b
	smlsll	za.s[w8, 0:3], { z0.b - z3.b }, z15.b
	smlsll	za.s[w9, 0:3], { z25.b - z28.b }, z14.b

	smlsll	za.s[w8, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	smlsll	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	smlsll	za.s[w11, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	smlsll	za.s[w8, 4:7], { z0.b - z1.b }, { z0.b - z1.b }
	smlsll	za.s[w8, 0:3], { z30.b - z31.b }, { z0.b - z1.b }
	smlsll	za.s[w8, 0:3], { z0.b - z1.b }, { z30.b - z31.b }
	smlsll	za.s[w10, 4:7], { z22.b - z23.b }, { z18.b - z19.b }

	smlsll	za.s[w8, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	smlsll	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	smlsll	za.s[w11, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	smlsll	za.s[w8, 4:7], { z0.b - z3.b }, { z0.b - z3.b }
	smlsll	za.s[w8, 0:3], { z28.b - z31.b }, { z0.b - z3.b }
	smlsll	za.s[w8, 0:3], { z0.b - z3.b }, { z28.b - z31.b }
	smlsll	za.s[w11, 0:3], { z16.b - z19.b }, { z24.b - z27.b }

	umlall	za.s[w8, 0:3], z0.b, z0.b[0]
	umlall	za.s[w11, 0:3], z0.b, z0.b[0]
	umlall	za.s[w8, 12:15], z0.b, z0.b[0]
	umlall	za.s[w8, 0:3], z31.b, z0.b[0]
	umlall	za.s[w8, 0:3], z0.b, z15.b[0]
	umlall	za.s[w8, 0:3], z0.b, z0.b[15]
	umlall	za.s[w9, 8:11], z21.b, z9.b[9]

	umlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[0]
	umlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b[0]
	umlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b[0]
	umlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b[0]
	umlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b[0]
	umlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b[0]
	umlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[15]
	umlall	za.s[w9, 4:7], { z18.b - z19.b }, z9.b[12]

	umlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[0]
	umlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b[0]
	umlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b[0]
	umlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b[0]
	umlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b[0]
	umlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b[0]
	umlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[15]
	umlall	za.s[w10, 0:3], { z24.b - z27.b }, z14.b[6]

	umlall	za.s[w8, 0:3], z0.b, z0.b
	umlall	za.s[w11, 0:3], z0.b, z0.b
	umlall	za.s[w8, 12:15], z0.b, z0.b
	umlall	za.s[w8, 0:3], z31.b, z0.b
	umlall	za.s[w8, 0:3], z0.b, z15.b
	umlall	za.s[w10, 4:7], z25.b, z7.b

	umlall	za.s[w8, 0:3], { z0.b - z1.b }, z0.b
	umlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b
	umlall	za.s[w11, 0:3], { z0.b - z1.b }, z0.b
	umlall	za.s[w8, 4:7], { z0.b - z1.b }, z0.b
	umlall	za.s[w8, 0:3], { z30.b - z31.b }, z0.b
	umlall	za.s[w8, 0:3], { z31.b, z0.b }, z0.b
	umlall	za.s[w8, 0:3], { z31.b - z0.b }, z0.b
	umlall	za.s[w8, 0:3], { z0.b - z1.b }, z15.b
	umlall	za.s[w9, 4:7], { z19.b - z20.b }, z13.b

	umlall	za.s[w8, 0:3], { z0.b - z3.b }, z0.b
	umlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b
	umlall	za.s[w11, 0:3], { z0.b - z3.b }, z0.b
	umlall	za.s[w8, 4:7], { z0.b - z3.b }, z0.b
	umlall	za.s[w8, 0:3], { z28.b - z31.b }, z0.b
	umlall	za.s[w8, 0:3], { z29.b - z0.b }, z0.b
	umlall	za.s[w8, 0:3], { z30.b, z31.b, z0.b, z1.b }, z0.b
	umlall	za.s[w8, 0:3], { z30.b - z1.b }, z0.b
	umlall	za.s[w8, 0:3], { z31.b - z2.b }, z0.b
	umlall	za.s[w8, 0:3], { z0.b - z3.b }, z15.b
	umlall	za.s[w9, 0:3], { z25.b - z28.b }, z14.b

	umlall	za.s[w8, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	umlall	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	umlall	za.s[w11, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	umlall	za.s[w8, 4:7], { z0.b - z1.b }, { z0.b - z1.b }
	umlall	za.s[w8, 0:3], { z30.b - z31.b }, { z0.b - z1.b }
	umlall	za.s[w8, 0:3], { z0.b - z1.b }, { z30.b - z31.b }
	umlall	za.s[w10, 4:7], { z22.b - z23.b }, { z18.b - z19.b }

	umlall	za.s[w8, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	umlall	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	umlall	za.s[w11, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	umlall	za.s[w8, 4:7], { z0.b - z3.b }, { z0.b - z3.b }
	umlall	za.s[w8, 0:3], { z28.b - z31.b }, { z0.b - z3.b }
	umlall	za.s[w8, 0:3], { z0.b - z3.b }, { z28.b - z31.b }
	umlall	za.s[w11, 0:3], { z16.b - z19.b }, { z24.b - z27.b }

	umlsll	za.s[w8, 0:3], z0.b, z0.b[0]
	umlsll	za.s[w11, 0:3], z0.b, z0.b[0]
	umlsll	za.s[w8, 12:15], z0.b, z0.b[0]
	umlsll	za.s[w8, 0:3], z31.b, z0.b[0]
	umlsll	za.s[w8, 0:3], z0.b, z15.b[0]
	umlsll	za.s[w8, 0:3], z0.b, z0.b[15]
	umlsll	za.s[w9, 8:11], z21.b, z9.b[9]

	umlsll	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[0]
	umlsll	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b[0]
	umlsll	za.s[w11, 0:3], { z0.b - z1.b }, z0.b[0]
	umlsll	za.s[w8, 4:7], { z0.b - z1.b }, z0.b[0]
	umlsll	za.s[w8, 0:3], { z30.b - z31.b }, z0.b[0]
	umlsll	za.s[w8, 0:3], { z0.b - z1.b }, z15.b[0]
	umlsll	za.s[w8, 0:3], { z0.b - z1.b }, z0.b[15]
	umlsll	za.s[w9, 4:7], { z18.b - z19.b }, z9.b[12]

	umlsll	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[0]
	umlsll	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b[0]
	umlsll	za.s[w11, 0:3], { z0.b - z3.b }, z0.b[0]
	umlsll	za.s[w8, 4:7], { z0.b - z3.b }, z0.b[0]
	umlsll	za.s[w8, 0:3], { z28.b - z31.b }, z0.b[0]
	umlsll	za.s[w8, 0:3], { z0.b - z3.b }, z15.b[0]
	umlsll	za.s[w8, 0:3], { z0.b - z3.b }, z0.b[15]
	umlsll	za.s[w10, 0:3], { z24.b - z27.b }, z14.b[6]

	umlsll	za.s[w8, 0:3], z0.b, z0.b
	umlsll	za.s[w11, 0:3], z0.b, z0.b
	umlsll	za.s[w8, 12:15], z0.b, z0.b
	umlsll	za.s[w8, 0:3], z31.b, z0.b
	umlsll	za.s[w8, 0:3], z0.b, z15.b
	umlsll	za.s[w10, 4:7], z25.b, z7.b

	umlsll	za.s[w8, 0:3], { z0.b - z1.b }, z0.b
	umlsll	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, z0.b
	umlsll	za.s[w11, 0:3], { z0.b - z1.b }, z0.b
	umlsll	za.s[w8, 4:7], { z0.b - z1.b }, z0.b
	umlsll	za.s[w8, 0:3], { z30.b - z31.b }, z0.b
	umlsll	za.s[w8, 0:3], { z31.b, z0.b }, z0.b
	umlsll	za.s[w8, 0:3], { z31.b - z0.b }, z0.b
	umlsll	za.s[w8, 0:3], { z0.b - z1.b }, z15.b
	umlsll	za.s[w9, 4:7], { z19.b - z20.b }, z13.b

	umlsll	za.s[w8, 0:3], { z0.b - z3.b }, z0.b
	umlsll	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, z0.b
	umlsll	za.s[w11, 0:3], { z0.b - z3.b }, z0.b
	umlsll	za.s[w8, 4:7], { z0.b - z3.b }, z0.b
	umlsll	za.s[w8, 0:3], { z28.b - z31.b }, z0.b
	umlsll	za.s[w8, 0:3], { z29.b - z0.b }, z0.b
	umlsll	za.s[w8, 0:3], { z30.b, z31.b, z0.b, z1.b }, z0.b
	umlsll	za.s[w8, 0:3], { z30.b - z1.b }, z0.b
	umlsll	za.s[w8, 0:3], { z31.b - z2.b }, z0.b
	umlsll	za.s[w8, 0:3], { z0.b - z3.b }, z15.b
	umlsll	za.s[w9, 0:3], { z25.b - z28.b }, z14.b

	umlsll	za.s[w8, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	umlsll	za.s[w8, 0:3, vgx2], { z0.b - z1.b }, { z0.b - z1.b }
	umlsll	za.s[w11, 0:3], { z0.b - z1.b }, { z0.b - z1.b }
	umlsll	za.s[w8, 4:7], { z0.b - z1.b }, { z0.b - z1.b }
	umlsll	za.s[w8, 0:3], { z30.b - z31.b }, { z0.b - z1.b }
	umlsll	za.s[w8, 0:3], { z0.b - z1.b }, { z30.b - z31.b }
	umlsll	za.s[w10, 4:7], { z22.b - z23.b }, { z18.b - z19.b }

	umlsll	za.s[w8, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	umlsll	za.s[w8, 0:3, vgx4], { z0.b - z3.b }, { z0.b - z3.b }
	umlsll	za.s[w11, 0:3], { z0.b - z3.b }, { z0.b - z3.b }
	umlsll	za.s[w8, 4:7], { z0.b - z3.b }, { z0.b - z3.b }
	umlsll	za.s[w8, 0:3], { z28.b - z31.b }, { z0.b - z3.b }
	umlsll	za.s[w8, 0:3], { z0.b - z3.b }, { z28.b - z31.b }
	umlsll	za.s[w11, 0:3], { z16.b - z19.b }, { z24.b - z27.b }
