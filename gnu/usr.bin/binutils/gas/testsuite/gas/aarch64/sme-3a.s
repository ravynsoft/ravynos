/* Scalable Matrix Extension (SME).  */

/* MOV alias (vector to tile) variant.  */
mov    za0v.b[w12, 0], p0/m, z0.b
mov    za0v.h[w12, 0], p0/m, z0.h
mov    za0v.s[w12, 0], p0/m, z0.s
mov    za0v.d[w12, 0], p0/m, z0.d
mov    za0v.q[w12, 0], p0/m, z0.q

mov    za0v.b[w15, 15], p7/m, z31.b
mov    za1v.h[w15, 7], p7/m, z31.h
mov    za3v.s[w15, 3], p7/m, z31.s
mov    za7v.d[w15, 1], p7/m, z31.d
mov    za15v.q[w15, 0], p7/m, z31.q

mov    za0h.b[w12, 0], p0/m, z0.b
mov    za0h.h[w12, 0], p0/m, z0.h
mov    za0h.s[w12, 0], p0/m, z0.s
mov    za0h.d[w12, 0], p0/m, z0.d
mov    za0h.q[w12, 0], p0/m, z0.q

mov    za0h.b[w15, 15], p7/m, z31.b
mov    za1h.h[w15, 7], p7/m, z31.h
mov    za3h.s[w15, 3], p7/m, z31.s
mov    za7h.d[w15, 1], p7/m, z31.d
mov    za15h.q[w15, 0], p7/m, z31.q
