	ld1	{ v31.2d - v0.2d }, [x0]
	ld1	{ v31.4s - v1.4s }, [x0]
	ld1	{ v31.8h - v2.8h }, [x0]
	ld1	{ v30.16b - v1.16b }, [x0]
	ld1	{ v30.8b - v0.8b }, [x0]
	ld1	{ v29.4h - v0.4h }, [x0]

	ld2b	{ z31.b - z0.b }, p0/z, [x0]

	ld3b	{ z30.b - z0.b }, p0/z, [x0]
	ld3b	{ z31.b - z1.b }, p0/z, [x0]

	ld4b	{ z29.b - z0.b }, p0/z, [x0]
	ld4b	{ z30.b - z1.b }, p0/z, [x0]
	ld4b	{ z31.b - z2.b }, p0/z, [x0]
