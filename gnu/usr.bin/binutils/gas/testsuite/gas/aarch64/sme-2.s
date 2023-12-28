/* Scalable Matrix Extension (SME).  */

/* MOVA (tile to vector) variant.  */
mova	z0.b, p0/m, za0v.b[w12, 0]
mova	z0.h, p0/m, za0v.h[w12, 0]
mova	z0.s, p0/m, za0v.s[w12, 0]
mova	z0.d, p0/m, za0v.d[w12, 0]
mova	z0.q, p0/m, za0v.q[w12, 0]

mova	z31.b, p7/m, za0v.b[w15, 15]
mova	z31.h, p7/m, za1v.h[w15, 7]
mova	z31.s, p7/m, za3v.s[w15, 3]
mova	z31.d, p7/m, za7v.d[w15, 1]
mova	z31.q, p7/m, za15v.q[w15, 0]

mova	z0.b, p0/m, za0h.b[w12, 0]
mova	z0.h, p0/m, za0h.h[w12, 0]
mova	z0.s, p0/m, za0h.s[w12, 0]
mova	z0.d, p0/m, za0h.d[w12, 0]
mova	z0.q, p0/m, za0h.q[w12, 0]

mova	z31.b, p7/m, za0h.b[w15, 15]
mova	z31.h, p7/m, za1h.h[w15, 7]
mova	z31.s, p7/m, za3h.s[w15, 3]
mova	z31.d, p7/m, za7h.d[w15, 1]
mova	z31.q, p7/m, za15h.q[w15, 0]

/* Parser checks.  */
mova	z31.b , p7/m , za0h.b [ w15 , 15 ]
mova	z31.h , p7/m , za1h.h [ w15 , 7 ]
mova	z31.s , p7/m , za3h.s [ w15 , 3 ]
mova	z31.d , p7/m , za7h.d [ w15 , 1 ]
mova	z31.q , p7/m , za15h.q [ w15 , #0 ]
mova	z31.b , p7/m , za0h.b [ w15 , #15 ]
mova	z31.h , p7/m , za1h.h [ w15 , #7 ]
mova	z31.s , p7/m , za3h.s [ w15 , #3 ]
mova	z31.d , p7/m , za7h.d [ w15 , #1 ]
mova	z31.q , p7/m , za15h.q [ w15, #0 ]

/* Register aliases.  */
foo .req w15
bar .req za7h
baz .req z31

mova	z31.d , p7/m , bar.d [ foo , #1 ]
mova	baz.q , p7/m , za15h.q [ foo , #0 ]

/* Named immediate.  */
val_zero = 0
val_seven = 7
mova z0.b, p1/m, za0v.b[w13, #val_zero]
mova z0.b, p1/m, za0v.b[w13, #val_seven]
