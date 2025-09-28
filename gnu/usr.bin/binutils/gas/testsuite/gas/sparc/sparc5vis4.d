#as: -Av9m
#objdump: -dr
#name: sparc SPARC5 and VIS4.0

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	87 b0 48 22 	subxc  %g1, %g2, %g3
   4:	87 b0 48 62 	subxccc  %g1, %g2, %g3
   8:	91 b0 a4 84 	fpadd8  %f2, %f4, %f8
   c:	99 b2 24 ca 	fpadds8  %f8, %f10, %f12
  10:	a1 b3 24 ee 	fpaddus8  %f12, %f14, %f16
  14:	a9 b4 24 72 	fpaddus16  %f16, %f18, %f20
  18:	83 b0 86 84 	fpcmple8  %f2, %f4, %g1
  1c:	85 b1 07 86 	fpcmpgt8  %f4, %f6, %g2
  20:	87 b1 a5 c8 	fpcmpule16  %f6, %f8, %g3
  24:	89 b2 25 6a 	fpcmpugt16  %f8, %f10, %g4
  28:	8b b2 a5 ec 	fpcmpule32  %f10, %f12, %g5
  2c:	8d b3 25 8e 	fpcmpugt32  %f12, %f14, %g6
  30:	a5 b3 a3 b0 	fpmax8  %f14, %f16, %f18
  34:	ad b4 a3 d4 	fpmax16  %f18, %f20, %f22
  38:	b5 b5 a3 f8 	fpmax32  %f22, %f24, %f26
  3c:	bd b6 ab bc 	fpmaxu8  %f26, %f28, %f30
  40:	87 b7 ab c1 	fpmaxu16  %f30, %f32, %f34
  44:	8f b0 eb e5 	fpmaxu32  %f34, %f36, %f38
  48:	97 b1 e3 49 	fpmin8  %f38, %f40, %f42
  4c:	9f b2 e3 6d 	fpmin16  %f42, %f44, %f46
  50:	a7 b3 e3 91 	fpmin32  %f46, %f48, %f50
  54:	af b4 eb 55 	fpminu8  %f50, %f52, %f54
  58:	b7 b5 eb 79 	fpminu16  %f54, %f56, %f58
  5c:	bf b6 eb 9d 	fpminu32  %f58, %f60, %f62
  60:	8d b0 aa 84 	fpsub8  %f2, %f4, %f6
  64:	95 b1 aa c8 	fpsubs8  %f6, %f8, %f10
  68:	9d b2 aa ec 	fpsubus8  %f10, %f12, %f14
  6c:	a5 b3 aa 70 	fpsubus16  %f14, %f16, %f18
  70:	bf b0 09 3f 	faligndata  %f0, %f62, %f4, %f62
