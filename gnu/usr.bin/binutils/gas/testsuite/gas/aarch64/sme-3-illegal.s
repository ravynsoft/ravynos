/* Scalable Matrix Extension (SME).  */

/* MOVA (vector to tile) variant.  */
mova    za1v.b[w12, #0], p0/m, z0.b
mova    za2v.h[w12, #0], p0/m, z0.h
mova    za4v.s[w12, #0], p0/m, z0.s
mova    za8v.d[w12, #0], p0/m, z0.d
mova    za16v.q[w12], p0/m, z0.q

mova    za0v.b[w15, #16], p7/m, z31.b
mova    za1v.h[w15, #8], p7/m, z31.h
mova    za3v.s[w15, #4], p7/m, z31.s
mova    za7v.d[w15, #2], p7/m, z31.d
mova    za15v.q[w15, #1], p7/m, z31.q

mova	za0h.b[w12, #0, vgx2], p0/m, z0.b
mova	za0h.b[w12, #0, vgx4], p0/m, z0.b
mova	za0h.b[w12, #0, vgx8], p0/m, z0.b

mova	za0h.b[w12, #0, vgx2], p0/z, z0.b
