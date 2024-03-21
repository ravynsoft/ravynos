	uzp	{ z0.b - z1.b }, z0.b, z0.b
	uzp	{ z30.b - z31.b }, z0.b, z0.b
	uzp	{ z0.b - z1.b }, z31.b, z0.b
	uzp	{ z0.b - z1.b }, z0.b, z31.b
	uzp	{ z18.b - z19.b }, z11.b, z25.b

	uzp	{ z0.h - z1.h }, z0.h, z0.h
	uzp	{ z30.h - z31.h }, z0.h, z0.h
	uzp	{ z0.h - z1.h }, z31.h, z0.h
	uzp	{ z0.h - z1.h }, z0.h, z31.h
	uzp	{ z6.h - z7.h }, z8.h, z22.h

	uzp	{ z0.s - z1.s }, z0.s, z0.s
	uzp	{ z30.s - z31.s }, z0.s, z0.s
	uzp	{ z0.s - z1.s }, z31.s, z0.s
	uzp	{ z0.s - z1.s }, z0.s, z31.s
	uzp	{ z24.s - z25.s }, z19.s, z2.s

	uzp	{ z0.d - z1.d }, z0.d, z0.d
	uzp	{ z30.d - z31.d }, z0.d, z0.d
	uzp	{ z0.d - z1.d }, z31.d, z0.d
	uzp	{ z0.d - z1.d }, z0.d, z31.d
	uzp	{ z2.d - z3.d }, z29.d, z5.d

	uzp	{ z0.q - z1.q }, z0.q, z0.q
	uzp	{ z30.q - z31.q }, z0.q, z0.q
	uzp	{ z0.q - z1.q }, z31.q, z0.q
	uzp	{ z0.q - z1.q }, z0.q, z31.q
	uzp	{ z14.q - z15.q }, z24.q, z9.q

	uzp	{ z0.b - z3.b }, { z0.b - z3.b }
	uzp	{ z28.b - z31.b }, { z0.b - z3.b }
	uzp	{ z0.b - z3.b }, { z28.b - z31.b }
	uzp	{ z4.b - z7.b }, { z24.b - z27.b }

	uzp	{ z0.h - z3.h }, { z0.h - z3.h }
	uzp	{ z28.h - z31.h }, { z0.h - z3.h }
	uzp	{ z0.h - z3.h }, { z28.h - z31.h }
	uzp	{ z16.h - z19.h }, { z8.h - z11.h }

	uzp	{ z0.s - z3.s }, { z0.s - z3.s }
	uzp	{ z28.s - z31.s }, { z0.s - z3.s }
	uzp	{ z0.s - z3.s }, { z28.s - z31.s }
	uzp	{ z20.s - z23.s }, { z12.s - z15.s }

	uzp	{ z0.d - z3.d }, { z0.d - z3.d }
	uzp	{ z28.d - z31.d }, { z0.d - z3.d }
	uzp	{ z0.d - z3.d }, { z28.d - z31.d }
	uzp	{ z8.d - z11.d }, { z16.d - z19.d }

	uzp	{ z0.q - z3.q }, { z0.q - z3.q }
	uzp	{ z28.q - z31.q }, { z0.q - z3.q }
	uzp	{ z0.q - z3.q }, { z28.q - z31.q }
	uzp	{ z12.q - z15.q }, { z4.q - z7.q }

	zip	{ z0.b - z1.b }, z0.b, z0.b
	zip	{ z30.b - z31.b }, z0.b, z0.b
	zip	{ z0.b - z1.b }, z31.b, z0.b
	zip	{ z0.b - z1.b }, z0.b, z31.b
	zip	{ z18.b - z19.b }, z11.b, z25.b

	zip	{ z0.h - z1.h }, z0.h, z0.h
	zip	{ z30.h - z31.h }, z0.h, z0.h
	zip	{ z0.h - z1.h }, z31.h, z0.h
	zip	{ z0.h - z1.h }, z0.h, z31.h
	zip	{ z6.h - z7.h }, z8.h, z22.h

	zip	{ z0.s - z1.s }, z0.s, z0.s
	zip	{ z30.s - z31.s }, z0.s, z0.s
	zip	{ z0.s - z1.s }, z31.s, z0.s
	zip	{ z0.s - z1.s }, z0.s, z31.s
	zip	{ z24.s - z25.s }, z19.s, z2.s

	zip	{ z0.d - z1.d }, z0.d, z0.d
	zip	{ z30.d - z31.d }, z0.d, z0.d
	zip	{ z0.d - z1.d }, z31.d, z0.d
	zip	{ z0.d - z1.d }, z0.d, z31.d
	zip	{ z2.d - z3.d }, z29.d, z5.d

	zip	{ z0.q - z1.q }, z0.q, z0.q
	zip	{ z30.q - z31.q }, z0.q, z0.q
	zip	{ z0.q - z1.q }, z31.q, z0.q
	zip	{ z0.q - z1.q }, z0.q, z31.q
	zip	{ z14.q - z15.q }, z24.q, z9.q

	zip	{ z0.b - z3.b }, { z0.b - z3.b }
	zip	{ z28.b - z31.b }, { z0.b - z3.b }
	zip	{ z0.b - z3.b }, { z28.b - z31.b }
	zip	{ z4.b - z7.b }, { z24.b - z27.b }

	zip	{ z0.h - z3.h }, { z0.h - z3.h }
	zip	{ z28.h - z31.h }, { z0.h - z3.h }
	zip	{ z0.h - z3.h }, { z28.h - z31.h }
	zip	{ z16.h - z19.h }, { z8.h - z11.h }

	zip	{ z0.s - z3.s }, { z0.s - z3.s }
	zip	{ z28.s - z31.s }, { z0.s - z3.s }
	zip	{ z0.s - z3.s }, { z28.s - z31.s }
	zip	{ z20.s - z23.s }, { z12.s - z15.s }

	zip	{ z0.d - z3.d }, { z0.d - z3.d }
	zip	{ z28.d - z31.d }, { z0.d - z3.d }
	zip	{ z0.d - z3.d }, { z28.d - z31.d }
	zip	{ z8.d - z11.d }, { z16.d - z19.d }

	zip	{ z0.q - z3.q }, { z0.q - z3.q }
	zip	{ z28.q - z31.q }, { z0.q - z3.q }
	zip	{ z0.q - z3.q }, { z28.q - z31.q }
	zip	{ z12.q - z15.q }, { z4.q - z7.q }
