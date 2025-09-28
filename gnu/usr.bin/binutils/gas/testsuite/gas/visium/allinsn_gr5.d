#as: -mtune=gr5
#objdump: -dzr
#name: allinsn_gr5

.*: +file format .*

Disassembly of section .text:

0+0000000 <begin>:
   0:	03 e2 00 14 	write.l 0\(r2\),r1
   4:	03 e2 00 14 	write.l 0\(r2\),r1
   8:	83 e1 04 22 	write.w 1\(r1\),r2
   c:	03 e3 7c 71 	write.b 31\(r3\),r7
  10:	03 e4 00 71 	write.b 0\(r4\),r7
  14:	03 e4 80 54 	mults   r4,r5
  18:	83 e7 fc a4 	eamwrite 31,r7,r10
  1c:	83 ee 90 f4 	writemd r14,r15
  20:	83 e9 94 04 	writemdc r9
  24:	03 e5 88 04 	divs    r5
  28:	83 e6 8c 04 	divu    r6
  2c:	83 ea 98 04 	divds   r10
  30:	83 eb 9c 04 	divdu   r11
  34:	83 ec a4 04 	asrd    r12
  38:	03 ed a8 04 	lsrd    r13
  3c:	83 ee ac 04 	asld    r14
  40:	82 80 00 04 	dsi
  44:	83 e7 80 84 	mults   r7,r8
  48:	03 e9 84 a4 	multu   r9,r10
  4c:	02 a0 00 04 	eni
  50:	82 80 00 04 	dsi
  54:	82 fe 01 d4 	rfi

0+0000058 <nsrel>:
  58:	00 00 00 00 	nop
  5c:	07 a0 00 04 	rflag   r0
  60:	08 00 ff fe 	brr     eq,-2
  64:	07 a0 00 04 	rflag   r0
  68:	90 00 ff fc 	brr     cs,-4
  6c:	07 a0 00 04 	rflag   r0
  70:	18 00 ff fa 	brr     os,-6
  74:	07 a0 00 04 	rflag   r0
  78:	20 00 00 1c 	brr     ns,\+28
  7c:	07 a0 00 04 	rflag   r0
  80:	a8 00 00 1a 	brr     ne,\+26
  84:	07 a0 00 04 	rflag   r0
  88:	30 00 00 18 	brr     cc,\+24
  8c:	07 a0 00 04 	rflag   r0
  90:	38 00 00 16 	brr     oc,\+22
  94:	07 a0 00 04 	rflag   r0
  98:	c0 00 00 14 	brr     nc,\+20
  9c:	07 a0 00 04 	rflag   r0
  a0:	48 00 00 12 	brr     ge,\+18
  a4:	07 a0 00 04 	rflag   r0
  a8:	d0 00 00 10 	brr     gt,\+16
  ac:	07 a0 00 04 	rflag   r0
  b0:	58 00 00 0e 	brr     hi,\+14
  b4:	07 a0 00 04 	rflag   r0
  b8:	60 00 00 0c 	brr     le,\+12
  bc:	07 a0 00 04 	rflag   r0
  c0:	e8 00 00 0a 	brr     ls,\+10
  c4:	07 a0 00 04 	rflag   r0
  c8:	70 00 00 08 	brr     lt,\+8
  cc:	07 a0 00 04 	rflag   r0
  d0:	78 00 00 06 	brr     tr,\+6
  d4:	07 a0 00 04 	rflag   r0
  d8:	08 00 ff e0 	brr     eq,-32
  dc:	00 00 00 00 	nop
  e0:	00 00 00 00 	nop
  e4:	00 00 00 00 	nop

