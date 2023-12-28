#as: -Av9v
#objdump: -dr
#name: sparc CBCOND

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	12 c2 49 0a 	cwbe  %o1, %o2, 0x120
   4:	12 c2 68 e2 	cwbe  %o1, 2, 0x120
   8:	12 e2 88 cb 	cxbe  %o2, %o3, 0x120
   c:	12 e2 a8 a3 	cxbe  %o2, 3, 0x120
  10:	14 c2 c8 8c 	cwble  %o3, %o4, 0x120
  14:	14 c2 e8 64 	cwble  %o3, 4, 0x120
  18:	14 e3 08 4d 	cxble  %o4, %o5, 0x120
  1c:	14 e3 28 25 	cxble  %o4, 5, 0x120
  20:	16 c3 48 10 	cwbl  %o5, %l0, 0x120
  24:	16 c3 67 e6 	cwbl  %o5, 6, 0x120
  28:	16 e4 07 d1 	cxbl  %l0, %l1, 0x120
  2c:	16 e4 27 a7 	cxbl  %l0, 7, 0x120
  30:	18 c4 47 92 	cwbleu  %l1, %l2, 0x120
  34:	18 c4 67 68 	cwbleu  %l1, 8, 0x120
  38:	18 e4 87 53 	cxbleu  %l2, %l3, 0x120
  3c:	18 e4 a7 29 	cxbleu  %l2, 9, 0x120
  40:	1a c4 c7 14 	cwbcs  %l3, %l4, 0x120
  44:	1a c4 e6 ea 	cwbcs  %l3, 0xa, 0x120
  48:	1a e5 06 d5 	cxbcs  %l4, %l5, 0x120
  4c:	1a e5 26 ab 	cxbcs  %l4, 0xb, 0x120
  50:	1c c5 46 96 	cwbneg  %l5, %l6, 0x120
  54:	1c c5 66 6c 	cwbneg  %l5, 0xc, 0x120
  58:	1c e5 86 57 	cxbneg  %l6, %l7, 0x120
  5c:	1c e5 a6 2d 	cxbneg  %l6, 0xd, 0x120
  60:	1e c5 c6 18 	cwbvs  %l7, %i0, 0x120
  64:	1e c5 e5 ee 	cwbvs  %l7, 0xe, 0x120
  68:	1e e6 05 d9 	cxbvs  %i0, %i1, 0x120
  6c:	1e e6 25 af 	cxbvs  %i0, 0xf, 0x120
  70:	32 c6 45 9a 	cwbne  %i1, %i2, 0x120
  74:	32 c6 65 70 	cwbne  %i1, 0x10, 0x120
  78:	32 e6 85 5b 	cxbne  %i2, %i3, 0x120
  7c:	32 e6 a5 31 	cxbne  %i2, 0x11, 0x120
  80:	34 c6 c5 1c 	cwbg  %i3, %i4, 0x120
  84:	34 c6 e4 f2 	cwbg  %i3, 0x12, 0x120
  88:	34 e7 04 dd 	cxbg  %i4, %i5, 0x120
  8c:	34 e7 24 b3 	cxbg  %i4, 0x13, 0x120
  90:	36 c7 44 88 	cwbge  %i5, %o0, 0x120
  94:	36 c7 64 74 	cwbge  %i5, 0x14, 0x120
  98:	36 e2 04 49 	cxbge  %o0, %o1, 0x120
  9c:	36 e2 24 35 	cxbge  %o0, 0x15, 0x120
  a0:	38 c2 44 0a 	cwbgu  %o1, %o2, 0x120
  a4:	38 c2 63 f6 	cwbgu  %o1, 0x16, 0x120
  a8:	38 e2 83 cb 	cxbgu  %o2, %o3, 0x120
  ac:	38 e2 a3 b6 	cxbgu  %o2, 0x16, 0x120
  b0:	3a c2 c3 8c 	cwbcc  %o3, %o4, 0x120
  b4:	3a c2 e3 77 	cwbcc  %o3, 0x17, 0x120
  b8:	3a e3 03 4d 	cxbcc  %o4, %o5, 0x120
  bc:	3a e3 23 38 	cxbcc  %o4, 0x18, 0x120
  c0:	3c c3 43 10 	cwbpos  %o5, %l0, 0x120
  c4:	3c c3 62 f9 	cwbpos  %o5, 0x19, 0x120
  c8:	3c e4 02 d1 	cxbpos  %l0, %l1, 0x120
  cc:	3c e4 22 b9 	cxbpos  %l0, 0x19, 0x120
  d0:	3e c4 42 92 	cwbvc  %l1, %l2, 0x120
  d4:	3e c4 62 7a 	cwbvc  %l1, 0x1a, 0x120
  d8:	3e e4 82 53 	cxbvc  %l2, %l3, 0x120
  dc:	3e e4 a2 3b 	cxbvc  %l2, 0x1b, 0x120
  e0:	12 c4 c2 14 	cwbe  %l3, %l4, 0x120
  e4:	12 c4 e1 fc 	cwbe  %l3, 0x1c, 0x120
  e8:	12 e5 01 d5 	cxbe  %l4, %l5, 0x120
  ec:	12 e5 21 bd 	cxbe  %l4, 0x1d, 0x120
  f0:	1a c5 41 96 	cwbcs  %l5, %l6, 0x120
  f4:	1a c5 61 7c 	cwbcs  %l5, 0x1c, 0x120
  f8:	1a e5 81 57 	cxbcs  %l6, %l7, 0x120
  fc:	1a e5 a1 3d 	cxbcs  %l6, 0x1d, 0x120
 100:	32 c5 c1 08 	cwbne  %l7, %o0, 0x120
 104:	32 c5 e0 fe 	cwbne  %l7, 0x1e, 0x120
 108:	32 e2 00 c9 	cxbne  %o0, %o1, 0x120
 10c:	32 e2 20 bf 	cxbne  %o0, 0x1f, 0x120
 110:	3a c2 40 8a 	cwbcc  %o1, %o2, 0x120
 114:	3a c2 60 61 	cwbcc  %o1, 1, 0x120
 118:	3a e2 80 4b 	cxbcc  %o2, %o3, 0x120
 11c:	3a e2 a0 22 	cxbcc  %o2, 2, 0x120
 120:	01 00 00 00 	nop 
