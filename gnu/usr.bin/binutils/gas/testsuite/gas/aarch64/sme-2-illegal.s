/* Scalable Matrix Extension (SME).  */

/* MOVA (tile to vector) variant.  */
mova	z0.b, p0/m, za1h.b[w12, #0]
mova	z0.h, p0/m, za2h.h[w12, #0]
mova	z0.s, p0/m, za4h.s[w12, #0]
mova	z0.d, p0/m, za8h.d[w12, #0]
mova	z0.q, p0/m, za16h.q[w12]

mova	z31.b, p7/m, za0v.b[w15, #16]
mova	z31.h, p7/m, za1v.h[w15, #8]
mova	z31.s, p7/m, za3v.s[w15, #4]
mova	z31.d, p7/m, za7v.d[w15, #2]
mova    z31.q, p7/m, za15v.q[w15, #1]
mova    z31.q, p7/m, za15v.q[w15]

/* Syntax issues.  */
mova    z0.b, p0/m, za0v.b
mova    z31.b, p7/m, za0v.b[15, w15]
mova    z0.h, p0/m, za0v.h[w12. 0]
mova    z0.s, p0/m, za0v.s[x12, 0]
mova    z0.d, p0/m, za0v.d[w21, 0]
mova    z0.q, p0/m, za0v.q[s12]
mova    z0.q, p0/m, za0v.q[d12]
mova    z0.q, p0/m, za0v.q[w12,]
mova    z0.q, p0/m, za0v.q[w12.]
mova    z0.q, p0/m, za0v.q[w12, abc]
mova    z0.q, p0/m, za0v.q[w12, #abc]
mova    z0.q, p0/m, za0v.q[w12, 1a]
mova    z0.q, p0/m, za0v.q[w12, #1a]
mova    z0.q, p0/m, za0v.q[w12, 1a2]
mova    z0.q, p0/m, za0v.q[w12, #1a2]

mova	z0.b, p0/m, za0h.b[w12, #0, vgx2]
mova	z0.b, p0/m, za0h.b[w12, #0, vgx4]
mova	z0.b, p0/m, za0h.b[w12, #0, vgx8]

mova	z0.b, p0/m, za0h.b[w12, 1:0]
mova	z0.b, p0/m, za0h.b[w12, 0:0]
mova	z0.b, p0/m, za0h.b[w12, 0:1]
mova	z0.b, p0/m, za0h.b[w12, 0:2]
mova	z0.b, p0/m, za0h.h[w12, 0:1]
mova	z0.b, p0/m, za0h.b[w12, 0:foo]
