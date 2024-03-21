#name: SME F64 extension
#as: -march=armv8-a+sme-f64f64
#source: sme-f64.s
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	80c82020 	fmopa	za0.d, p0/m, p1/m, z1.d, z8.d
   4:	80c76841 	fmopa	za1.d, p2/m, p3/m, z2.d, z7.d
   8:	80c6b062 	fmopa	za2.d, p4/m, p5/m, z3.d, z6.d
   c:	80c5f883 	fmopa	za3.d, p6/m, p7/m, z4.d, z5.d
  10:	80c404a4 	fmopa	za4.d, p1/m, p0/m, z5.d, z4.d
  14:	80c34cc5 	fmopa	za5.d, p3/m, p2/m, z6.d, z3.d
  18:	80c294e6 	fmopa	za6.d, p5/m, p4/m, z7.d, z2.d
  1c:	80c1dd07 	fmopa	za7.d, p7/m, p6/m, z8.d, z1.d
  20:	80c41ca4 	fmopa	za4.d, p7/m, p0/m, z5.d, z4.d
  24:	80c338c5 	fmopa	za5.d, p6/m, p1/m, z6.d, z3.d
  28:	80c254e6 	fmopa	za6.d, p5/m, p2/m, z7.d, z2.d
  2c:	80c17107 	fmopa	za7.d, p4/m, p3/m, z8.d, z1.d
  30:	80c82030 	fmops	za0.d, p0/m, p1/m, z1.d, z8.d
  34:	80c76851 	fmops	za1.d, p2/m, p3/m, z2.d, z7.d
  38:	80c6b072 	fmops	za2.d, p4/m, p5/m, z3.d, z6.d
  3c:	80c5f893 	fmops	za3.d, p6/m, p7/m, z4.d, z5.d
  40:	80c404b4 	fmops	za4.d, p1/m, p0/m, z5.d, z4.d
  44:	80c34cd5 	fmops	za5.d, p3/m, p2/m, z6.d, z3.d
  48:	80c294f6 	fmops	za6.d, p5/m, p4/m, z7.d, z2.d
  4c:	80c1dd17 	fmops	za7.d, p7/m, p6/m, z8.d, z1.d
  50:	81a1f803 	fmopa	za3.s, p6/m, p7/m, z0.h, z1.h
  54:	8081f813 	fmops	za3.s, p6/m, p7/m, z0.s, z1.s
