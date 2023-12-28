#name: SME extension
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	c0902020 	addha	za0.s, p0/m, p1/m, z1.s
   4:	c0906841 	addha	za1.s, p2/m, p3/m, z2.s
   8:	c090b062 	addha	za2.s, p4/m, p5/m, z3.s
   c:	c090f883 	addha	za3.s, p6/m, p7/m, z4.s
  10:	c0912020 	addva	za0.s, p0/m, p1/m, z1.s
  14:	c0916841 	addva	za1.s, p2/m, p3/m, z2.s
  18:	c091b062 	addva	za2.s, p4/m, p5/m, z3.s
  1c:	c091f883 	addva	za3.s, p6/m, p7/m, z4.s
  20:	81842020 	bfmopa	za0.s, p0/m, p1/m, z1.h, z4.h
  24:	81836841 	bfmopa	za1.s, p2/m, p3/m, z2.h, z3.h
  28:	8182b062 	bfmopa	za2.s, p4/m, p5/m, z3.h, z2.h
  2c:	8181f883 	bfmopa	za3.s, p6/m, p7/m, z4.h, z1.h
  30:	81842030 	bfmops	za0.s, p0/m, p1/m, z1.h, z4.h
  34:	81836851 	bfmops	za1.s, p2/m, p3/m, z2.h, z3.h
  38:	8182b072 	bfmops	za2.s, p4/m, p5/m, z3.h, z2.h
  3c:	8181f893 	bfmops	za3.s, p6/m, p7/m, z4.h, z1.h
  40:	80842020 	fmopa	za0.s, p0/m, p1/m, z1.s, z4.s
  44:	80836841 	fmopa	za1.s, p2/m, p3/m, z2.s, z3.s
  48:	8082b062 	fmopa	za2.s, p4/m, p5/m, z3.s, z2.s
  4c:	8081f883 	fmopa	za3.s, p6/m, p7/m, z4.s, z1.s
  50:	81a42020 	fmopa	za0.s, p0/m, p1/m, z1.h, z4.h
  54:	81a36841 	fmopa	za1.s, p2/m, p3/m, z2.h, z3.h
  58:	81a2b062 	fmopa	za2.s, p4/m, p5/m, z3.h, z2.h
  5c:	81a1f883 	fmopa	za3.s, p6/m, p7/m, z4.h, z1.h
  60:	80842030 	fmops	za0.s, p0/m, p1/m, z1.s, z4.s
  64:	80836851 	fmops	za1.s, p2/m, p3/m, z2.s, z3.s
  68:	8082b072 	fmops	za2.s, p4/m, p5/m, z3.s, z2.s
  6c:	8081f893 	fmops	za3.s, p6/m, p7/m, z4.s, z1.s
  70:	80841c30 	fmops	za0.s, p7/m, p0/m, z1.s, z4.s
  74:	80833851 	fmops	za1.s, p6/m, p1/m, z2.s, z3.s
  78:	80825472 	fmops	za2.s, p5/m, p2/m, z3.s, z2.s
  7c:	80817093 	fmops	za3.s, p4/m, p3/m, z4.s, z1.s
  80:	80842030 	fmops	za0.s, p0/m, p1/m, z1.s, z4.s
  84:	80836851 	fmops	za1.s, p2/m, p3/m, z2.s, z3.s
  88:	8082b072 	fmops	za2.s, p4/m, p5/m, z3.s, z2.s
  8c:	8081f893 	fmops	za3.s, p6/m, p7/m, z4.s, z1.s
  90:	a0842020 	smopa	za0.s, p0/m, p1/m, z1.b, z4.b
  94:	a0836841 	smopa	za1.s, p2/m, p3/m, z2.b, z3.b
  98:	a082b062 	smopa	za2.s, p4/m, p5/m, z3.b, z2.b
  9c:	a081f883 	smopa	za3.s, p6/m, p7/m, z4.b, z1.b
  a0:	a0842030 	smops	za0.s, p0/m, p1/m, z1.b, z4.b
  a4:	a0836851 	smops	za1.s, p2/m, p3/m, z2.b, z3.b
  a8:	a082b072 	smops	za2.s, p4/m, p5/m, z3.b, z2.b
  ac:	a081f893 	smops	za3.s, p6/m, p7/m, z4.b, z1.b
  b0:	a0a42020 	sumopa	za0.s, p0/m, p1/m, z1.b, z4.b
  b4:	a0a36841 	sumopa	za1.s, p2/m, p3/m, z2.b, z3.b
  b8:	a0a2b062 	sumopa	za2.s, p4/m, p5/m, z3.b, z2.b
  bc:	a0a1f883 	sumopa	za3.s, p6/m, p7/m, z4.b, z1.b
  c0:	a0a42030 	sumops	za0.s, p0/m, p1/m, z1.b, z4.b
  c4:	a0a36851 	sumops	za1.s, p2/m, p3/m, z2.b, z3.b
  c8:	a0a2b072 	sumops	za2.s, p4/m, p5/m, z3.b, z2.b
  cc:	a0a1f893 	sumops	za3.s, p6/m, p7/m, z4.b, z1.b
  d0:	a0a41c30 	sumops	za0.s, p7/m, p0/m, z1.b, z4.b
  d4:	a0a33851 	sumops	za1.s, p6/m, p1/m, z2.b, z3.b
  d8:	a0a25472 	sumops	za2.s, p5/m, p2/m, z3.b, z2.b
  dc:	a0a17093 	sumops	za3.s, p4/m, p3/m, z4.b, z1.b
  e0:	a1a42020 	umopa	za0.s, p0/m, p1/m, z1.b, z4.b
  e4:	a1a36841 	umopa	za1.s, p2/m, p3/m, z2.b, z3.b
  e8:	a1a2b062 	umopa	za2.s, p4/m, p5/m, z3.b, z2.b
  ec:	a1a1f883 	umopa	za3.s, p6/m, p7/m, z4.b, z1.b
  f0:	a1a42030 	umops	za0.s, p0/m, p1/m, z1.b, z4.b
  f4:	a1a36851 	umops	za1.s, p2/m, p3/m, z2.b, z3.b
  f8:	a1a2b072 	umops	za2.s, p4/m, p5/m, z3.b, z2.b
  fc:	a1a1f893 	umops	za3.s, p6/m, p7/m, z4.b, z1.b
 100:	a1842020 	usmopa	za0.s, p0/m, p1/m, z1.b, z4.b
 104:	a1836841 	usmopa	za1.s, p2/m, p3/m, z2.b, z3.b
 108:	a182b062 	usmopa	za2.s, p4/m, p5/m, z3.b, z2.b
 10c:	a181f883 	usmopa	za3.s, p6/m, p7/m, z4.b, z1.b
 110:	a1841c20 	usmopa	za0.s, p7/m, p0/m, z1.b, z4.b
 114:	a1833841 	usmopa	za1.s, p6/m, p1/m, z2.b, z3.b
 118:	a1825462 	usmopa	za2.s, p5/m, p2/m, z3.b, z2.b
 11c:	a1817083 	usmopa	za3.s, p4/m, p3/m, z4.b, z1.b
 120:	a1842030 	usmops	za0.s, p0/m, p1/m, z1.b, z4.b
 124:	a1836851 	usmops	za1.s, p2/m, p3/m, z2.b, z3.b
 128:	a182b072 	usmops	za2.s, p4/m, p5/m, z3.b, z2.b
 12c:	a181f893 	usmops	za3.s, p6/m, p7/m, z4.b, z1.b
 130:	8181f883 	bfmopa	za3.s, p6/m, p7/m, z4.h, z1.h
 134:	8181f893 	bfmops	za3.s, p6/m, p7/m, z4.h, z1.h
 138:	81a1f883 	fmopa	za3.s, p6/m, p7/m, z4.h, z1.h
 13c:	8081f893 	fmops	za3.s, p6/m, p7/m, z4.s, z1.s
 140:	a1a1f883 	umopa	za3.s, p6/m, p7/m, z4.b, z1.b
 144:	a1a1f893 	umops	za3.s, p6/m, p7/m, z4.b, z1.b
 148:	a1817083 	usmopa	za3.s, p4/m, p3/m, z4.b, z1.b
 14c:	a181f893 	usmops	za3.s, p6/m, p7/m, z4.b, z1.b