0+00000e8 <sreg>:
  e8:	86 20 00 14 	adc.l   r0,r0,r1
  ec:	86 20 08 32 	adc.w   r2,r0,r3
  f0:	86 20 10 51 	adc.b   r4,r0,r5
  f4:	86 00 08 14 	add.l   r2,r0,r1
  f8:	06 04 14 32 	add.w   r5,r4,r3
  fc:	86 07 1c 61 	add.b   r7,r7,r6
 100:	87 40 08 14 	and.l   r2,r0,r1
 104:	07 44 14 32 	and.w   r5,r4,r3
 108:	87 47 1c 61 	and.b   r7,r7,r6
 10c:	06 e3 10 44 	asl.l   r4,r3,r4
 110:	86 e5 1a 02 	asl.w   r6,r5,0
 114:	06 e5 1a 12 	asl.w   r6,r5,1
 118:	06 e7 23 f1 	asl.b   r8,r7,31
 11c:	86 a3 10 44 	asr.l   r4,r3,r4
 120:	06 a5 1a 02 	asr.w   r6,r5,0
 124:	86 a5 1a 12 	asr.w   r6,r5,1
 128:	86 a7 23 f1 	asr.b   r8,r7,31
 12c:	0f 89 28 04 	bra     eq,r9,r10
 130:	07 a0 00 04 	rflag   r0
 134:	af 87 04 04 	bra     ne,r7,r1
 138:	07 e0 ae 04 	readmda r11
 13c:	07 e0 b3 f4 	eamread r12,31
 140:	87 cd 30 04 	extb.l  r12,r13
 144:	87 cf 38 02 	extb.w  r14,r15
 148:	87 c1 00 01 	extb.b  r0,r1
 14c:	86 83 08 04 	extw.l  r2,r3
 150:	86 85 10 02 	extw.w  r4,r5
 154:	86 c7 18 84 	lsr.l   r6,r7,r8
 158:	06 ca 26 02 	lsr.w   r9,r10,0
 15c:	86 ca 26 12 	lsr.w   r9,r10,1
 160:	86 ca 27 f1 	lsr.b   r9,r10,31
 164:	87 6c 2c 04 	not.l   r11,r12
 168:	07 6e 34 02 	not.w   r13,r14
 16c:	07 6a 3c 01 	not.b   r15,r10
 170:	07 26 14 74 	or.l    r5,r6,r7
 174:	07 29 20 a2 	or.w    r8,r9,r10
 178:	87 22 04 31 	or.b    r1,r2,r3
 17c:	87 e5 12 04 	read.l  r4,0\(r5\)
 180:	87 e5 12 04 	read.l  r4,0\(r5\)
 184:	07 e7 1a 12 	read.w  r6,1\(r7\)
 188:	07 e9 23 f1 	read.b  r8,31\(r9\)
 18c:	87 e9 1a 11 	read.b  r6,1\(r9\)
 190:	87 e0 aa 04 	readmda r10
 194:	87 e0 ae 14 	readmdb r11
 198:	07 e0 c6 24 	readmdc r17
 19c:	87 a0 10 04 	rflag   r4
 1a0:	87 a0 1c 04 	rflag   r7
 1a4:	86 45 10 64 	sub.l   r4,r5,r6
 1a8:	06 48 1c 92 	sub.w   r7,r8,r9
 1ac:	06 41 00 21 	cmp.b   r1,r2
 1b0:	06 65 10 64 	subc.l  r4,r5,r6
 1b4:	86 68 1c 92 	subc.w  r7,r8,r9
 1b8:	86 61 00 21 	cmpc.b  r1,r2
 1bc:	07 03 10 24 	xor.l   r4,r3,r2
 1c0:	87 06 14 72 	xor.w   r5,r6,r7
 1c4:	07 09 04 81 	xor.b   r1,r9,r8
 1c8:	04 07 ff ff 	addi    r7,65535
 1cc:	04 87 80 00 	movil   r7,0x8000
 1d0:	84 a7 7f ff 	moviu   r7,0x7FFF
 1d4:	04 c6 00 01 	moviq   r6,1
 1d8:	84 47 ff ff 	subi    r7,65535
 1dc:	ff 86 00 04 	bra     tr,r6,r0
 1e0:	86 00 00 04 	add.l   r0,r0,r0
 1e4:	d3 e3 84 5c 	fpinst  10,f1,f3,f5
 1e8:	db e4 88 6c 	fpinst  11,f2,f4,f6
 1ec:	7b ed ac fc 	fpinst  15,f11,f13,f15
 1f0:	8f ef e6 ec 	fpuread 1,r25,f15,f14
 1f4:	3b e3 9c 0c 	fabs    f7,f3
 1f8:	0b e6 b0 ec 	fadd    f12,f6,f14
 1fc:	8b e6 b0 0c 	fadd    f12,f6,f0
 200:	63 e6 b0 0c 	fmove   f12,f6
 204:	b3 e3 9c 0c 	fneg    f7,f3
 208:	93 e0 8c 9c 	fsub    f3,f0,f9
 20c:	1b e2 84 3c 	fmult   f1,f2,f3
 210:	23 eb a8 cc 	fdiv    f10,f11,f12
 214:	2b e9 8c 0c 	fsqrt   f3,f9
 218:	43 e4 94 0c 	ftoi    f5,f4
 21c:	4b e8 9c 0c 	itof    f7,f8
 220:	03 ff b4 0c 	fload   f13,r31
 224:	07 e7 e6 0c 	fstore  r25,f7
 228:	d7 ef 8a 0c 	fcmp    r2,f15,f0
 22c:	df ef 86 1c 	fcmpe   r1,f15,f1
