.text
.arch armv8.2-a+bf16+sve

movprfx z0, z1
bfdot	z0.s,  z2.h,  z3.h

movprfx z0, z1
bfdot	z0.s,  z2.h,  z3.h[0]

movprfx z0, z1
bfmmla z0.s,  z2.h,  z3.h

movprfx z0, z1
bfmlalb	z0.s,  z2.h,  z3.h

movprfx z0, z1
bfmlalt	z0.s,  z2.h,  z3.h

movprfx z0, z1
bfmlalb	z0.s,  z2.h,  z3.h[0]

movprfx z0, z1
bfmlalt	z0.s,  z2.h,  z3.h[0]

# Unpredicated movprfx + bfcvt
movprfx z0, z1
bfcvt z0.h, p0/m, z2.s

# Predicated movprfx + bfcvt
movprfx z0.s, p0/m, z1.s
bfcvt z0.h, p0/m, z2.s
