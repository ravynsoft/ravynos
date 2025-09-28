// SVE
bfdot	z0.s,  z1.h,  z2.s  // Fails from size types

bfdot	z0.s,  z1.h,  z3.s[3] // Fails from size types
bfdot	z0.s,  z1.h,  z3.h[4] // Fails from index size
bfdot	z0.s,  z1.h,  z8.h[3] // Fails from vector number

bfmmla	z0.s,  z1.h,  z2.s  // Fails from size types

bfcvt	z0.h, p1/z, z2.s   // Fails from merge type
bfcvt	z0.h, p1/m, z2.h   // Fails from size type

bfcvtnt	z0.h, p1/z, z2.s   // Fails from merge type
bfcvtnt	z0.h, p1/m, z2.h   // Fails from size type

bfmlalt z0.s, z0.h, z0.s   // Fails from size type
bfmlalt z32.s, z0.h, z0.h
bfmlalt z0.s, z32.h, z0.h
bfmlalt z0.s, z0.h, z32.h

bfmlalt z0.s, z0.h, z0.h[8] // Fails from index size
bfmlalt z0.s, z0.h, z0.s[0] // Fails from size type
bfmlalt z32.s, z0.h, z0.h[0]
bfmlalt z0.s, z32.h, z0.h[0]
bfmlalt z0.s, z0.h, z8.h[0] // Fails from vector index

bfmlalb z0.s, z0.h, z0.s   // Fails from size type
bfmlalb z32.s, z0.h, z0.h
bfmlalb z0.s, z32.h, z0.h
bfmlalb z0.s, z0.h, z32.h

bfmlalb z0.s, z0.h, z0.h[8] // Fails from index size
bfmlalb z0.s, z0.h, z0.s[0] // Fails from size type
bfmlalb z32.s, z0.h, z0.h[0]
bfmlalb z0.s, z32.h, z0.h[0]
bfmlalb z0.s, z0.h, z8.h[0] // Fails from vector index

// SIMD
bfdot	v0.2s, v1.4h, v2.2s[3] // Fails from size types
bfdot	v0.4s, v1.8h, v2.2h[4] // Fails from index size

bfmmla	v0.4s, v1.8h, v2.8s  // Fails from size types
bfmmla	v0.4s, v1.4h, v2.8h  // Fails from size types

bfmlalb	v0.4s, v0.4h, v0.8h
bfmlalb	v32.4s, v0.8h, v0.8h
bfmlalb	v0.4s, v32.8h, v0.8h
bfmlalb	v0.4s, v0.8h, v32.8h
bfmlalt	v0.4s, v0.8h, v0.4h
bfmlalt	v32.4s, v0.8h, v0.8h
bfmlalt	v0.4s, v32.8h, v0.8h
bfmlalt	v0.4s, v0.8h, v32.8h

bfmlalb	v0.4s, v0.8h, v0.h[8]
bfmlalb	v32.4s, v0.8h, v0.h[0]
bfmlalb	v0.4s, v32.8h, v0.h[0]
bfmlalb	v0.4s, v0.8h, v16.h[0]
bfmlalb	v0.4s, v0.4h, v0.h[0]
bfmlalb	v0.4s, v0.8h, v0.s[0]
bfmlalt	v0.4s, v0.8h, v0.s[0]
bfmlalt	v0.4s, v0.4h, v0.h[0]
bfmlalt	v0.4s, v0.8h, v0.h[8]
bfmlalt	v32.4s, v0.8h, v0.h[0]
bfmlalt	v0.4s, v32.8h, v0.h[0]
bfmlalt	v0.4s, v0.8h, v16.h[0]

bfcvt	h0, h1 // Fails from size types
