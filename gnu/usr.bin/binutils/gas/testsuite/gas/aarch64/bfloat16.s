/* The instructions with non-zero register numbers are there to ensure we have
   the correct argument positioning (i.e. check that the first argument is at
   the end of the word etc).
   The instructions with all-zero register numbers are to ensure the previous
   encoding didn't just "happen" to fit -- so that if we change the registers
   that changes the correct part of the word.
   Each of the numbered patterns begin and end with a 1, so we can replace
   them with all-zeros and see the entire range has changed. */

// SVE
bfdot	z17.s,  z21.h,  z27.h
bfdot	z0.s,  z0.h,  z0.h

bfdot	z17.s,  z21.h,  z5.h[3]
bfdot	z0.s,  z0.h,  z0.h[3]
bfdot	z0.s,  z0.h,  z0.h[0]

bfmmla	z17.s,  z21.h,  z27.h
bfmmla	z0.s,  z0.h,  z0.h

bfcvt	z17.h, p5/m, z21.s
bfcvt	z0.h, p0/m, z0.s
bfcvtnt	z17.h, p5/m, z21.s
bfcvtnt	z0.h, p0/m, z0.s

bfmlalt z17.s, z21.h, z27.h
bfmlalt z0.s, z0.h, z0.h
bfmlalb z17.s, z21.h, z27.h
bfmlalb z0.s, z0.h, z0.h

bfmlalt z17.s, z21.h, z5.h[0]
bfmlalt z0.s, z0.h, z0.h[7]
bfmlalb z17.s, z21.h, z5.h[0]
bfmlalb z0.s, z0.h, z0.h[7]

// SIMD
bfdot	v17.2s, v21.4h, v27.4h
bfdot	v0.2s, v0.4h, v0.4h
bfdot	v17.4s, v21.8h, v27.8h
bfdot	v0.4s, v0.8h, v0.8h

bfdot	v17.2s, v21.4h, v27.2h[3]
bfdot	v0.2s, v0.4h, v0.2h[3]
bfdot	v17.4s, v21.8h, v27.2h[3]
bfdot	v0.4s, v0.8h, v0.2h[3]
bfdot	v17.2s, v21.4h, v27.2h[0]
bfdot	v0.2s, v0.4h, v0.2h[0]
bfdot	v17.4s, v21.8h, v27.2h[0]
bfdot	v0.4s, v0.8h, v0.2h[0]

bfmmla	v17.4s, v21.8h, v27.8h
bfmmla	v0.4s, v0.8h, v0.8h

bfmlalb	v17.4s, v21.8h, v27.8h
bfmlalb	v0.4s, v0.8h, v0.8h
bfmlalt	v17.4s, v21.8h, v27.8h
bfmlalt	v0.4s, v0.8h, v0.8h

bfmlalb	v17.4s, v21.8h, v15.h[0]
bfmlalb	v0.4s, v0.8h, v0.h[7]
bfmlalt	v17.4s, v21.8h, v15.h[0]
bfmlalt	v0.4s, v0.8h, v0.h[7]

bfcvtn	v17.4h, v21.4s
bfcvtn	v0.4h, v0.4s
bfcvtn2	v17.8h, v21.4s
bfcvtn2	v0.8h, v0.4s

bfcvt	h17, s21
bfcvt	h0, s0
