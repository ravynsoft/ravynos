/* MOVPRFX tests for matrix multiply instructions */

movprfx z17, z0
smmla z17.s,  z21.b,  z27.b

movprfx z17, z0
ummla z17.s,  z21.b,  z27.b

movprfx z17, z0
usmmla z17.s,  z21.b,  z27.b

movprfx z17, z0
usdot	z17.s,  z21.b,  z27.b

movprfx z17, z0
usdot	z17.s,  z21.b,  z7.b[3]

movprfx z17, z0
sudot	z17.s,  z21.b,  z7.b[3]

movprfx z17, z0
fmmla	z17.s,  z21.s,  z27.s

movprfx z17, z0
fmmla	z17.d,  z21.d,  z27.d