[^:]+:	04605800 	addspl	x0, x0, #0
[^:]+:	04605801 	addspl	x1, x0, #0
[^:]+:	0460581f 	addspl	sp, x0, #0
[^:]+:	04625800 	addspl	x0, x2, #0
[^:]+:	047f5800 	addspl	x0, sp, #0
[^:]+:	04605be0 	addspl	x0, x0, #31
[^:]+:	04605c00 	addspl	x0, x0, #-32
[^:]+:	04605c20 	addspl	x0, x0, #-31
[^:]+:	04605fe0 	addspl	x0, x0, #-1
[^:]+:	04205800 	addsvl	x0, x0, #0
[^:]+:	04205801 	addsvl	x1, x0, #0
[^:]+:	0420581f 	addsvl	sp, x0, #0
[^:]+:	04225800 	addsvl	x0, x2, #0
[^:]+:	043f5800 	addsvl	x0, sp, #0
[^:]+:	04205be0 	addsvl	x0, x0, #31
[^:]+:	04205c00 	addsvl	x0, x0, #-32
[^:]+:	04205c20 	addsvl	x0, x0, #-31
[^:]+:	04205fe0 	addsvl	x0, x0, #-1
[^:]+:	04bf5800 	rdsvl	x0, #0
[^:]+:	04bf5801 	rdsvl	x1, #0
[^:]+:	04bf581f 	rdsvl	xzr, #0
[^:]+:	04bf5be0 	rdsvl	x0, #31
[^:]+:	04bf5c00 	rdsvl	x0, #-32
[^:]+:	04bf5c20 	rdsvl	x0, #-31
[^:]+:	04bf5fe0 	rdsvl	x0, #-1
