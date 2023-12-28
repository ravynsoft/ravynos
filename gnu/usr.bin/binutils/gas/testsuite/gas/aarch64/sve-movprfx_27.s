	.text
	.arch armv8-a+sve

f:
	movprfx z0, z1
	fmov z0.s, p0/m, #1.0

	movprfx z0, z1
	fcpy z0.s, p0/m, #1.0

	ret
