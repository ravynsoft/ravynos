#objdump: -d -Mpower4
#as: -a32 -mpower4
#name: Power4 instructions

.*

Disassembly of section \.text:

0+00 <start>:
   0:	(80 c7 00 00|00 00 c7 80) 	lwz     r6,0\(r7\)
   4:	(80 c7 00 10|10 00 c7 80) 	lwz     r6,16\(r7\)
   8:	(80 c7 ff f0|f0 ff c7 80) 	lwz     r6,-16\(r7\)
   c:	(80 c7 80 00|00 80 c7 80) 	lwz     r6,-32768\(r7\)
  10:	(80 c7 7f f0|f0 7f c7 80) 	lwz     r6,32752\(r7\)
  14:	(90 c7 00 00|00 00 c7 90) 	stw     r6,0\(r7\)
  18:	(90 c7 00 10|10 00 c7 90) 	stw     r6,16\(r7\)
  1c:	(90 c7 ff f0|f0 ff c7 90) 	stw     r6,-16\(r7\)
  20:	(90 c7 80 00|00 80 c7 90) 	stw     r6,-32768\(r7\)
  24:	(90 c7 7f f0|f0 7f c7 90) 	stw     r6,32752\(r7\)
  28:	(00 00 02 00|00 02 00 00) 	attn
  2c:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
  30:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
  34:	(7c 68 11 20|20 11 68 7c) 	mtcrf   129,r3
  38:	(7c 70 11 20|20 11 70 7c) 	mtocrf  1,r3
  3c:	(7c 70 21 20|20 21 70 7c) 	mtocrf  2,r3
  40:	(7c 70 41 20|20 41 70 7c) 	mtocrf  4,r3
  44:	(7c 70 81 20|20 81 70 7c) 	mtocrf  8,r3
  48:	(7c 71 01 20|20 01 71 7c) 	mtocrf  16,r3
  4c:	(7c 72 01 20|20 01 72 7c) 	mtocrf  32,r3
  50:	(7c 74 01 20|20 01 74 7c) 	mtocrf  64,r3
  54:	(7c 78 01 20|20 01 78 7c) 	mtocrf  128,r3
  58:	(7c 60 00 26|26 00 60 7c) 	mfcr    r3
  5c:	(7c 70 10 26|26 10 70 7c) 	mfocrf  r3,1
  60:	(7c 70 20 26|26 20 70 7c) 	mfocrf  r3,2
  64:	(7c 70 40 26|26 40 70 7c) 	mfocrf  r3,4
  68:	(7c 70 80 26|26 80 70 7c) 	mfocrf  r3,8
  6c:	(7c 71 00 26|26 00 71 7c) 	mfocrf  r3,16
  70:	(7c 72 00 26|26 00 72 7c) 	mfocrf  r3,32
  74:	(7c 74 00 26|26 00 74 7c) 	mfocrf  r3,64
  78:	(7c 78 00 26|26 00 78 7c) 	mfocrf  r3,128
  7c:	(7c 01 17 ec|ec 17 01 7c) 	dcbz    r1,r2
  80:	(7c 23 27 ec|ec 27 23 7c) 	dcbzl   r3,r4
  84:	(7c 05 37 ec|ec 37 05 7c) 	dcbz    r5,r6
  88:	(7c 05 32 2c|2c 32 05 7c) 	dcbtct  r5,r6
  8c:	(7c 05 32 2c|2c 32 05 7c) 	dcbtct  r5,r6
  90:	(7d 05 32 2c|2c 32 05 7d) 	dcbtds  r5,r6
#pass
