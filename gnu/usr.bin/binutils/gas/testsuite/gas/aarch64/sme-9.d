#name: SVE2 instructions added to support SME
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	25277c61 	psel	p1, p15, p3.b\[w15, 0\]
   4:	252778a2 	psel	p2, p14, p5.b\[w15, 0\]
   8:	257f74e3 	psel	p3, p13, p7.b\[w15, 7\]
   c:	25ff7125 	psel	p5, p12, p9.b\[w15, 15\]
  10:	252a6de8 	psel	p8, p11, p15.h\[w14, 0\]
  14:	252a682d 	psel	p13, p10, p1.h\[w14, 0\]
  18:	257a640f 	psel	p15, p9, p0.h\[w14, 3\]
  1c:	25fa60c1 	psel	p1, p8, p6.h\[w14, 7\]
  20:	25315de2 	psel	p2, p7, p15.s\[w13, 0\]
  24:	253159e3 	psel	p3, p6, p15.s\[w13, 0\]
  28:	257155e5 	psel	p5, p5, p15.s\[w13, 1\]
  2c:	25f151e8 	psel	p8, p4, p15.s\[w13, 3\]
  30:	25604c2d 	psel	p13, p3, p1.d\[w12, 0\]
  34:	2560482f 	psel	p15, p2, p1.d\[w12, 0\]
  38:	25e04421 	psel	p1, p1, p1.d\[w12, 1\]
  3c:	052e8000 	revd	z0.q, p0/m, z0.q
  40:	052e9c00 	revd	z0.q, p7/m, z0.q
  44:	052e83e0 	revd	z0.q, p0/m, z31.q
  48:	052e9c1f 	revd	z31.q, p7/m, z0.q
  4c:	4411c3e0 	sclamp	z0.b, z31.b, z17.b
  50:	4411c01f 	sclamp	z31.b, z0.b, z17.b
  54:	441fc028 	sclamp	z8.b, z1.b, z31.b
  58:	4451c01f 	sclamp	z31.h, z0.h, z17.h
  5c:	445fc028 	sclamp	z8.h, z1.h, z31.h
  60:	4491c3e0 	sclamp	z0.s, z31.s, z17.s
  64:	4491c01f 	sclamp	z31.s, z0.s, z17.s
  68:	449fc028 	sclamp	z8.s, z1.s, z31.s
  6c:	44d1c3e0 	sclamp	z0.d, z31.d, z17.d
  70:	44d1c01f 	sclamp	z31.d, z0.d, z17.d
  74:	44dfc028 	sclamp	z8.d, z1.d, z31.d
  78:	4411c7e0 	uclamp	z0.b, z31.b, z17.b
  7c:	4411c41f 	uclamp	z31.b, z0.b, z17.b
  80:	441fc428 	uclamp	z8.b, z1.b, z31.b
  84:	4451c7e0 	uclamp	z0.h, z31.h, z17.h
  88:	4451c41f 	uclamp	z31.h, z0.h, z17.h
  8c:	445fc428 	uclamp	z8.h, z1.h, z31.h
  90:	4491c7e0 	uclamp	z0.s, z31.s, z17.s
  94:	4491c41f 	uclamp	z31.s, z0.s, z17.s
  98:	449fc428 	uclamp	z8.s, z1.s, z31.s
  9c:	44d1c7e0 	uclamp	z0.d, z31.d, z17.d
  a0:	44d1c41f 	uclamp	z31.d, z0.d, z17.d
  a4:	44dfc428 	uclamp	z8.d, z1.d, z31.d
  a8:	0420bca3 	movprfx	z3, z5
  ac:	052e84a3 	revd	z3.q, p1/m, z5.q
  b0:	0420bc81 	movprfx	z1, z4
  b4:	052e84a1 	revd	z1.q, p1/m, z5.q
  b8:	0420bc81 	movprfx	z1, z4
  bc:	440bc141 	sclamp	z1.b, z10.b, z11.b
  c0:	0420bc82 	movprfx	z2, z4
  c4:	444bc142 	sclamp	z2.h, z10.h, z11.h
  c8:	0420bc83 	movprfx	z3, z4
  cc:	448bc143 	sclamp	z3.s, z10.s, z11.s
  d0:	0420bca4 	movprfx	z4, z5
  d4:	44cbc144 	sclamp	z4.d, z10.d, z11.d
  d8:	0420bc81 	movprfx	z1, z4
  dc:	440bc541 	uclamp	z1.b, z10.b, z11.b
  e0:	0420bc82 	movprfx	z2, z4
  e4:	444bc542 	uclamp	z2.h, z10.h, z11.h
  e8:	0420bc83 	movprfx	z3, z4
  ec:	448bc543 	uclamp	z3.s, z10.s, z11.s
  f0:	0420bca4 	movprfx	z4, z5
  f4:	44cbc544 	uclamp	z4.d, z10.d, z11.d
  f8:	25277c61 	psel	p1, p15, p3.b\[w15, 0\]
  fc:	252778a2 	psel	p2, p14, p5.b\[w15, 0\]
 100:	25244200 	\.inst	0x25244200 ; undefined
 104:	25244010 	whilege	pn8.b, x0, x4, vlx2
 108:	25244210 	whilege	pn8.b, x16, x4, vlx2
