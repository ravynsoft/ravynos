/* Scalable Matrix Extension (SME F64).  */

/* FMOPA (non-widening), double-precision.  */
fmopa za0.d, p0/m, p1/m, z1.d, z8.d
fmopa za1.d, p2/m, p3/m, z2.d, z7.d
fmopa za2.d, p4/m, p5/m, z3.d, z6.d
fmopa za3.d, p6/m, p7/m, z4.d, z5.d
fmopa za4.d, p1/m, p0/m, z5.d, z4.d
fmopa za5.d, p3/m, p2/m, z6.d, z3.d
fmopa za6.d, p5/m, p4/m, z7.d, z2.d
fmopa za7.d, p7/m, p6/m, z8.d, z1.d
fmopa za4.d, p7/m, p0/m, z5.d, z4.d
fmopa za5.d, p6/m, p1/m, z6.d, z3.d
fmopa za6.d, p5/m, p2/m, z7.d, z2.d
fmopa za7.d, p4/m, p3/m, z8.d, z1.d

/* FMOPS (non-widening), double-precision.  */
fmops za0.d, p0/m, p1/m, z1.d, z8.d
fmops za1.d, p2/m, p3/m, z2.d, z7.d
fmops za2.d, p4/m, p5/m, z3.d, z6.d
fmops za3.d, p6/m, p7/m, z4.d, z5.d
fmops za4.d, p1/m, p0/m, z5.d, z4.d
fmops za5.d, p3/m, p2/m, z6.d, z3.d
fmops za6.d, p5/m, p4/m, z7.d, z2.d
fmops za7.d, p7/m, p6/m, z8.d, z1.d

/* Register aliases.  */
foo .req za3
bar .req z0

fmopa foo.s, p6/m, p7/m, bar.h, z1.h
fmops foo.s, p6/m, p7/m, bar.s, z1.s
