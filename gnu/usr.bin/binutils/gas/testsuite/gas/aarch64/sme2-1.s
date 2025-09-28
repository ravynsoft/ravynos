	mov	{ z0.b - z1.b }, za.b[w8, 0]
	mov	{ z0.h - z1.h }, za.h[w8, 0]
	mov	{ z0.s - z1.s }, za.s[w8, 0]
	mov	{ z0.d - z1.d }, za.d[w8, 0]
	mov	{ z30.d - z31.d }, za.d[w8, 0]
	mov	{ z0.d - z1.d }, za.d[w11, 0]
	mov	{ z0.d - z1.d }, za.d[w8, 7]

	mov	{ z0.b - z3.b }, za.b[w8, 0]
	mov	{ z0.h - z3.h }, za.h[w8, 0]
	mov	{ z0.s - z3.s }, za.s[w8, 0]
	mov	{ z0.d - z3.d }, za.d[w8, 0]
	mov	{ z28.d - z31.d }, za.d[w8, 0]
	mov	{ z0.d - z3.d }, za.d[w11, 0]
	mov	{ z0.d - z3.d }, za.d[w8, 7]

	mov	{ z0.b - z1.b }, za0h.b[w12, 0:1]
	mov	{ z30.b - z31.b }, za0h.b[w12, 0:1]
	mov	{ z0.b - z1.b }, za0v.b[w12, 0:1]
	mov	{ z0.b - z1.b }, za0h.b[w15, 0:1]
	mov	{ z0.b - z1.b }, za0h.b[w12, 14:15]
	mov	{ z8.b - z9.b }, za0h.b[w14, 6:7]

	mov	{ z0.h - z1.h }, za0h.h[w12, 0:1]
	mov	{ z30.h - z31.h }, za0h.h[w12, 0:1]
	mov	{ z0.h - z1.h }, za0v.h[w12, 0:1]
	mov	{ z0.h - z1.h }, za1h.h[w12, 0:1]
	mov	{ z0.h - z1.h }, za1v.h[w12, 0:1]
	mov	{ z0.h - z1.h }, za0h.h[w15, 0:1]
	mov	{ z0.h - z1.h }, za0h.h[w12, 6:7]
	mov	{ z10.h - z11.h }, za0h.h[w13, 2:3]

	mov	{ z0.s - z1.s }, za0h.s[w12, 0:1]
	mov	{ z30.s - z31.s }, za0h.s[w12, 0:1]
	mov	{ z0.s - z1.s }, za0v.s[w12, 0:1]
	mov	{ z0.s - z1.s }, za3h.s[w12, 0:1]
	mov	{ z0.s - z1.s }, za3v.s[w12, 0:1]
	mov	{ z0.s - z1.s }, za0h.s[w15, 0:1]
	mov	{ z0.s - z1.s }, za0h.s[w12, 2:3]
	mov	{ z18.s - z19.s }, za2h.s[w14, 0:1]

	mov	{ z0.d - z1.d }, za0h.d[w12, 0:1]
	mov	{ z30.d - z31.d }, za0h.d[w12, 0:1]
	mov	{ z0.d - z1.d }, za0v.d[w12, 0:1]
	mov	{ z0.d - z1.d }, za7h.d[w12, 0:1]
	mov	{ z0.d - z1.d }, za7v.d[w12, 0:1]
	mov	{ z0.d - z1.d }, za0h.d[w15, 0:1]
	mov	{ z22.d - z23.d }, za6h.d[w13, 0:1]

	mov	{ z0.b - z3.b }, za0h.b[w12, 0:3]
	mov	{ z28.b - z31.b }, za0h.b[w12, 0:3]
	mov	{ z0.b - z3.b }, za0v.b[w12, 0:3]
	mov	{ z0.b - z3.b }, za0h.b[w15, 0:3]
	mov	{ z0.b - z3.b }, za0h.b[w12, 12:15]
	mov	{ z12.b - z15.b }, za0h.b[w14, 8:11]

	mov	{ z0.h - z3.h }, za0h.h[w12, 0:3]
	mov	{ z28.h - z31.h }, za0h.h[w12, 0:3]
	mov	{ z0.h - z3.h }, za0v.h[w12, 0:3]
	mov	{ z0.h - z3.h }, za1h.h[w12, 0:3]
	mov	{ z0.h - z3.h }, za1v.h[w12, 0:3]
	mov	{ z0.h - z3.h }, za0h.h[w15, 0:3]
	mov	{ z0.h - z3.h }, za0h.h[w12, 4:7]
	mov	{ z16.h - z19.h }, za0h.h[w13, 4:7]

	mov	{ z0.s - z3.s }, za0h.s[w12, 0:3]
	mov	{ z28.s - z31.s }, za0h.s[w12, 0:3]
	mov	{ z0.s - z3.s }, za0v.s[w12, 0:3]
	mov	{ z0.s - z3.s }, za3h.s[w12, 0:3]
	mov	{ z0.s - z3.s }, za3v.s[w12, 0:3]
	mov	{ z0.s - z3.s }, za0h.s[w15, 0:3]
	mov	{ z20.s - z23.s }, za2h.s[w13, 0:3]

	mov	{ z0.d - z3.d }, za0h.d[w12, 0:3]
	mov	{ z28.d - z31.d }, za0h.d[w12, 0:3]
	mov	{ z0.d - z3.d }, za0v.d[w12, 0:3]
	mov	{ z0.d - z3.d }, za7h.d[w12, 0:3]
	mov	{ z0.d - z3.d }, za7v.d[w12, 0:3]
	mov	{ z0.d - z3.d }, za0h.d[w15, 0:3]
	mov	{ z24.d - z27.d }, za5h.d[w13, 0:3]

	// Invalid MOVAs
	.inst	0xc0060480
	.inst	0xc00604e0
	.inst	0xc0460480
	.inst	0xc04604e0
	.inst	0xc0860480
	.inst	0xc08604e0
	// Valid MOVAs
	.inst	0xc0c60480
	.inst	0xc0c604e0

	mov	za.b[w8, 0], { z0.b - z1.b }
	mov	za.h[w8, 0], { z0.h - z1.h }
	mov	za.s[w8, 0], { z0.s - z1.s }
	mov	za.d[w8, 0], { z0.d - z1.d }
	mov	za.d[w11, 0], { z0.d - z1.d }
	mov	za.d[w8, 7], { z0.d - z1.d }
	mov	za.d[w8, 0], { z30.d - z31.d }
	mov	za.d[w9, 5], { z2.d - z3.d }

	mov	za.b[w8, 0], { z0.b - z3.b }
	mov	za.h[w8, 0], { z0.h - z3.h }
	mov	za.s[w8, 0], { z0.s - z3.s }
	mov	za.d[w8, 0], { z0.d - z3.d }
	mov	za.d[w11, 0], { z0.d - z3.d }
	mov	za.d[w8, 7], { z0.d - z3.d }
	mov	za.d[w8, 0], { z28.d - z31.d }
	mov	za.d[w10, 1], { z20.d - z23.d }

	mov	za0h.b[w12, 0:1], { z0.b - z1.b }
	mov	za0v.b[w12, 0:1], { z0.b - z1.b }
	mov	za0h.b[w15, 0:1], { z0.b - z1.b }
	mov	za0h.b[w12, 14:15], { z0.b - z1.b }
	mov	za0h.b[w12, 0:1], { z30.b - z31.b }
	mov	za0h.b[w14, 6:7], { z8.b - z9.b }

	mov	za0h.h[w12, 0:1], { z0.h - z1.h }
	mov	za0v.h[w12, 0:1], { z0.h - z1.h }
	mov	za1h.h[w12, 0:1], { z0.h - z1.h }
	mov	za1v.h[w12, 0:1], { z0.h - z1.h }
	mov	za0h.h[w15, 0:1], { z0.h - z1.h }
	mov	za0h.h[w12, 6:7], { z0.h - z1.h }
	mov	za0h.h[w12, 0:1], { z30.h - z31.h }
	mov	za0h.h[w13, 2:3], { z10.h - z11.h }

	mov	za0h.s[w12, 0:1], { z0.s - z1.s }
	mov	za0v.s[w12, 0:1], { z0.s - z1.s }
	mov	za3h.s[w12, 0:1], { z0.s - z1.s }
	mov	za3v.s[w12, 0:1], { z0.s - z1.s }
	mov	za0h.s[w15, 0:1], { z0.s - z1.s }
	mov	za0h.s[w12, 2:3], { z0.s - z1.s }
	mov	za0h.s[w12, 0:1], { z30.s - z31.s }
	mov	za2h.s[w14, 0:1], { z18.s - z19.s }

	mov	za0h.d[w12, 0:1], { z0.d - z1.d }
	mov	za0v.d[w12, 0:1], { z0.d - z1.d }
	mov	za7h.d[w12, 0:1], { z0.d - z1.d }
	mov	za7v.d[w12, 0:1], { z0.d - z1.d }
	mov	za0h.d[w15, 0:1], { z0.d - z1.d }
	mov	za0h.d[w12, 0:1], { z30.d - z31.d }
	mov	za6h.d[w13, 0:1], { z22.d - z23.d }

	mov	za0h.b[w12, 0:3], { z0.b - z3.b }
	mov	za0v.b[w12, 0:3], { z0.b - z3.b }
	mov	za0h.b[w15, 0:3], { z0.b - z3.b }
	mov	za0h.b[w12, 12:15], { z0.b - z3.b }
	mov	za0h.b[w12, 0:3], { z28.b - z31.b }
	mov	za0h.b[w14, 8:11], { z12.b - z15.b }

	mov	za0h.h[w12, 0:3], { z0.h - z3.h }
	mov	za0v.h[w12, 0:3], { z0.h - z3.h }
	mov	za1h.h[w12, 0:3], { z0.h - z3.h }
	mov	za1v.h[w12, 0:3], { z0.h - z3.h }
	mov	za0h.h[w15, 0:3], { z0.h - z3.h }
	mov	za0h.h[w12, 4:7], { z0.h - z3.h }
	mov	za0h.h[w12, 0:3], { z28.h - z31.h }
	mov	za0h.h[w13, 4:7], { z16.h - z19.h }

	mov	za0h.s[w12, 0:3], { z0.s - z3.s }
	mov	za0v.s[w12, 0:3], { z0.s - z3.s }
	mov	za3h.s[w12, 0:3], { z0.s - z3.s }
	mov	za3v.s[w12, 0:3], { z0.s - z3.s }
	mov	za0h.s[w15, 0:3], { z0.s - z3.s }
	mov	za0h.s[w12, 0:3], { z28.s - z31.s }
	mov	za2h.s[w13, 0:3], { z20.s - z23.s }

	mov	za0h.d[w12, 0:3], { z0.d - z3.d }
	mov	za0v.d[w12, 0:3], { z0.d - z3.d }
	mov	za7h.d[w12, 0:3], { z0.d - z3.d }
	mov	za7v.d[w12, 0:3], { z0.d - z3.d }
	mov	za0h.d[w15, 0:3], { z0.d - z3.d }
	mov	za0h.d[w12, 0:3], { z28.d - z31.d }
	mov	za5h.d[w13, 0:3], { z24.d - z27.d }

	mova	{ z0.b - z1.b }, za.b[w8, 0]
	mova	{ z0.h - z1.h }, za.h[w8, 0]
	mova	{ z0.s - z1.s }, za.s[w8, 0]
	mova	{ z0.d - z1.d }, za.d[w8, 0]
	mova	{ z30.d - z31.d }, za.d[w8, 0]
	mova	{ z0.d - z1.d }, za.d[w11, 0]
	mova	{ z0.d - z1.d }, za.d[w8, 7]

	mova	{ z0.b - z3.b }, za.b[w8, 0]
	mova	{ z0.h - z3.h }, za.h[w8, 0]
	mova	{ z0.s - z3.s }, za.s[w8, 0]
	mova	{ z0.d - z3.d }, za.d[w8, 0]
	mova	{ z28.d - z31.d }, za.d[w8, 0]
	mova	{ z0.d - z3.d }, za.d[w11, 0]
	mova	{ z0.d - z3.d }, za.d[w8, 7]

	mova	{ z0.b - z1.b }, za0h.b[w12, 0:1]
	mova	{ z30.b - z31.b }, za0h.b[w12, 0:1]
	mova	{ z0.b - z1.b }, za0v.b[w12, 0:1]
	mova	{ z0.b - z1.b }, za0h.b[w15, 0:1]
	mova	{ z0.b - z1.b }, za0h.b[w12, 14:15]
	mova	{ z8.b - z9.b }, za0h.b[w14, 6:7]

	mova	{ z0.h - z1.h }, za0h.h[w12, 0:1]
	mova	{ z30.h - z31.h }, za0h.h[w12, 0:1]
	mova	{ z0.h - z1.h }, za0v.h[w12, 0:1]
	mova	{ z0.h - z1.h }, za1h.h[w12, 0:1]
	mova	{ z0.h - z1.h }, za1v.h[w12, 0:1]
	mova	{ z0.h - z1.h }, za0h.h[w15, 0:1]
	mova	{ z0.h - z1.h }, za0h.h[w12, 6:7]
	mova	{ z10.h - z11.h }, za0h.h[w13, 2:3]

	mova	{ z0.s - z1.s }, za0h.s[w12, 0:1]
	mova	{ z30.s - z31.s }, za0h.s[w12, 0:1]
	mova	{ z0.s - z1.s }, za0v.s[w12, 0:1]
	mova	{ z0.s - z1.s }, za3h.s[w12, 0:1]
	mova	{ z0.s - z1.s }, za3v.s[w12, 0:1]
	mova	{ z0.s - z1.s }, za0h.s[w15, 0:1]
	mova	{ z0.s - z1.s }, za0h.s[w12, 2:3]
	mova	{ z18.s - z19.s }, za2h.s[w14, 0:1]

	mova	{ z0.d - z1.d }, za0h.d[w12, 0:1]
	mova	{ z30.d - z31.d }, za0h.d[w12, 0:1]
	mova	{ z0.d - z1.d }, za0v.d[w12, 0:1]
	mova	{ z0.d - z1.d }, za7h.d[w12, 0:1]
	mova	{ z0.d - z1.d }, za7v.d[w12, 0:1]
	mova	{ z0.d - z1.d }, za0h.d[w15, 0:1]
	mova	{ z22.d - z23.d }, za6h.d[w13, 0:1]

	mova	{ z0.b - z3.b }, za0h.b[w12, 0:3]
	mova	{ z28.b - z31.b }, za0h.b[w12, 0:3]
	mova	{ z0.b - z3.b }, za0v.b[w12, 0:3]
	mova	{ z0.b - z3.b }, za0h.b[w15, 0:3]
	mova	{ z0.b - z3.b }, za0h.b[w12, 12:15]
	mova	{ z12.b - z15.b }, za0h.b[w14, 8:11]

	mova	{ z0.h - z3.h }, za0h.h[w12, 0:3]
	mova	{ z28.h - z31.h }, za0h.h[w12, 0:3]
	mova	{ z0.h - z3.h }, za0v.h[w12, 0:3]
	mova	{ z0.h - z3.h }, za1h.h[w12, 0:3]
	mova	{ z0.h - z3.h }, za1v.h[w12, 0:3]
	mova	{ z0.h - z3.h }, za0h.h[w15, 0:3]
	mova	{ z0.h - z3.h }, za0h.h[w12, 4:7]
	mova	{ z16.h - z19.h }, za0h.h[w13, 4:7]

	mova	{ z0.s - z3.s }, za0h.s[w12, 0:3]
	mova	{ z28.s - z31.s }, za0h.s[w12, 0:3]
	mova	{ z0.s - z3.s }, za0v.s[w12, 0:3]
	mova	{ z0.s - z3.s }, za3h.s[w12, 0:3]
	mova	{ z0.s - z3.s }, za3v.s[w12, 0:3]
	mova	{ z0.s - z3.s }, za0h.s[w15, 0:3]
	mova	{ z20.s - z23.s }, za2h.s[w13, 0:3]

	mova	{ z0.d - z3.d }, za0h.d[w12, 0:3]
	mova	{ z28.d - z31.d }, za0h.d[w12, 0:3]
	mova	{ z0.d - z3.d }, za0v.d[w12, 0:3]
	mova	{ z0.d - z3.d }, za7h.d[w12, 0:3]
	mova	{ z0.d - z3.d }, za7v.d[w12, 0:3]
	mova	{ z0.d - z3.d }, za0h.d[w15, 0:3]
	mova	{ z24.d - z27.d }, za5h.d[w13, 0:3]

	mova	za.b[w8, 0], { z0.b - z1.b }
	mova	za.h[w8, 0], { z0.h - z1.h }
	mova	za.s[w8, 0], { z0.s - z1.s }
	mova	za.d[w8, 0], { z0.d - z1.d }
	mova	za.d[w11, 0], { z0.d - z1.d }
	mova	za.d[w8, 7], { z0.d - z1.d }
	mova	za.d[w8, 0], { z30.d - z31.d }
	mova	za.d[w9, 5], { z2.d - z3.d }

	mova	za.b[w8, 0], { z0.b - z3.b }
	mova	za.h[w8, 0], { z0.h - z3.h }
	mova	za.s[w8, 0], { z0.s - z3.s }
	mova	za.d[w8, 0], { z0.d - z3.d }
	mova	za.d[w11, 0], { z0.d - z3.d }
	mova	za.d[w8, 7], { z0.d - z3.d }
	mova	za.d[w8, 0], { z28.d - z31.d }
	mova	za.d[w10, 1], { z20.d - z23.d }

	mova	za0h.b[w12, 0:1], { z0.b - z1.b }
	mova	za0v.b[w12, 0:1], { z0.b - z1.b }
	mova	za0h.b[w15, 0:1], { z0.b - z1.b }
	mova	za0h.b[w12, 14:15], { z0.b - z1.b }
	mova	za0h.b[w12, 0:1], { z30.b - z31.b }
	mova	za0h.b[w14, 6:7], { z8.b - z9.b }

	mova	za0h.h[w12, 0:1], { z0.h - z1.h }
	mova	za0v.h[w12, 0:1], { z0.h - z1.h }
	mova	za1h.h[w12, 0:1], { z0.h - z1.h }
	mova	za1v.h[w12, 0:1], { z0.h - z1.h }
	mova	za0h.h[w15, 0:1], { z0.h - z1.h }
	mova	za0h.h[w12, 6:7], { z0.h - z1.h }
	mova	za0h.h[w12, 0:1], { z30.h - z31.h }
	mova	za0h.h[w13, 2:3], { z10.h - z11.h }

	mova	za0h.s[w12, 0:1], { z0.s - z1.s }
	mova	za0v.s[w12, 0:1], { z0.s - z1.s }
	mova	za3h.s[w12, 0:1], { z0.s - z1.s }
	mova	za3v.s[w12, 0:1], { z0.s - z1.s }
	mova	za0h.s[w15, 0:1], { z0.s - z1.s }
	mova	za0h.s[w12, 2:3], { z0.s - z1.s }
	mova	za0h.s[w12, 0:1], { z30.s - z31.s }
	mova	za2h.s[w14, 0:1], { z18.s - z19.s }

	mova	za0h.d[w12, 0:1], { z0.d - z1.d }
	mova	za0v.d[w12, 0:1], { z0.d - z1.d }
	mova	za7h.d[w12, 0:1], { z0.d - z1.d }
	mova	za7v.d[w12, 0:1], { z0.d - z1.d }
	mova	za0h.d[w15, 0:1], { z0.d - z1.d }
	mova	za0h.d[w12, 0:1], { z30.d - z31.d }
	mova	za6h.d[w13, 0:1], { z22.d - z23.d }

	mova	za0h.b[w12, 0:3], { z0.b - z3.b }
	mova	za0v.b[w12, 0:3], { z0.b - z3.b }
	mova	za0h.b[w15, 0:3], { z0.b - z3.b }
	mova	za0h.b[w12, 12:15], { z0.b - z3.b }
	mova	za0h.b[w12, 0:3], { z28.b - z31.b }
	mova	za0h.b[w14, 8:11], { z12.b - z15.b }

	mova	za0h.h[w12, 0:3], { z0.h - z3.h }
	mova	za0v.h[w12, 0:3], { z0.h - z3.h }
	mova	za1h.h[w12, 0:3], { z0.h - z3.h }
	mova	za1v.h[w12, 0:3], { z0.h - z3.h }
	mova	za0h.h[w15, 0:3], { z0.h - z3.h }
	mova	za0h.h[w12, 4:7], { z0.h - z3.h }
	mova	za0h.h[w12, 0:3], { z28.h - z31.h }
	mova	za0h.h[w13, 4:7], { z16.h - z19.h }

	mova	za0h.s[w12, 0:3], { z0.s - z3.s }
	mova	za0v.s[w12, 0:3], { z0.s - z3.s }
	mova	za3h.s[w12, 0:3], { z0.s - z3.s }
	mova	za3v.s[w12, 0:3], { z0.s - z3.s }
	mova	za0h.s[w15, 0:3], { z0.s - z3.s }
	mova	za0h.s[w12, 0:3], { z28.s - z31.s }
	mova	za2h.s[w13, 0:3], { z20.s - z23.s }

	mova	za0h.d[w12, 0:3], { z0.d - z3.d }
	mova	za0v.d[w12, 0:3], { z0.d - z3.d }
	mova	za7h.d[w12, 0:3], { z0.d - z3.d }
	mova	za7v.d[w12, 0:3], { z0.d - z3.d }
	mova	za0h.d[w15, 0:3], { z0.d - z3.d }
	mova	za0h.d[w12, 0:3], { z28.d - z31.d }
	mova	za5h.d[w13, 0:3], { z24.d - z27.d }
