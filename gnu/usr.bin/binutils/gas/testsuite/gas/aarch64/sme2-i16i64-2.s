	smlall	za.d[w8, 0:3], z0.h, z0.h[0]
	smlall	za.d[w11, 0:3], z0.h, z0.h[0]
	smlall	za.d[w8, 12:15], z0.h, z0.h[0]
	smlall	za.d[w8, 0:3], z31.h, z0.h[0]
	smlall	za.d[w8, 0:3], z0.h, z15.h[0]
	smlall	za.d[w8, 0:3], z0.h, z0.h[7]
	smlall	za.d[w9, 8:11], z21.h, z9.h[3]

	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w11, 0:3], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 4:7], { z0.h - z1.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z30.h - z31.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z15.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[7]
	smlall	za.d[w9, 4:7], { z18.h - z19.h }, z9.h[2]

	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w11, 0:3], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 4:7], { z0.h - z3.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z28.h - z31.h }, z0.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z15.h[0]
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[7]
	smlall	za.d[w10, 0:3], { z24.h - z27.h }, z14.h[6]

	smlall	za.d[w8, 0:3], z0.h, z0.h
	smlall	za.d[w11, 0:3], z0.h, z0.h
	smlall	za.d[w8, 12:15], z0.h, z0.h
	smlall	za.d[w8, 0:3], z31.h, z0.h
	smlall	za.d[w8, 0:3], z0.h, z15.h
	smlall	za.d[w10, 4:7], z25.h, z7.h

	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h
	smlall	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h
	smlall	za.d[w11, 0:3], { z0.h - z1.h }, z0.h
	smlall	za.d[w8, 4:7], { z0.h - z1.h }, z0.h
	smlall	za.d[w8, 0:3], { z30.h - z31.h }, z0.h
	smlall	za.d[w8, 0:3], { z31.h, z0.h }, z0.h
	smlall	za.d[w8, 0:3], { z31.h - z0.h }, z0.h
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, z15.h
	smlall	za.d[w9, 4:7], { z19.h - z20.h }, z13.h

	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h
	smlall	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h
	smlall	za.d[w11, 0:3], { z0.h - z3.h }, z0.h
	smlall	za.d[w8, 4:7], { z0.h - z3.h }, z0.h
	smlall	za.d[w8, 0:3], { z28.h - z31.h }, z0.h
	smlall	za.d[w8, 0:3], { z29.h - z0.h }, z0.h
	smlall	za.d[w8, 0:3], { z30.h, z31.h, z0.h, z1.h }, z0.h
	smlall	za.d[w8, 0:3], { z30.h - z1.h }, z0.h
	smlall	za.d[w8, 0:3], { z31.h - z2.h }, z0.h
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, z15.h
	smlall	za.d[w9, 0:3], { z25.h - z28.h }, z14.h

	smlall	za.d[w8, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	smlall	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	smlall	za.d[w11, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	smlall	za.d[w8, 4:7], { z0.h - z1.h }, { z0.h - z1.h }
	smlall	za.d[w8, 0:3], { z30.h - z31.h }, { z0.h - z1.h }
	smlall	za.d[w8, 0:3], { z0.h - z1.h }, { z30.h - z31.h }
	smlall	za.d[w10, 4:7], { z22.h - z23.h }, { z18.h - z19.h }

	smlall	za.d[w8, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	smlall	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	smlall	za.d[w11, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	smlall	za.d[w8, 4:7], { z0.h - z3.h }, { z0.h - z3.h }
	smlall	za.d[w8, 0:3], { z28.h - z31.h }, { z0.h - z3.h }
	smlall	za.d[w8, 0:3], { z0.h - z3.h }, { z28.h - z31.h }
	smlall	za.d[w11, 0:3], { z16.h - z19.h }, { z24.h - z27.h }

	smlsll	za.d[w8, 0:3], z0.h, z0.h[0]
	smlsll	za.d[w11, 0:3], z0.h, z0.h[0]
	smlsll	za.d[w8, 12:15], z0.h, z0.h[0]
	smlsll	za.d[w8, 0:3], z31.h, z0.h[0]
	smlsll	za.d[w8, 0:3], z0.h, z15.h[0]
	smlsll	za.d[w8, 0:3], z0.h, z0.h[7]
	smlsll	za.d[w9, 8:11], z21.h, z9.h[3]

	smlsll	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[0]
	smlsll	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h[0]
	smlsll	za.d[w11, 0:3], { z0.h - z1.h }, z0.h[0]
	smlsll	za.d[w8, 4:7], { z0.h - z1.h }, z0.h[0]
	smlsll	za.d[w8, 0:3], { z30.h - z31.h }, z0.h[0]
	smlsll	za.d[w8, 0:3], { z0.h - z1.h }, z15.h[0]
	smlsll	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[7]
	smlsll	za.d[w9, 4:7], { z18.h - z19.h }, z9.h[2]

	smlsll	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[0]
	smlsll	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h[0]
	smlsll	za.d[w11, 0:3], { z0.h - z3.h }, z0.h[0]
	smlsll	za.d[w8, 4:7], { z0.h - z3.h }, z0.h[0]
	smlsll	za.d[w8, 0:3], { z28.h - z31.h }, z0.h[0]
	smlsll	za.d[w8, 0:3], { z0.h - z3.h }, z15.h[0]
	smlsll	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[7]
	smlsll	za.d[w10, 0:3], { z24.h - z27.h }, z14.h[6]

	smlsll	za.d[w8, 0:3], z0.h, z0.h
	smlsll	za.d[w11, 0:3], z0.h, z0.h
	smlsll	za.d[w8, 12:15], z0.h, z0.h
	smlsll	za.d[w8, 0:3], z31.h, z0.h
	smlsll	za.d[w8, 0:3], z0.h, z15.h
	smlsll	za.d[w10, 4:7], z25.h, z7.h

	smlsll	za.d[w8, 0:3], { z0.h - z1.h }, z0.h
	smlsll	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h
	smlsll	za.d[w11, 0:3], { z0.h - z1.h }, z0.h
	smlsll	za.d[w8, 4:7], { z0.h - z1.h }, z0.h
	smlsll	za.d[w8, 0:3], { z30.h - z31.h }, z0.h
	smlsll	za.d[w8, 0:3], { z31.h, z0.h }, z0.h
	smlsll	za.d[w8, 0:3], { z31.h - z0.h }, z0.h
	smlsll	za.d[w8, 0:3], { z0.h - z1.h }, z15.h
	smlsll	za.d[w9, 4:7], { z19.h - z20.h }, z13.h

	smlsll	za.d[w8, 0:3], { z0.h - z3.h }, z0.h
	smlsll	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h
	smlsll	za.d[w11, 0:3], { z0.h - z3.h }, z0.h
	smlsll	za.d[w8, 4:7], { z0.h - z3.h }, z0.h
	smlsll	za.d[w8, 0:3], { z28.h - z31.h }, z0.h
	smlsll	za.d[w8, 0:3], { z29.h - z0.h }, z0.h
	smlsll	za.d[w8, 0:3], { z30.h, z31.h, z0.h, z1.h }, z0.h
	smlsll	za.d[w8, 0:3], { z30.h - z1.h }, z0.h
	smlsll	za.d[w8, 0:3], { z31.h - z2.h }, z0.h
	smlsll	za.d[w8, 0:3], { z0.h - z3.h }, z15.h
	smlsll	za.d[w9, 0:3], { z25.h - z28.h }, z14.h

	smlsll	za.d[w8, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	smlsll	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	smlsll	za.d[w11, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	smlsll	za.d[w8, 4:7], { z0.h - z1.h }, { z0.h - z1.h }
	smlsll	za.d[w8, 0:3], { z30.h - z31.h }, { z0.h - z1.h }
	smlsll	za.d[w8, 0:3], { z0.h - z1.h }, { z30.h - z31.h }
	smlsll	za.d[w10, 4:7], { z22.h - z23.h }, { z18.h - z19.h }

	smlsll	za.d[w8, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	smlsll	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	smlsll	za.d[w11, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	smlsll	za.d[w8, 4:7], { z0.h - z3.h }, { z0.h - z3.h }
	smlsll	za.d[w8, 0:3], { z28.h - z31.h }, { z0.h - z3.h }
	smlsll	za.d[w8, 0:3], { z0.h - z3.h }, { z28.h - z31.h }
	smlsll	za.d[w11, 0:3], { z16.h - z19.h }, { z24.h - z27.h }

	umlall	za.d[w8, 0:3], z0.h, z0.h[0]
	umlall	za.d[w11, 0:3], z0.h, z0.h[0]
	umlall	za.d[w8, 12:15], z0.h, z0.h[0]
	umlall	za.d[w8, 0:3], z31.h, z0.h[0]
	umlall	za.d[w8, 0:3], z0.h, z15.h[0]
	umlall	za.d[w8, 0:3], z0.h, z0.h[7]
	umlall	za.d[w9, 8:11], z21.h, z9.h[3]

	umlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[0]
	umlall	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h[0]
	umlall	za.d[w11, 0:3], { z0.h - z1.h }, z0.h[0]
	umlall	za.d[w8, 4:7], { z0.h - z1.h }, z0.h[0]
	umlall	za.d[w8, 0:3], { z30.h - z31.h }, z0.h[0]
	umlall	za.d[w8, 0:3], { z0.h - z1.h }, z15.h[0]
	umlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[7]
	umlall	za.d[w9, 4:7], { z18.h - z19.h }, z9.h[5]

	umlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[0]
	umlall	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h[0]
	umlall	za.d[w11, 0:3], { z0.h - z3.h }, z0.h[0]
	umlall	za.d[w8, 4:7], { z0.h - z3.h }, z0.h[0]
	umlall	za.d[w8, 0:3], { z28.h - z31.h }, z0.h[0]
	umlall	za.d[w8, 0:3], { z0.h - z3.h }, z15.h[0]
	umlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[7]
	umlall	za.d[w10, 0:3], { z24.h - z27.h }, z14.h[1]

	umlall	za.d[w8, 0:3], z0.h, z0.h
	umlall	za.d[w11, 0:3], z0.h, z0.h
	umlall	za.d[w8, 12:15], z0.h, z0.h
	umlall	za.d[w8, 0:3], z31.h, z0.h
	umlall	za.d[w8, 0:3], z0.h, z15.h
	umlall	za.d[w10, 4:7], z25.h, z7.h

	umlall	za.d[w8, 0:3], { z0.h - z1.h }, z0.h
	umlall	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h
	umlall	za.d[w11, 0:3], { z0.h - z1.h }, z0.h
	umlall	za.d[w8, 4:7], { z0.h - z1.h }, z0.h
	umlall	za.d[w8, 0:3], { z30.h - z31.h }, z0.h
	umlall	za.d[w8, 0:3], { z31.h, z0.h }, z0.h
	umlall	za.d[w8, 0:3], { z31.h - z0.h }, z0.h
	umlall	za.d[w8, 0:3], { z0.h - z1.h }, z15.h
	umlall	za.d[w9, 4:7], { z19.h - z20.h }, z13.h

	umlall	za.d[w8, 0:3], { z0.h - z3.h }, z0.h
	umlall	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h
	umlall	za.d[w11, 0:3], { z0.h - z3.h }, z0.h
	umlall	za.d[w8, 4:7], { z0.h - z3.h }, z0.h
	umlall	za.d[w8, 0:3], { z28.h - z31.h }, z0.h
	umlall	za.d[w8, 0:3], { z29.h - z0.h }, z0.h
	umlall	za.d[w8, 0:3], { z30.h, z31.h, z0.h, z1.h }, z0.h
	umlall	za.d[w8, 0:3], { z30.h - z1.h }, z0.h
	umlall	za.d[w8, 0:3], { z31.h - z2.h }, z0.h
	umlall	za.d[w8, 0:3], { z0.h - z3.h }, z15.h
	umlall	za.d[w9, 0:3], { z25.h - z28.h }, z14.h

	umlall	za.d[w8, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	umlall	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	umlall	za.d[w11, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	umlall	za.d[w8, 4:7], { z0.h - z1.h }, { z0.h - z1.h }
	umlall	za.d[w8, 0:3], { z30.h - z31.h }, { z0.h - z1.h }
	umlall	za.d[w8, 0:3], { z0.h - z1.h }, { z30.h - z31.h }
	umlall	za.d[w10, 4:7], { z22.h - z23.h }, { z18.h - z19.h }

	umlall	za.d[w8, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	umlall	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	umlall	za.d[w11, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	umlall	za.d[w8, 4:7], { z0.h - z3.h }, { z0.h - z3.h }
	umlall	za.d[w8, 0:3], { z28.h - z31.h }, { z0.h - z3.h }
	umlall	za.d[w8, 0:3], { z0.h - z3.h }, { z28.h - z31.h }
	umlall	za.d[w11, 0:3], { z16.h - z19.h }, { z24.h - z27.h }

	umlsll	za.d[w8, 0:3], z0.h, z0.h[0]
	umlsll	za.d[w11, 0:3], z0.h, z0.h[0]
	umlsll	za.d[w8, 12:15], z0.h, z0.h[0]
	umlsll	za.d[w8, 0:3], z31.h, z0.h[0]
	umlsll	za.d[w8, 0:3], z0.h, z15.h[0]
	umlsll	za.d[w8, 0:3], z0.h, z0.h[7]
	umlsll	za.d[w9, 8:11], z21.h, z9.h[3]

	umlsll	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[0]
	umlsll	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h[0]
	umlsll	za.d[w11, 0:3], { z0.h - z1.h }, z0.h[0]
	umlsll	za.d[w8, 4:7], { z0.h - z1.h }, z0.h[0]
	umlsll	za.d[w8, 0:3], { z30.h - z31.h }, z0.h[0]
	umlsll	za.d[w8, 0:3], { z0.h - z1.h }, z15.h[0]
	umlsll	za.d[w8, 0:3], { z0.h - z1.h }, z0.h[7]
	umlsll	za.d[w9, 4:7], { z18.h - z19.h }, z9.h[2]

	umlsll	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[0]
	umlsll	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h[0]
	umlsll	za.d[w11, 0:3], { z0.h - z3.h }, z0.h[0]
	umlsll	za.d[w8, 4:7], { z0.h - z3.h }, z0.h[0]
	umlsll	za.d[w8, 0:3], { z28.h - z31.h }, z0.h[0]
	umlsll	za.d[w8, 0:3], { z0.h - z3.h }, z15.h[0]
	umlsll	za.d[w8, 0:3], { z0.h - z3.h }, z0.h[7]
	umlsll	za.d[w10, 0:3], { z24.h - z27.h }, z14.h[6]

	umlsll	za.d[w8, 0:3], z0.h, z0.h
	umlsll	za.d[w11, 0:3], z0.h, z0.h
	umlsll	za.d[w8, 12:15], z0.h, z0.h
	umlsll	za.d[w8, 0:3], z31.h, z0.h
	umlsll	za.d[w8, 0:3], z0.h, z15.h
	umlsll	za.d[w10, 4:7], z25.h, z7.h

	umlsll	za.d[w8, 0:3], { z0.h - z1.h }, z0.h
	umlsll	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, z0.h
	umlsll	za.d[w11, 0:3], { z0.h - z1.h }, z0.h
	umlsll	za.d[w8, 4:7], { z0.h - z1.h }, z0.h
	umlsll	za.d[w8, 0:3], { z30.h - z31.h }, z0.h
	umlsll	za.d[w8, 0:3], { z31.h, z0.h }, z0.h
	umlsll	za.d[w8, 0:3], { z31.h - z0.h }, z0.h
	umlsll	za.d[w8, 0:3], { z0.h - z1.h }, z15.h
	umlsll	za.d[w9, 4:7], { z19.h - z20.h }, z13.h

	umlsll	za.d[w8, 0:3], { z0.h - z3.h }, z0.h
	umlsll	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, z0.h
	umlsll	za.d[w11, 0:3], { z0.h - z3.h }, z0.h
	umlsll	za.d[w8, 4:7], { z0.h - z3.h }, z0.h
	umlsll	za.d[w8, 0:3], { z28.h - z31.h }, z0.h
	umlsll	za.d[w8, 0:3], { z29.h - z0.h }, z0.h
	umlsll	za.d[w8, 0:3], { z30.h, z31.h, z0.h, z1.h }, z0.h
	umlsll	za.d[w8, 0:3], { z30.h - z1.h }, z0.h
	umlsll	za.d[w8, 0:3], { z31.h - z2.h }, z0.h
	umlsll	za.d[w8, 0:3], { z0.h - z3.h }, z15.h
	umlsll	za.d[w9, 0:3], { z25.h - z28.h }, z14.h

	umlsll	za.d[w8, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	umlsll	za.d[w8, 0:3, vgx2], { z0.h - z1.h }, { z0.h - z1.h }
	umlsll	za.d[w11, 0:3], { z0.h - z1.h }, { z0.h - z1.h }
	umlsll	za.d[w8, 4:7], { z0.h - z1.h }, { z0.h - z1.h }
	umlsll	za.d[w8, 0:3], { z30.h - z31.h }, { z0.h - z1.h }
	umlsll	za.d[w8, 0:3], { z0.h - z1.h }, { z30.h - z31.h }
	umlsll	za.d[w10, 4:7], { z22.h - z23.h }, { z18.h - z19.h }

	umlsll	za.d[w8, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	umlsll	za.d[w8, 0:3, vgx4], { z0.h - z3.h }, { z0.h - z3.h }
	umlsll	za.d[w11, 0:3], { z0.h - z3.h }, { z0.h - z3.h }
	umlsll	za.d[w8, 4:7], { z0.h - z3.h }, { z0.h - z3.h }
	umlsll	za.d[w8, 0:3], { z28.h - z31.h }, { z0.h - z3.h }
	umlsll	za.d[w8, 0:3], { z0.h - z3.h }, { z28.h - z31.h }
	umlsll	za.d[w11, 0:3], { z16.h - z19.h }, { z24.h - z27.h }
