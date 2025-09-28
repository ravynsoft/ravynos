/* Scalable Matrix Extension (SME).  */

/* MOVA (vector to tile) variant.  */
mova    za0v.b[w12, 0], p0/m, z0.b
mova    za0v.h[w12, 0], p0/m, z0.h
mova    za0v.s[w12, 0], p0/m, z0.s
mova    za0v.d[w12, 0], p0/m, z0.d
mova    za0v.q[w12, 0], p0/m, z0.q

mova    za0v.b[w15, 15], p7/m, z31.b
mova    za1v.h[w15, 7], p7/m, z31.h
mova    za3v.s[w15, 3], p7/m, z31.s
mova    za7v.d[w15, 1], p7/m, z31.d
mova    za15v.q[w15, 0], p7/m, z31.q

mova    za0h.b[w12, 0], p0/m, z0.b
mova    za0h.h[w12, 0], p0/m, z0.h
mova    za0h.s[w12, 0], p0/m, z0.s
mova    za0h.d[w12, 0], p0/m, z0.d
mova    za0h.q[w12, 0], p0/m, z0.q

mova    za0h.b[w15, 15], p7/m, z31.b
mova    za1h.h[w15, 7], p7/m, z31.h
mova    za3h.s[w15, 3], p7/m, z31.s
mova    za7h.d[w15, 1], p7/m, z31.d
mova    za15h.q[w15, 0], p7/m, z31.q

foo .req w12
bar .req w15
mova    za0v.b[foo, 0], p0/m, z0.b
mova    za15h.q[bar, 0], p7/m, z31.q
