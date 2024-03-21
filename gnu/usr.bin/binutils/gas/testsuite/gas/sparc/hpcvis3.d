#as: -Av9v
#objdump: -dr
#name: sparc HPC+VIS3

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	01 00 00 00 	nop 
   4:	01 00 00 00 	nop 
   8:	c7 08 c0 00 	ldx  \[ %g3 \], %efsr
   c:	01 00 00 00 	nop 
  10:	01 00 00 00 	nop 
  14:	87 a0 4a 22 	fnadds  %f1, %f2, %f3
  18:	8d a0 8a 44 	fnaddd  %f2, %f4, %f6
  1c:	8f a0 cb 25 	fnmuls  %f3, %f5, %f7
  20:	95 a1 8b 48 	fnmuld  %f6, %f8, %f10
  24:	97 a1 cc 29 	fhadds  %f7, %f9, %f11
  28:	99 a2 0c 4a 	fhaddd  %f8, %f10, %f12
  2c:	9b a2 4c ab 	fhsubs  %f9, %f11, %f13
  30:	9d a2 8c cc 	fhsubd  %f10, %f12, %f14
  34:	9f a2 ce 2d 	fnhadds  %f11, %f13, %f15
  38:	a1 a3 0e 4e 	fnhaddd  %f12, %f14, %f16
  3c:	a1 a3 4f 2f 	fnsmuld  %f13, %f15, %f16
  40:	ab bb e6 31 	fmadds  %f15, %f17, %f19, %f21
  44:	a9 bb a4 50 	fmaddd  %f14, %f16, %f18, %f20
  48:	af bc 6a b3 	fmsubs  %f17, %f19, %f21, %f23
  4c:	ad bc 28 d2 	fmsubd  %f16, %f18, %f20, %f22
  50:	b3 bc ef 35 	fnmsubs  %f19, %f21, %f23, %f25
  54:	b1 bc ad 54 	fnmsubd  %f18, %f20, %f22, %f24
  58:	b7 bd 73 b7 	fnmadds  %f21, %f23, %f25, %f27
  5c:	b5 bd 31 d6 	fnmaddd  %f20, %f22, %f24, %f26
  60:	bb fd f6 39 	fumadds  %f23, %f25, %f27, %f29
  64:	b9 fd b4 58 	fumaddd  %f22, %f24, %f26, %f28
  68:	bf fe 7a bb 	fumsubs  %f25, %f27, %f29, %f31
  6c:	bd fe 38 da 	fumsubd  %f24, %f26, %f28, %f30
  70:	8f f8 4b 23 	fnumsubs  %f1, %f3, %f5, %f7
  74:	91 f8 8d 44 	fnumsubd  %f2, %f4, %f6, %f8
  78:	93 f8 cf a5 	fnumadds  %f3, %f5, %f7, %f9
  7c:	95 f9 11 c6 	fnumaddd  %f4, %f6, %f8, %f10
  80:	8f b1 42 26 	addxc  %g5, %g6, %g7
  84:	97 b2 42 6a 	addxccc  %o1, %o2, %o3
  88:	01 00 00 00 	nop 
  8c:	9f b3 42 ce 	umulxhi  %o5, %sp, %o7
  90:	b5 b0 02 f9 	lzcnt  %i1, %i2
  94:	81 b0 03 7b 	cmask8  %i3
  98:	81 b0 03 bc 	cmask16  %i4
  9c:	81 b0 03 fd 	cmask32  %i5
  a0:	8b b0 44 23 	fsll16  %f32, %f34, %f36
  a4:	8f b0 c4 65 	fsrl16  %f34, %f36, %f38
  a8:	93 b1 44 a7 	fsll32  %f36, %f38, %f40
  ac:	97 b1 c4 e9 	fsrl32  %f38, %f40, %f42
  b0:	9b b2 45 2b 	fslas16  %f40, %f42, %f44
  b4:	9f b2 c5 6d 	fsra16  %f42, %f44, %f46
  b8:	a3 b3 45 af 	fslas32  %f44, %f46, %f48
  bc:	a7 b3 c5 f1 	fsra32  %f46, %f48, %f50
  c0:	83 b4 47 f3 	pdistn  %f48, %f50, %g1
  c4:	af b4 c8 15 	fmean16  %f50, %f52, %f54
  c8:	b3 b5 48 57 	fpadd64  %f52, %f54, %f56
  cc:	b7 b5 c8 99 	fchksm16  %f54, %f56, %f58
  d0:	bb b6 48 db 	fpsub64  %f56, %f58, %f60
  d4:	bf b6 cb 1d 	fpadds16  %f58, %f60, %f62
  d8:	8d b0 8b 24 	fpadds16s  %f2, %f4, %f6
  dc:	91 b1 0b 46 	fpadds32  %f4, %f6, %f8
  e0:	95 b1 8b 68 	fpadds32s  %f6, %f8, %f10
  e4:	99 b2 0b 8a 	fpsubs16  %f8, %f10, %f12
  e8:	9d b2 8b ac 	fpsubs16s  %f10, %f12, %f14
  ec:	a1 b3 0b ce 	fpsubs32  %f12, %f14, %f16
  f0:	a5 b3 8b f0 	fpsubs32s  %f14, %f16, %f18
  f4:	83 b0 22 14 	movdtox  %f20, %g1
  f8:	85 b0 22 35 	movstouw  %f21, %g2
  fc:	87 b0 22 77 	movstosw  %f23, %g3
 100:	ad b0 23 04 	movxtod  %g4, %f22
 104:	af b0 23 25 	movwtos  %g5, %f23
 108:	97 b2 62 aa 	xmulx  %o1, %o2, %o3
 10c:	9d b3 22 cd 	xmulxhi  %o4, %o5, %sp
 110:	83 b4 24 12 	fpcmpule8  %f16, %f18, %g1
 114:	85 b4 a4 54 	fpcmpune8  %f18, %f20, %g2
 118:	87 b5 25 16 	fpcmpugt8  %f20, %f22, %g3
 11c:	89 b5 a5 58 	fpcmpueq8  %f22, %f24, %g4
 120:	81 b0 6a 23 	flcmps  %fcc0, %f1, %f3
 124:	83 b0 ea 25 	flcmps  %fcc1, %f3, %f5
 128:	85 b1 6a 27 	flcmps  %fcc2, %f5, %f7
 12c:	87 b1 ea 29 	flcmps  %fcc3, %f7, %f9
 130:	81 b3 2a 4e 	flcmpd  %fcc0, %f12, %f14
 134:	83 b3 aa 50 	flcmpd  %fcc1, %f14, %f16
 138:	85 b4 2a 52 	flcmpd  %fcc2, %f16, %f18
 13c:	87 b4 aa 54 	flcmpd  %fcc3, %f18, %f20
 140:	b5 b0 02 f9 	lzcnt  %i1, %i2
