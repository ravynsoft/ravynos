/* The instructions with non-zero register numbers are there to ensure we have
   the correct argument positioning (i.e. check that the first argument is at
   the end of the word etc).
   The instructions with all-zero register numbers are to ensure the previous
   encoding didn't just "happen" to fit -- so that if we change the registers
   that changes the correct part of the word.
   Each of the numbered patterns begin and end with a 1, so we can replace
   them with all-zeros and see the entire range has changed. */

// SVE
smmla z17.s,  z21.b,  z27.b
smmla z0.s,  z0.b,  z0.b

ummla z17.s,  z21.b,  z27.b
ummla z0.s,  z0.b,  z0.b

usmmla z17.s,  z21.b,  z27.b
usmmla z0.s,  z0.b,  z0.b

usdot	z17.s,  z21.b,  z27.b
usdot	z0.s,  z0.b,  z0.b

usdot	z17.s,  z21.b,  z7.b[3]
usdot	z0.s,  z0.b,  z0.b[3]
usdot	z17.s,  z21.b,  z7.b[0]
usdot	z0.s,  z0.b,  z0.b[0]

sudot	z17.s,  z21.b,  z7.b[3]
sudot	z0.s,  z0.b,  z0.b[3]
sudot	z17.s,  z21.b,  z7.b[0]
sudot	z0.s,  z0.b,  z0.b[0]

// SIMD
smmla	v17.4s, v21.16b, v27.16b
smmla	v17.4s, v21.16b, v27.16b

ummla	v17.4s, v21.16b, v27.16b
ummla	v0.4s, v0.16b, v0.16b

usmmla	v0.4s, v0.16b, v0.16b
usmmla	v17.4s, v21.16b, v27.16b

usdot	v17.2s, v21.8b, v27.8b
usdot	v0.2s, v0.8b, v0.8b
usdot	v17.4s, v21.16b, v27.16b
usdot	v0.4s, v0.16b, v0.16b

usdot	v17.2s, v21.8b, v27.4b[3]
usdot	v0.2s, v0.8b, v0.4b[3]
usdot	v17.2s, v21.8b, v27.4b[0]
usdot	v0.2s, v0.8b, v0.4b[0]
usdot	v17.4s, v21.16b, v27.4b[3]
usdot	v0.4s, v0.16b, v0.4b[3]
usdot	v17.4s, v21.16b, v27.4b[0]
usdot	v0.4s, v0.16b, v0.4b[0]

sudot	v17.2s, v21.8b, v27.4b[3]
sudot	v0.2s, v0.8b, v0.4b[3]
sudot	v17.2s, v21.8b, v27.4b[0]
sudot	v0.2s, v0.8b, v0.4b[0]
sudot	v17.4s, v21.16b, v27.4b[3]
sudot	v0.4s, v0.16b, v0.4b[3]
sudot	v17.4s, v21.16b, v27.4b[0]
sudot	v0.4s, v0.16b, v0.4b[0]
