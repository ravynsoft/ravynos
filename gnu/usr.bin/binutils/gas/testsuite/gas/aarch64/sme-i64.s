/* Scalable Matrix Extension (SME I64).  */

/* ADDHA 64-bit variant.  */
addha za0.d, p0/m, p1/m, z1.d
addha za1.d, p2/m, p3/m, z2.d
addha za2.d, p4/m, p5/m, z3.d
addha za3.d, p6/m, p7/m, z4.d
addha za4.d, p1/m, p0/m, z5.d
addha za5.d, p3/m, p2/m, z6.d
addha za6.d, p5/m, p4/m, z7.d
addha za7.d, p7/m, p6/m, z8.d
addha za4.d, p7/m, p0/m, z5.d
addha za5.d, p6/m, p1/m, z6.d
addha za6.d, p5/m, p2/m, z7.d
addha za7.d, p4/m, p3/m, z8.d

/* ADDVA 64-bit variant.  */
addva za0.d, p0/m, p1/m, z1.d
addva za1.d, p2/m, p3/m, z2.d
addva za2.d, p4/m, p5/m, z3.d
addva za3.d, p6/m, p7/m, z4.d
addva za4.d, p1/m, p0/m, z5.d
addva za5.d, p3/m, p2/m, z6.d
addva za6.d, p5/m, p4/m, z7.d
addva za7.d, p7/m, p6/m, z8.d
addva za4.d, p7/m, p0/m, z5.d
addva za5.d, p6/m, p1/m, z6.d
addva za6.d, p5/m, p2/m, z7.d
addva za7.d, p4/m, p3/m, z8.d

/* SMOPA 64-bit variant.  */
smopa za0.d, p0/m, p1/m, z1.h, z8.h
smopa za1.d, p2/m, p3/m, z2.h, z7.h
smopa za2.d, p4/m, p5/m, z3.h, z6.h
smopa za3.d, p6/m, p7/m, z4.h, z5.h
smopa za4.d, p1/m, p0/m, z5.h, z4.h
smopa za5.d, p3/m, p2/m, z6.h, z3.h
smopa za6.d, p5/m, p4/m, z7.h, z2.h
smopa za7.d, p7/m, p6/m, z8.h, z1.h

/* SMOPS 64-bit variant.  */
smops za0.d, p0/m, p1/m, z1.h, z8.h
smops za1.d, p2/m, p3/m, z2.h, z7.h
smops za2.d, p4/m, p5/m, z3.h, z6.h
smops za3.d, p6/m, p7/m, z4.h, z5.h
smops za4.d, p1/m, p0/m, z5.h, z4.h
smops za5.d, p3/m, p2/m, z6.h, z3.h
smops za6.d, p5/m, p4/m, z7.h, z2.h
smops za7.d, p7/m, p6/m, z8.h, z1.h
smops za4.d, p7/m, p0/m, z5.h, z4.h
smops za5.d, p6/m, p1/m, z6.h, z3.h
smops za6.d, p5/m, p2/m, z7.h, z2.h
smops za7.d, p4/m, p3/m, z8.h, z1.h

/* SUMOPA 64-bit variant.  */
sumopa za0.d, p0/m, p1/m, z1.h, z8.h
sumopa za1.d, p2/m, p3/m, z2.h, z7.h
sumopa za2.d, p4/m, p5/m, z3.h, z6.h
sumopa za3.d, p6/m, p7/m, z4.h, z5.h
sumopa za4.d, p1/m, p0/m, z5.h, z4.h
sumopa za5.d, p3/m, p2/m, z6.h, z3.h
sumopa za6.d, p5/m, p4/m, z7.h, z2.h
sumopa za7.d, p7/m, p6/m, z8.h, z1.h

