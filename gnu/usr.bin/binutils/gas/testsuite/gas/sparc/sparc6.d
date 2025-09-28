#as: -Av9m8
#objdump: -dr
#name: SPARC6

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	87 b0 43 8a 	dictunpack  %f32, 0xa, %f34
   4:	8b b0 60 c3 	fpsll64x  %f32, %f34, %f36
   8:	97 b1 e1 e9 	fpsra64x  %f38, %f40, %f42
   c:	a7 b3 60 f1 	fpsrl64x  %f44, %f48, %f50
  10:	85 b0 43 c0 	revbitsb  %g1, %g2
  14:	89 b0 c3 c1 	revbytesh  %g3, %g4
  18:	8d b1 43 c2 	revbytesw  %g5, %g6
  1c:	89 b0 83 c3 	revbytesx  %g2, %g4
  20:	81 b0 28 80 	sha3 
