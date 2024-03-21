/* Scalable Matrix Extension (SME).  */

/* MOV alias (tile to vector) variant.  */
mov	z0.b, p0/m, za0v.b[w12, 0]
mov	z0.h, p0/m, za0v.h[w12, 0]
mov	z0.s, p0/m, za0v.s[w12, 0]
mov	z0.d, p0/m, za0v.d[w12, 0]
mov	z0.q, p0/m, za0v.q[w12, 0]

mov	z31.b, p7/m, za0v.b[w15, 15]
mov	z31.h, p7/m, za1v.h[w15, 7]
mov	z31.s, p7/m, za3v.s[w15, 3]
mov	z31.d, p7/m, za7v.d[w15, 1]
mov	z31.q, p7/m, za15v.q[w15, 0]

mov	z0.b, p0/m, za0h.b[w12, 0]
mov	z0.h, p0/m, za0h.h[w12, 0]
mov	z0.s, p0/m, za0h.s[w12, 0]
mov	z0.d, p0/m, za0h.d[w12, 0]
mov	z0.q, p0/m, za0h.q[w12, 0]

mov	z31.b, p7/m, za0h.b[w15, 15]
mov	z31.h, p7/m, za1h.h[w15, 7]
mov	z31.s, p7/m, za3h.s[w15, 3]
mov	z31.d, p7/m, za7h.d[w15, 1]
mov	z31.q, p7/m, za15h.q[w15, 0]
