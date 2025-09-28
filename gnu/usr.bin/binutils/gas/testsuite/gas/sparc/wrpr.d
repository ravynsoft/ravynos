#as: -64 -Av9m
#objdump: -dr
#name: sparc64 wrpr

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	81 90 40 02 	wrpr  %g1, %g2, %tpc
   4:	81 90 40 00 	wrpr  %g1, %tpc
   8:	81 90 62 9a 	wrpr  %g1, 0x29a, %tpc
   c:	81 90 62 9a 	wrpr  %g1, 0x29a, %tpc
  10:	81 90 22 9a 	wrpr  0x29a, %tpc
  14:	83 90 40 02 	wrpr  %g1, %g2, %tnpc
  18:	83 90 40 00 	wrpr  %g1, %tnpc
  1c:	83 90 62 9a 	wrpr  %g1, 0x29a, %tnpc
  20:	83 90 62 9a 	wrpr  %g1, 0x29a, %tnpc
  24:	83 90 22 9a 	wrpr  0x29a, %tnpc
  28:	85 90 40 02 	wrpr  %g1, %g2, %tstate
  2c:	85 90 40 00 	wrpr  %g1, %tstate
  30:	85 90 62 9a 	wrpr  %g1, 0x29a, %tstate
  34:	85 90 62 9a 	wrpr  %g1, 0x29a, %tstate
  38:	85 90 22 9a 	wrpr  0x29a, %tstate
  3c:	87 90 40 02 	wrpr  %g1, %g2, %tt
  40:	87 90 40 00 	wrpr  %g1, %tt
  44:	87 90 62 9a 	wrpr  %g1, 0x29a, %tt
  48:	87 90 62 9a 	wrpr  %g1, 0x29a, %tt
  4c:	87 90 22 9a 	wrpr  0x29a, %tt
  50:	89 90 40 02 	wrpr  %g1, %g2, %tick
  54:	89 90 40 00 	wrpr  %g1, %tick
  58:	89 90 62 9a 	wrpr  %g1, 0x29a, %tick
  5c:	89 90 62 9a 	wrpr  %g1, 0x29a, %tick
  60:	89 90 22 9a 	wrpr  0x29a, %tick
  64:	8b 90 40 02 	wrpr  %g1, %g2, %tba
  68:	8b 90 40 00 	wrpr  %g1, %tba
  6c:	8b 90 62 9a 	wrpr  %g1, 0x29a, %tba
  70:	8b 90 62 9a 	wrpr  %g1, 0x29a, %tba
  74:	8b 90 22 9a 	wrpr  0x29a, %tba
  78:	8d 90 40 02 	wrpr  %g1, %g2, %pstate
  7c:	8d 90 40 00 	wrpr  %g1, %pstate
  80:	8d 90 62 9a 	wrpr  %g1, 0x29a, %pstate
  84:	8d 90 62 9a 	wrpr  %g1, 0x29a, %pstate
  88:	8d 90 22 9a 	wrpr  0x29a, %pstate
  8c:	8f 90 40 02 	wrpr  %g1, %g2, %tl
  90:	8f 90 40 00 	wrpr  %g1, %tl
  94:	8f 90 62 9a 	wrpr  %g1, 0x29a, %tl
  98:	8f 90 62 9a 	wrpr  %g1, 0x29a, %tl
  9c:	8f 90 22 9a 	wrpr  0x29a, %tl
  a0:	91 90 40 02 	wrpr  %g1, %g2, %pil
  a4:	91 90 40 00 	wrpr  %g1, %pil
  a8:	91 90 62 9a 	wrpr  %g1, 0x29a, %pil
  ac:	91 90 62 9a 	wrpr  %g1, 0x29a, %pil
  b0:	91 90 22 9a 	wrpr  0x29a, %pil
  b4:	93 90 40 02 	wrpr  %g1, %g2, %cwp
  b8:	93 90 40 00 	wrpr  %g1, %cwp
  bc:	93 90 62 9a 	wrpr  %g1, 0x29a, %cwp
  c0:	93 90 62 9a 	wrpr  %g1, 0x29a, %cwp
  c4:	93 90 22 9a 	wrpr  0x29a, %cwp
  c8:	95 90 40 02 	wrpr  %g1, %g2, %cansave
  cc:	95 90 40 00 	wrpr  %g1, %cansave
  d0:	95 90 62 9a 	wrpr  %g1, 0x29a, %cansave
  d4:	95 90 62 9a 	wrpr  %g1, 0x29a, %cansave
  d8:	95 90 22 9a 	wrpr  0x29a, %cansave
  dc:	97 90 40 02 	wrpr  %g1, %g2, %canrestore
  e0:	97 90 40 00 	wrpr  %g1, %canrestore
  e4:	97 90 62 9a 	wrpr  %g1, 0x29a, %canrestore
  e8:	97 90 62 9a 	wrpr  %g1, 0x29a, %canrestore
  ec:	97 90 22 9a 	wrpr  0x29a, %canrestore
  f0:	99 90 40 02 	wrpr  %g1, %g2, %cleanwin
  f4:	99 90 40 00 	wrpr  %g1, %cleanwin
  f8:	99 90 62 9a 	wrpr  %g1, 0x29a, %cleanwin
  fc:	99 90 62 9a 	wrpr  %g1, 0x29a, %cleanwin
 100:	99 90 22 9a 	wrpr  0x29a, %cleanwin
 104:	9b 90 40 02 	wrpr  %g1, %g2, %otherwin
 108:	9b 90 40 00 	wrpr  %g1, %otherwin
 10c:	9b 90 62 9a 	wrpr  %g1, 0x29a, %otherwin
 110:	9b 90 62 9a 	wrpr  %g1, 0x29a, %otherwin
 114:	9b 90 22 9a 	wrpr  0x29a, %otherwin
 118:	9d 90 40 02 	wrpr  %g1, %g2, %wstate
 11c:	9d 90 40 00 	wrpr  %g1, %wstate
 120:	9d 90 62 9a 	wrpr  %g1, 0x29a, %wstate
 124:	9d 90 62 9a 	wrpr  %g1, 0x29a, %wstate
 128:	9d 90 22 9a 	wrpr  0x29a, %wstate
 12c:	9f 90 40 02 	wrpr  %g1, %g2, %fq
 130:	9f 90 40 00 	wrpr  %g1, %fq
 134:	9f 90 62 9a 	wrpr  %g1, 0x29a, %fq
 138:	9f 90 62 9a 	wrpr  %g1, 0x29a, %fq
 13c:	9f 90 22 9a 	wrpr  0x29a, %fq
 140:	a1 90 40 02 	wrpr  %g1, %g2, %gl
 144:	a1 90 40 00 	wrpr  %g1, %gl
 148:	a1 90 62 9a 	wrpr  %g1, 0x29a, %gl
 14c:	a1 90 62 9a 	wrpr  %g1, 0x29a, %gl
 150:	a1 90 22 9a 	wrpr  0x29a, %gl
 154:	af 90 40 02 	wrpr  %g1, %g2, %pmcdper
 158:	af 90 40 00 	wrpr  %g1, %pmcdper
 15c:	af 90 62 9a 	wrpr  %g1, 0x29a, %pmcdper
 160:	af 90 62 9a 	wrpr  %g1, 0x29a, %pmcdper
 164:	af 90 22 9a 	wrpr  0x29a, %pmcdper
 168:	bf 90 40 02 	wrpr  %g1, %g2, %ver
 16c:	bf 90 40 00 	wrpr  %g1, %ver
 170:	bf 90 62 9a 	wrpr  %g1, 0x29a, %ver
 174:	bf 90 62 9a 	wrpr  %g1, 0x29a, %ver
 178:	bf 90 22 9a 	wrpr  0x29a, %ver
