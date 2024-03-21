/* Scalable Matrix Extension (SME).  */

/* ADDHA 32-bit variant.  */
addha za0.s, p0/m, p1/m, z1.s
addha za1.s, p2/m, p3/m, z2.s
addha za2.s, p4/m, p5/m, z3.s
addha za3.s, p6/m, p7/m, z4.s

/* ADDVA 32-bit variant.  */
addva za0.s, p0/m, p1/m, z1.s
addva za1.s, p2/m, p3/m, z2.s
addva za2.s, p4/m, p5/m, z3.s
addva za3.s, p6/m, p7/m, z4.s

/* BFMOPA.  */
bfmopa za0.s, p0/m, p1/m, z1.h, z4.h
bfmopa za1.s, p2/m, p3/m, z2.h, z3.h
bfmopa za2.s, p4/m, p5/m, z3.h, z2.h
bfmopa za3.s, p6/m, p7/m, z4.h, z1.h

/* BFMOPS.  */
bfmops za0.s, p0/m, p1/m, z1.h, z4.h
bfmops za1.s, p2/m, p3/m, z2.h, z3.h
bfmops za2.s, p4/m, p5/m, z3.h, z2.h
bfmops za3.s, p6/m, p7/m, z4.h, z1.h

/* FMOPA (non-widening), single-precision.  */
fmopa za0.s, p0/m, p1/m, z1.s, z4.s
fmopa za1.s, p2/m, p3/m, z2.s, z3.s
fmopa za2.s, p4/m, p5/m, z3.s, z2.s
fmopa za3.s, p6/m, p7/m, z4.s, z1.s

/* FMOPA (widening)  */
fmopa za0.s, p0/m, p1/m, z1.h, z4.h
fmopa za1.s, p2/m, p3/m, z2.h, z3.h
fmopa za2.s, p4/m, p5/m, z3.h, z2.h
fmopa za3.s, p6/m, p7/m, z4.h, z1.h

/* FMOPS (non-widening), single-precision.  */
fmops za0.s, p0/m, p1/m, z1.s, z4.s
fmops za1.s, p2/m, p3/m, z2.s, z3.s
fmops za2.s, p4/m, p5/m, z3.s, z2.s
fmops za3.s, p6/m, p7/m, z4.s, z1.s
fmops za0.s, p7/m, p0/m, z1.s, z4.s
fmops za1.s, p6/m, p1/m, z2.s, z3.s
fmops za2.s, p5/m, p2/m, z3.s, z2.s
fmops za3.s, p4/m, p3/m, z4.s, z1.s

/* FMOPS (widening)  */
fmops za0.s, p0/m, p1/m, z1.s, z4.s
fmops za1.s, p2/m, p3/m, z2.s, z3.s
fmops za2.s, p4/m, p5/m, z3.s, z2.s
fmops za3.s, p6/m, p7/m, z4.s, z1.s

/* SMOPA 32-bit variant.  */
smopa za0.s, p0/m, p1/m, z1.b, z4.b
smopa za1.s, p2/m, p3/m, z2.b, z3.b
smopa za2.s, p4/m, p5/m, z3.b, z2.b
smopa za3.s, p6/m, p7/m, z4.b, z1.b

/* SMOPS 32-bit variant.  */
smops za0.s, p0/m, p1/m, z1.b, z4.b
smops za1.s, p2/m, p3/m, z2.b, z3.b
smops za2.s, p4/m, p5/m, z3.b, z2.b
smops za3.s, p6/m, p7/m, z4.b, z1.b

/* SUMOPA 32-bit variant.  */
sumopa za0.s, p0/m, p1/m, z1.b, z4.b
sumopa za1.s, p2/m, p3/m, z2.b, z3.b
sumopa za2.s, p4/m, p5/m, z3.b, z2.b
sumopa za3.s, p6/m, p7/m, z4.b, z1.b

/* SUMOPS 32-bit variant.  */
sumops za0.s, p0/m, p1/m, z1.b, z4.b
sumops za1.s, p2/m, p3/m, z2.b, z3.b
sumops za2.s, p4/m, p5/m, z3.b, z2.b
sumops za3.s, p6/m, p7/m, z4.b, z1.b
sumops za0.s, p7/m, p0/m, z1.b, z4.b
sumops za1.s, p6/m, p1/m, z2.b, z3.b
sumops za2.s, p5/m, p2/m, z3.b, z2.b
sumops za3.s, p4/m, p3/m, z4.b, z1.b

/* UMOPA 32-bit variant.  */
umopa za0.s, p0/m, p1/m, z1.b, z4.b
umopa za1.s, p2/m, p3/m, z2.b, z3.b
umopa za2.s, p4/m, p5/m, z3.b, z2.b
umopa za3.s, p6/m, p7/m, z4.b, z1.b

/* UMOPS 32-bit variant.  */
umops za0.s, p0/m, p1/m, z1.b, z4.b
umops za1.s, p2/m, p3/m, z2.b, z3.b
umops za2.s, p4/m, p5/m, z3.b, z2.b
umops za3.s, p6/m, p7/m, z4.b, z1.b

/* USMOPA 32-bit variant.  */
usmopa za0.s, p0/m, p1/m, z1.b, z4.b
usmopa za1.s, p2/m, p3/m, z2.b, z3.b
usmopa za2.s, p4/m, p5/m, z3.b, z2.b
usmopa za3.s, p6/m, p7/m, z4.b, z1.b
usmopa za0.s, p7/m, p0/m, z1.b, z4.b
usmopa za1.s, p6/m, p1/m, z2.b, z3.b
usmopa za2.s, p5/m, p2/m, z3.b, z2.b
usmopa za3.s, p4/m, p3/m, z4.b, z1.b

/* USMOPS 32-bit variant.  */
usmops za0.s, p0/m, p1/m, z1.b, z4.b
usmops za1.s, p2/m, p3/m, z2.b, z3.b
usmops za2.s, p4/m, p5/m, z3.b, z2.b
usmops za3.s, p6/m, p7/m, z4.b, z1.b

/* Register aliases.  */
foo .req za3
bar .req za7
baz .req za0

bfmopa foo.s, p6/m, p7/m, z4.h, z1.h
bfmops foo.s, p6/m, p7/m, z4.h, z1.h
fmopa foo.s, p6/m, p7/m, z4.h, z1.h
fmops foo.s, p6/m, p7/m, z4.s, z1.s
umopa foo.s, p6/m, p7/m, z4.b, z1.b
umops foo.s, p6/m, p7/m, z4.b, z1.b
usmopa foo.s, p4/m, p3/m, z4.b, z1.b
usmops foo.s, p6/m, p7/m, z4.b, z1.b

/* ADDSPL.  */
addspl x0, x0, #0
addspl x1, x0, #0
addspl sp, x0, #0
addspl x0, x2, #0
addspl x0, sp, #0
addspl x0, x0, #31
addspl x0, x0, #-32
addspl x0, x0, #-31
addspl x0, x0, #-1

/* ADDSVL.  */
addsvl x0, x0, #0
addsvl x1, x0, #0
addsvl sp, x0, #0
addsvl x0, x2, #0
addsvl x0, sp, #0
addsvl x0, x0, #31
addsvl x0, x0, #-32
addsvl x0, x0, #-31
addsvl x0, x0, #-1

/* RDSVL.  */
rdsvl x0, #0
rdsvl x1, #0
rdsvl xzr, #0
rdsvl x0, #31
rdsvl x0, #-32
rdsvl x0, #-31
rdsvl x0, #-1
