	.text
	.arch armv8-a+sve+bf16

f:
	// OK
	movprfx z0.s, p1/m, z1.s
	bfcvt z0.h, p1/m, z2.s

	// OK
	movprfx z0.s, p1/z, z1.s
	bfcvt z0.h, p1/m, z2.s

	// Wrong size
	movprfx z0.h, p1/m, z1.h
	bfcvt z0.h, p1/m, z2.s

	// Wrong size
	movprfx z0.h, p1/z, z1.h
	bfcvt z0.h, p1/m, z2.s

	// OK
	movprfx z0, z1
	bfcvt z0.h, p1/m, z2.s

	// Not prefixable
	movprfx z0, z1
	bfcvtnt z0.h, p1/m, z2.s

	// Not prefixable
	movprfx z0.s, p1/m, z1.s
	bfcvtnt z0.h, p1/m, z2.s

	// Not prefixable
	movprfx z0.s, p1/z, z1.s
	bfcvtnt z0.h, p1/m, z2.s

	// Not prefixable
	movprfx z0.h, p1/m, z1.h
	bfcvtnt z0.h, p1/m, z2.s

	// Not prefixable
	movprfx z0.h, p1/z, z1.h
	bfcvtnt z0.h, p1/m, z2.s

	ret
