#name: SME I64 extension
#as: -march=armv8-a+sme-i64
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	c0d02020 	addha	za0.d, p0/m, p1/m, z1.d
   4:	c0d06841 	addha	za1.d, p2/m, p3/m, z2.d
   8:	c0d0b062 	addha	za2.d, p4/m, p5/m, z3.d
   c:	c0d0f883 	addha	za3.d, p6/m, p7/m, z4.d
  10:	c0d004a4 	addha	za4.d, p1/m, p0/m, z5.d
  14:	c0d04cc5 	addha	za5.d, p3/m, p2/m, z6.d
  18:	c0d094e6 	addha	za6.d, p5/m, p4/m, z7.d
  1c:	c0d0dd07 	addha	za7.d, p7/m, p6/m, z8.d
  20:	c0d01ca4 	addha	za4.d, p7/m, p0/m, z5.d
  24:	c0d038c5 	addha	za5.d, p6/m, p1/m, z6.d
  28:	c0d054e6 	addha	za6.d, p5/m, p2/m, z7.d
  2c:	c0d07107 	addha	za7.d, p4/m, p3/m, z8.d
  30:	c0d12020 	addva	za0.d, p0/m, p1/m, z1.d
  34:	c0d16841 	addva	za1.d, p2/m, p3/m, z2.d
  38:	c0d1b062 	addva	za2.d, p4/m, p5/m, z3.d
  3c:	c0d1f883 	addva	za3.d, p6/m, p7/m, z4.d
  40:	c0d104a4 	addva	za4.d, p1/m, p0/m, z5.d
  44:	c0d14cc5 	addva	za5.d, p3/m, p2/m, z6.d
  48:	c0d194e6 	addva	za6.d, p5/m, p4/m, z7.d
  4c:	c0d1dd07 	addva	za7.d, p7/m, p6/m, z8.d
  50:	c0d11ca4 	addva	za4.d, p7/m, p0/m, z5.d
  54:	c0d138c5 	addva	za5.d, p6/m, p1/m, z6.d
  58:	c0d154e6 	addva	za6.d, p5/m, p2/m, z7.d
  5c:	c0d17107 	addva	za7.d, p4/m, p3/m, z8.d
  60:	a0c82020 	smopa	za0.d, p0/m, p1/m, z1.h, z8.h
  64:	a0c76841 	smopa	za1.d, p2/m, p3/m, z2.h, z7.h
  68:	a0c6b062 	smopa	za2.d, p4/m, p5/m, z3.h, z6.h
  6c:	a0c5f883 	smopa	za3.d, p6/m, p7/m, z4.h, z5.h
  70:	a0c404a4 	smopa	za4.d, p1/m, p0/m, z5.h, z4.h
  74:	a0c34cc5 	smopa	za5.d, p3/m, p2/m, z6.h, z3.h
  78:	a0c294e6 	smopa	za6.d, p5/m, p4/m, z7.h, z2.h
  7c:	a0c1dd07 	smopa	za7.d, p7/m, p6/m, z8.h, z1.h
  80:	a0c82030 	smops	za0.d, p0/m, p1/m, z1.h, z8.h
  84:	a0c76851 	smops	za1.d, p2/m, p3/m, z2.h, z7.h
  88:	a0c6b072 	smops	za2.d, p4/m, p5/m, z3.h, z6.h
  8c:	a0c5f893 	smops	za3.d, p6/m, p7/m, z4.h, z5.h
  90:	a0c404b4 	smops	za4.d, p1/m, p0/m, z5.h, z4.h
  94:	a0c34cd5 	smops	za5.d, p3/m, p2/m, z6.h, z3.h
  98:	a0c294f6 	smops	za6.d, p5/m, p4/m, z7.h, z2.h
  9c:	a0c1dd17 	smops	za7.d, p7/m, p6/m, z8.h, z1.h
  a0:	a0c41cb4 	smops	za4.d, p7/m, p0/m, z5.h, z4.h
  a4:	a0c338d5 	smops	za5.d, p6/m, p1/m, z6.h, z3.h
  a8:	a0c254f6 	smops	za6.d, p5/m, p2/m, z7.h, z2.h
  ac:	a0c17117 	smops	za7.d, p4/m, p3/m, z8.h, z1.h
  b0:	a0e82020 	sumopa	za0.d, p0/m, p1/m, z1.h, z8.h
  b4:	a0e76841 	sumopa	za1.d, p2/m, p3/m, z2.h, z7.h
  b8:	a0e6b062 	sumopa	za2.d, p4/m, p5/m, z3.h, z6.h
  bc:	a0e5f883 	sumopa	za3.d, p6/m, p7/m, z4.h, z5.h
  c0:	a0e404a4 	sumopa	za4.d, p1/m, p0/m, z5.h, z4.h
  c4:	a0e34cc5 	sumopa	za5.d, p3/m, p2/m, z6.h, z3.h
  c8:	a0e294e6 	sumopa	za6.d, p5/m, p4/m, z7.h, z2.h
  cc:	a0e1dd07 	sumopa	za7.d, p7/m, p6/m, z8.h, z1.h
  d0:	a0e82030 	sumops	za0.d, p0/m, p1/m, z1.h, z8.h
  d4:	a0e76851 	sumops	za1.d, p2/m, p3/m, z2.h, z7.h
  d8:	a0e6b072 	sumops	za2.d, p4/m, p5/m, z3.h, z6.h
  dc:	a0e5f893 	sumops	za3.d, p6/m, p7/m, z4.h, z5.h
  e0:	a0e404b4 	sumops	za4.d, p1/m, p0/m, z5.h, z4.h
  e4:	a0e34cd5 	sumops	za5.d, p3/m, p2/m, z6.h, z3.h
  e8:	a0e294f6 	sumops	za6.d, p5/m, p4/m, z7.h, z2.h
  ec:	a0e1dd17 	sumops	za7.d, p7/m, p6/m, z8.h, z1.h
  f0:	a1e82020 	umopa	za0.d, p0/m, p1/m, z1.h, z8.h
  f4:	a1e76841 	umopa	za1.d, p2/m, p3/m, z2.h, z7.h
  f8:	a1e6b062 	umopa	za2.d, p4/m, p5/m, z3.h, z6.h
  fc:	a1e5f883 	umopa	za3.d, p6/m, p7/m, z4.h, z5.h
 100:	a1e404a4 	umopa	za4.d, p1/m, p0/m, z5.h, z4.h
 104:	a1e34cc5 	umopa	za5.d, p3/m, p2/m, z6.h, z3.h
 108:	a1e294e6 	umopa	za6.d, p5/m, p4/m, z7.h, z2.h
 10c:	a1e1dd07 	umopa	za7.d, p7/m, p6/m, z8.h, z1.h
 110:	a1e82030 	umops	za0.d, p0/m, p1/m, z1.h, z8.h
 114:	a1e76851 	umops	za1.d, p2/m, p3/m, z2.h, z7.h
 118:	a1e6b072 	umops	za2.d, p4/m, p5/m, z3.h, z6.h
 11c:	a1e5f893 	umops	za3.d, p6/m, p7/m, z4.h, z5.h
 120:	a1e404b4 	umops	za4.d, p1/m, p0/m, z5.h, z4.h
 124:	a1e34cd5 	umops	za5.d, p3/m, p2/m, z6.h, z3.h
 128:	a1e294f6 	umops	za6.d, p5/m, p4/m, z7.h, z2.h
 12c:	a1e1dd17 	umops	za7.d, p7/m, p6/m, z8.h, z1.h
 130:	a1c82020 	usmopa	za0.d, p0/m, p1/m, z1.h, z8.h
 134:	a1c76841 	usmopa	za1.d, p2/m, p3/m, z2.h, z7.h
 138:	a1c6b062 	usmopa	za2.d, p4/m, p5/m, z3.h, z6.h
 13c:	a1c5f883 	usmopa	za3.d, p6/m, p7/m, z4.h, z5.h
 140:	a1c404a4 	usmopa	za4.d, p1/m, p0/m, z5.h, z4.h
 144:	a1c34cc5 	usmopa	za5.d, p3/m, p2/m, z6.h, z3.h
 148:	a1c294e6 	usmopa	za6.d, p5/m, p4/m, z7.h, z2.h
 14c:	a1c1dd07 	usmopa	za7.d, p7/m, p6/m, z8.h, z1.h
 150:	a1c82030 	usmops	za0.d, p0/m, p1/m, z1.h, z8.h
 154:	a1c76851 	usmops	za1.d, p2/m, p3/m, z2.h, z7.h
 158:	a1c6b072 	usmops	za2.d, p4/m, p5/m, z3.h, z6.h
 15c:	a1c5f893 	usmops	za3.d, p6/m, p7/m, z4.h, z5.h
 160:	a1c404b4 	usmops	za4.d, p1/m, p0/m, z5.h, z4.h
 164:	a1c34cd5 	usmops	za5.d, p3/m, p2/m, z6.h, z3.h
 168:	a1c294f6 	usmops	za6.d, p5/m, p4/m, z7.h, z2.h
 16c:	a1c1dd17 	usmops	za7.d, p7/m, p6/m, z8.h, z1.h
 170:	a1c41cb4 	usmops	za4.d, p7/m, p0/m, z5.h, z4.h
 174:	a1c338d5 	usmops	za5.d, p6/m, p1/m, z6.h, z3.h
 178:	a1c254f6 	usmops	za6.d, p5/m, p2/m, z7.h, z2.h
 17c:	a1c17117 	usmops	za7.d, p4/m, p3/m, z8.h, z1.h
 180:	c0d02020 	addha	za0.d, p0/m, p1/m, z1.d
 184:	c0d17107 	addva	za7.d, p4/m, p3/m, z8.d
 188:	8181f883 	bfmopa	za3.s, p6/m, p7/m, z4.h, z1.h
 18c:	8181f893 	bfmops	za3.s, p6/m, p7/m, z4.h, z1.h
 190:	a0c1dd07 	smopa	za7.d, p7/m, p6/m, z8.h, z1.h
 194:	a0c17117 	smops	za7.d, p4/m, p3/m, z8.h, z1.h
 198:	a0e1dd07 	sumopa	za7.d, p7/m, p6/m, z8.h, z1.h
 19c:	a0e1dd17 	sumops	za7.d, p7/m, p6/m, z8.h, z1.h
 1a0:	a1a1f883 	umopa	za3.s, p6/m, p7/m, z4.b, z1.b
 1a4:	a1a1f893 	umops	za3.s, p6/m, p7/m, z4.b, z1.b
 1a8:	a1817083 	usmopa	za3.s, p4/m, p3/m, z4.b, z1.b
 1ac:	a181f893 	usmops	za3.s, p6/m, p7/m, z4.b, z1.b
