/* Scalable Matrix Extension (SME).  */

/* ADDHA 32-bit variant.  */
addha za4.s, p0/m, p1/m, z1.s
addha za15.s, p2/m, p3/m, z2.s
addha za0.s, p2/m, p3/m, z2.d
addha z0.s, p0/m, p1/m, z1.s

/* ADDHA 64-bit variant.  */
addha za8.d, p0/m, p1/m, z1.d
addha za15.d, p2/m, p3/m, z2.d
addha za0.d, p2/m, p3/m, z2.s

/* ADDVA 32-bit variant.  */
addva za4.s, p0/m, p1/m, z1.s
addva za15.s, p2/m, p3/m, z2.s
addva za0.s, p2/m, p3/m, z2.d

/* ADDVA 64-bit variant.  */
addva za8.d, p0/m, p1/m, z1.d
addva za15.d, p2/m, p3/m, z2.d
addva za0.d, p2/m, p3/m, z2.s

/* BFMOPA.  */
bfmopa za4.s, p0/m, p1/m, z1.h, z4.h
bfmopa za0.s, p2/m, p3/m, z2.s, z3.s

/* BFMOPS.  */
bfmops za4.s, p0/m, p1/m, z1.h, z4.h
bfmops za0.s, p2/m, p3/m, z2.s, z3.s

/* FMOPA (non-widening), single-precision.  */
fmopa za4.s, p0/m, p1/m, z1.s, z4.s
fmopa za0.s, p6/m, p7/m, z4.d, z1.d

/* FMOPA (non-widening), double-precision.  */
fmopa za8.d, p0/m, p1/m, z1.d, z8.d
fmopa za0.d, p2/m, p3/m, z2.s, z7.s

/* FMOPA (widening)  */
fmopa za4.s, p0/m, p1/m, z1.h, z4.h
fmopa za1.s, p2/m, p3/m, z2.q, z3.q

/* FMOPS (non-widening), single-precision.  */
fmops za4.s, p0/m, p1/m, z1.s, z4.s
fmops za1.s, p2/m, p3/m, z2.q, z3.q

/* FMOPS (non-widening), double-precision.  */
fmops za8.d, p0/m, p1/m, z1.d, z8.d
fmops za0.d, p2/m, p3/m, z2.s, z7.s

/* FMOPS (widening)  */
fmops za8.s, p0/m, p1/m, z1.h, z4.h
fmops za1.q, p2/m, p3/m, z2.h, z3.h

/* SMOPA 32-bit variant.  */
smopa za4.s, p0/m, p1/m, z1.b, z4.b
smopa za1.q, p2/m, p3/m, z2.b, z3.b

/* SMOPA 64-bit variant.  */
smopa za8.d, p0/m, p1/m, z1.h, z8.h
smopa za1.d, p2/m, p3/m, z2.h, z7.q

/* SMOPS 32-bit variant.  */
smops za4.s, p0/m, p1/m, z1.b, z4.b
smops za1.q, p2/m, p3/m, z2.b, z3.b

/* SMOPS 64-bit variant.  */
smops za8.d, p0/m, p1/m, z1.h, z8.h
smops za1.d, p2/m, p3/m, z2.h, z7.q

/* SUMOPA 32-bit variant.  */
sumopa za4.s, p0/m, p1/m, z1.b, z4.b
sumopa za1.q, p2/m, p3/m, z2.s, z3.s

/* SUMOPA 64-bit variant.  */
sumopa za8.d, p0/m, p1/m, z1.h, z8.h
sumopa za1.d, p2/m, p3/m, z2.h, z7.q

/* SUMOPS 32-bit variant.  */
sumops za4.s, p0/m, p1/m, z1.b, z4.b
sumops za1.q, p2/m, p3/m, z2.b, z3.b

/* SUMOPS 64-bit variant.  */
sumops za8.d, p0/m, p1/m, z1.h, z8.h
sumops za1.q, p2/m, p3/m, z2.h, z7.h

/* UMOPA 32-bit variant.  */
umopa za4.s, p0/m, p1/m, z1.b, z4.b
umopa za1.q, p2/m, p3/m, z2.b, z3.b

/* UMOPA 64-bit variant.  */
umopa za8.d, p0/m, p1/m, z1.h, z8.h
umopa za1.q, p2/m, p3/m, z2.h, z7.h

/* UMOPS 32-bit variant.  */
umops za4.s, p0/m, p1/m, z1.b, z4.b
umops za1.q, p2/m, p3/m, z2.b, z3.b

/* UMOPS 64-bit variant.  */
umops za8.d, p0/m, p1/m, z1.h, z8.h
umops za1.d, p2/m, p3/m, z2.d, z7.d

/* USMOPA 32-bit variant.  */
usmopa za4.s, p0/m, p1/m, z1.b, z4.b
usmopa za1.q, p2/m, p3/m, z2.b, z3.b

/* USMOPA 64-bit variant.  */
usmopa za8.d, p0/m, p1/m, z1.h, z8.h
usmopa za1.q, p2/m, p3/m, z2.h, z7.h

/* USMOPS 32-bit variant.  */
usmops za4.s, p0/m, p1/m, z1.b, z4.b
usmops za1.s, p2/m, p3/m, z2.s, z3.b

/* USMOPS 64-bit variant.  */
usmops za8.d, p0/m, p1/m, z1.h, z8.h
usmops za1.d, p2/m, p3/m, z2.d, z7.d
