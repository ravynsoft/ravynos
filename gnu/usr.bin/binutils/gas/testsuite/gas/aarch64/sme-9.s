/* SVE2 instructions added to support SME.  */

psel p1, p15, p3.b[w15, 0]
psel p2, p14, p5.b[w15, 0]
psel p3, p13, p7.b[w15, 7]
psel p5, p12, p9.b[w15, 15]

psel p8, p11, p15.h[w14, 0]
psel p13, p10, p1.h[w14, 0]
psel p15, p9, p0.h[w14, 3]
psel p1, p8, p6.h[w14, 7]

psel p2, p7, p15.s[w13, 0]
psel p3, p6, p15.s[w13, 0]
psel p5, p5, p15.s[w13, 1]
psel p8, p4, p15.s[w13, 3]

psel p13, p3, p1.d[w12, 0]
psel p15, p2, p1.d[w12, 0]
psel p1, p1, p1.d[w12, 1]

revd z0.q, p0/m, z0.q
revd z0.q, p7/m, z0.q
revd z0.q, p0/m, z31.q
revd z31.q, p7/m, z0.q

sclamp z0.b, z31.b, z17.b
sclamp z31.b, z0.b, z17.b
sclamp z8.b, z1.b, z31.b
sclamp z31.h, z0.h, z17.h
sclamp z8.h, z1.h, z31.h
sclamp z0.s, z31.s, z17.s
sclamp z31.s, z0.s, z17.s
sclamp z8.s, z1.s, z31.s
sclamp z0.d, z31.d, z17.d
sclamp z31.d, z0.d, z17.d
sclamp z8.d, z1.d, z31.d

uclamp z0.b, z31.b, z17.b
uclamp z31.b, z0.b, z17.b
uclamp z8.b, z1.b, z31.b
uclamp z0.h, z31.h, z17.h
uclamp z31.h, z0.h, z17.h
uclamp z8.h, z1.h, z31.h
uclamp z0.s, z31.s, z17.s
uclamp z31.s, z0.s, z17.s
uclamp z8.s, z1.s, z31.s
uclamp z0.d, z31.d, z17.d
uclamp z31.d, z0.d, z17.d
uclamp z8.d, z1.d, z31.d

/* The unpredicated MOVPRFX instruction.  */
movprfx z3, z5
revd z3.q, p1/m, z5.q

movprfx z1, z4
revd z1.q, p1/m, z5.q

movprfx z1, z4
sclamp z1.b, z10.b, z11.b

movprfx z2, z4
sclamp z2.h, z10.h, z11.h

movprfx z3, z4
sclamp z3.s, z10.s, z11.s

movprfx z4, z5
sclamp z4.d, z10.d, z11.d

movprfx z1, z4
uclamp z1.b, z10.b, z11.b

movprfx z2, z4
uclamp z2.h, z10.h, z11.h

movprfx z3, z4
uclamp z3.s, z10.s, z11.s

movprfx z4, z5
uclamp z4.d, z10.d, z11.d

foo .req p1
bar .req w15
psel foo, p15, p3.b[w15, 0]
psel p2, p14, p5.b[bar, 0]

// These were previously incorrectly decoded as PSELs.
.inst 0x25244200
.inst 0x25244010
.inst 0x25244210