/* SUMOPS 64-bit variant.  */
sumops za0.d, p0/m, p1/m, z1.h, z8.h
sumops za1.d, p2/m, p3/m, z2.h, z7.h
sumops za2.d, p4/m, p5/m, z3.h, z6.h
sumops za3.d, p6/m, p7/m, z4.h, z5.h
sumops za4.d, p1/m, p0/m, z5.h, z4.h
sumops za5.d, p3/m, p2/m, z6.h, z3.h
sumops za6.d, p5/m, p4/m, z7.h, z2.h
sumops za7.d, p7/m, p6/m, z8.h, z1.h

/* UMOPA 64-bit variant.  */
umopa za0.d, p0/m, p1/m, z1.h, z8.h
umopa za1.d, p2/m, p3/m, z2.h, z7.h
umopa za2.d, p4/m, p5/m, z3.h, z6.h
umopa za3.d, p6/m, p7/m, z4.h, z5.h
umopa za4.d, p1/m, p0/m, z5.h, z4.h
umopa za5.d, p3/m, p2/m, z6.h, z3.h
umopa za6.d, p5/m, p4/m, z7.h, z2.h
umopa za7.d, p7/m, p6/m, z8.h, z1.h

/* UMOPS 64-bit variant.  */
umops za0.d, p0/m, p1/m, z1.h, z8.h
umops za1.d, p2/m, p3/m, z2.h, z7.h
umops za2.d, p4/m, p5/m, z3.h, z6.h
umops za3.d, p6/m, p7/m, z4.h, z5.h
umops za4.d, p1/m, p0/m, z5.h, z4.h
umops za5.d, p3/m, p2/m, z6.h, z3.h
umops za6.d, p5/m, p4/m, z7.h, z2.h
umops za7.d, p7/m, p6/m, z8.h, z1.h

/* USMOPA 64-bit variant.  */
usmopa za0.d, p0/m, p1/m, z1.h, z8.h
usmopa za1.d, p2/m, p3/m, z2.h, z7.h
usmopa za2.d, p4/m, p5/m, z3.h, z6.h
usmopa za3.d, p6/m, p7/m, z4.h, z5.h
usmopa za4.d, p1/m, p0/m, z5.h, z4.h
usmopa za5.d, p3/m, p2/m, z6.h, z3.h
usmopa za6.d, p5/m, p4/m, z7.h, z2.h
usmopa za7.d, p7/m, p6/m, z8.h, z1.h

/* USMOPS 64-bit variant.  */
usmops za0.d, p0/m, p1/m, z1.h, z8.h
usmops za1.d, p2/m, p3/m, z2.h, z7.h
usmops za2.d, p4/m, p5/m, z3.h, z6.h
usmops za3.d, p6/m, p7/m, z4.h, z5.h
usmops za4.d, p1/m, p0/m, z5.h, z4.h
usmops za5.d, p3/m, p2/m, z6.h, z3.h
usmops za6.d, p5/m, p4/m, z7.h, z2.h
usmops za7.d, p7/m, p6/m, z8.h, z1.h
usmops za4.d, p7/m, p0/m, z5.h, z4.h
usmops za5.d, p6/m, p1/m, z6.h, z3.h
usmops za6.d, p5/m, p2/m, z7.h, z2.h
usmops za7.d, p4/m, p3/m, z8.h, z1.h

/* Register aliases.  */
foo .req za3
bar .req za7
baz .req za0

addha baz.d, p0/m, p1/m, z1.d
addva bar.d, p4/m, p3/m, z8.d
bfmopa foo.s, p6/m, p7/m, z4.h, z1.h
bfmops foo.s, p6/m, p7/m, z4.h, z1.h
smopa bar.d, p7/m, p6/m, z8.h, z1.h
smops bar.d, p4/m, p3/m, z8.h, z1.h
sumopa bar.d, p7/m, p6/m, z8.h, z1.h
sumops bar.d, p7/m, p6/m, z8.h, z1.h
umopa foo.s, p6/m, p7/m, z4.b, z1.b
umops foo.s, p6/m, p7/m, z4.b, z1.b
usmopa foo.s, p4/m, p3/m, z4.b, z1.b
usmops foo.s, p6/m, p7/m, z4.b, z1.b
