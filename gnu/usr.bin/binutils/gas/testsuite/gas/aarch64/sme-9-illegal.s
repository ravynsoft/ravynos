/* Scalable Matrix Extension (SME).  */

psel p1, p15, p3.b[w12]
psel p1, p15, p3.q[w15]
psel p1, p15, p3.q[w15, #0]
psel p1, p15, p3[w15,#0]
psel p1, p15, p3.b[w11]
psel p8, p11, p15.h[w16]
psel p2, p7, p15.s[w3]
psel p13, p3, p1.d[w17]
psel p1, p15, p3.b[w11, #0]
psel p8, p11, p15.h[w16, #0]
psel p2, p7, p15.s[w3, #0]
psel p13, p3, p1.d[w17, #0]
psel p5, p12, p9.b[w15, #16]
psel p1, p8, p6.h[w14, #8]
psel p8, p4, p15.s[w13, #4]
psel p1, p1, p1.d[w12, #2]

psel p0, p0, p0.b[w12, #0, vgx2]
psel p0, p0, p0.b[w12, #0, vgx4]
psel p0, p0, p0.b[w12, #0, vgx8]

psel p0.b, p0.b, p0.b[w12, #0, vgx2]

psel p0, p0, p0.b[w12, 0:1]
psel p0, p0, p0.b[w12, 0:1, vgx2]
psel p0.b, p0.b, p0.b[w12, 0:1, vgx2]

revd z0.q, p0/m, z0.b

sclamp z8.b, z1.b, z31.q
sclamp z31.h, z0.h, z17.q
sclamp z0.s, z31.s, z17.q
sclamp z31.d, z0.d, z17.q
sclamp z31.q, z0.d, z17.q

uclamp z8.b, z1.b, z31.q
uclamp z31.h, z0.h, z17.q
uclamp z0.s, z31.s, z17.q
uclamp z31.d, z0.d, z17.q
uclamp z31.q, z0.d, z17.q
