#name: ARM v8.2 FP16 support on SIMD (Thumb)
#source: armv8-2-fp16-simd.s
#objdump: -d
#as: -march=armv8.2-a+fp16 -mfpu=neon-fp-armv8 -mthumb
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:

00000000 <func>:
   0:	ff34 2d0e 	vabd.f16	d2, d4, d14
   4:	ff38 4d6c 	vabd.f16	q2, q4, q14
   8:	ef14 2f0e 	vmax.f16	d2, d4, d14
   c:	ef18 4f6c 	vmax.f16	q2, q4, q14
  10:	ef34 2f0e 	vmin.f16	d2, d4, d14
  14:	ef38 4f6c 	vmin.f16	q2, q4, q14
  18:	ff30 0dec 	vabd.f16	q0, q8, q14
  1c:	ef10 0fec 	vmax.f16	q0, q8, q14
  20:	ef30 0fec 	vmin.f16	q0, q8, q14
  24:	ff33 1d0f 	vabd.f16	d1, d3, d15
  28:	ff31 0d08 	vabd.f16	d0, d1, d8
  2c:	ffb5 0708 	vabs.f16	d0, d8
  30:	ffb5 0760 	vabs.f16	q0, q8
  34:	ffb5 0788 	vneg.f16	d0, d8
  38:	ffb5 07e0 	vneg.f16	q0, q8
  3c:	ffb5 474c 	vabs.f16	q2, q6
  40:	ffb5 47cc 	vneg.f16	q2, q6
  44:	ffb5 7703 	vabs.f16	d7, d3
  48:	ffb5 9781 	vneg.f16	d9, d1
  4c:	ff14 2e1e 	vacge.f16	d2, d4, d14
  50:	ff18 4e7c 	vacge.f16	q2, q4, q14
  54:	ff34 2e1e 	vacgt.f16	d2, d4, d14
  58:	ff38 4e7c 	vacgt.f16	q2, q4, q14
  5c:	ff3e 2e14 	vacgt.f16	d2, d14, d4
  60:	ff3c 4ed8 	vacgt.f16	q2, q14, q4
  64:	ff1e 2e14 	vacge.f16	d2, d14, d4
  68:	ff1c 4ed8 	vacge.f16	q2, q14, q4
  6c:	ef14 2e0e 	vceq.f16	d2, d4, d14
  70:	ef18 4e6c 	vceq.f16	q2, q4, q14
  74:	ff14 2e0e 	vcge.f16	d2, d4, d14
  78:	ff18 4e6c 	vcge.f16	q2, q4, q14
  7c:	ff34 2e0e 	vcgt.f16	d2, d4, d14
  80:	ff38 4e6c 	vcgt.f16	q2, q4, q14
  84:	ff1e 2e04 	vcge.f16	d2, d14, d4
  88:	ff1c 4ec8 	vcge.f16	q2, q14, q4
  8c:	ff3e 2e04 	vcgt.f16	d2, d14, d4
  90:	ff3c 4ec8 	vcgt.f16	q2, q14, q4
  94:	ff10 0efc 	vacge.f16	q0, q8, q14
  98:	ff30 0efc 	vacgt.f16	q0, q8, q14
  9c:	ff3c 0ef0 	vacgt.f16	q0, q14, q8
  a0:	ff1c 0ef0 	vacge.f16	q0, q14, q8
  a4:	ef10 0eec 	vceq.f16	q0, q8, q14
  a8:	ff10 0eec 	vcge.f16	q0, q8, q14
  ac:	ff30 0eec 	vcgt.f16	q0, q8, q14
  b0:	ff1c 0ee0 	vcge.f16	q0, q14, q8
  b4:	ff3c 0ee0 	vcgt.f16	q0, q14, q8
  b8:	ef14 2d0e 	vadd.f16	d2, d4, d14
  bc:	ef18 4d6c 	vadd.f16	q2, q4, q14
  c0:	ef34 2d0e 	vsub.f16	d2, d4, d14
  c4:	ef38 4d6c 	vsub.f16	q2, q4, q14
  c8:	ef10 0dec 	vadd.f16	q0, q8, q14
  cc:	ef30 0dec 	vsub.f16	q0, q8, q14
  d0:	ff14 2f1e 	vmaxnm.f16	d2, d4, d14
  d4:	ff18 4f7c 	vmaxnm.f16	q2, q4, q14
  d8:	ff34 2f1e 	vminnm.f16	d2, d4, d14
  dc:	ff38 4f7c 	vminnm.f16	q2, q4, q14
  e0:	ef14 2c1e 	vfma.f16	d2, d4, d14
  e4:	ef18 4c7c 	vfma.f16	q2, q4, q14
  e8:	ef34 2c1e 	vfms.f16	d2, d4, d14
  ec:	ef38 4c7c 	vfms.f16	q2, q4, q14
  f0:	ef14 2d1e 	vmla.f16	d2, d4, d14
  f4:	ef18 4d7c 	vmla.f16	q2, q4, q14
  f8:	ef34 2d1e 	vmls.f16	d2, d4, d14
  fc:	ef38 4d7c 	vmls.f16	q2, q4, q14
 100:	ffb6 458e 	vrintz.f16	d4, d14
 104:	ffb6 85ec 	vrintz.f16	q4, q14
 108:	ffb6 448e 	vrintx.f16	d4, d14
 10c:	ffb6 84ec 	vrintx.f16	q4, q14
 110:	ffb6 450e 	vrinta.f16	d4, d14
 114:	ffb6 856c 	vrinta.f16	q4, q14
 118:	ffb6 440e 	vrintn.f16	d4, d14
 11c:	ffb6 846c 	vrintn.f16	q4, q14
 120:	ffb6 478e 	vrintp.f16	d4, d14
 124:	ffb6 87ec 	vrintp.f16	q4, q14
 128:	ffb6 468e 	vrintm.f16	d4, d14
 12c:	ffb6 86ec 	vrintm.f16	q4, q14
 130:	ff18 4d0e 	vpadd.f16	d4, d8, d14
 134:	ffb7 4508 	vrecpe.f16	d4, d8
 138:	ffb7 8560 	vrecpe.f16	q4, q8
 13c:	ffb7 4588 	vrsqrte.f16	d4, d8
 140:	ffb7 85e0 	vrsqrte.f16	q4, q8
 144:	ffb7 0564 	vrecpe.f16	q0, q10
 148:	ffb7 05e4 	vrsqrte.f16	q0, q10
 14c:	ef1a 8f1c 	vrecps.f16	d8, d10, d12
 150:	ef54 0ff8 	vrecps.f16	q8, q10, q12
 154:	ef3a 8f1c 	vrsqrts.f16	d8, d10, d12
 158:	ef74 0ff8 	vrsqrts.f16	q8, q10, q12
 15c:	ef10 4f58 	vrecps.f16	q2, q0, q4
 160:	ef30 4f58 	vrsqrts.f16	q2, q0, q4
 164:	ff18 4f0e 	vpmax.f16	d4, d8, d14
 168:	ff38 af02 	vpmin.f16	d10, d8, d2
 16c:	ff18 4d1e 	vmul.f16	d4, d8, d14
 170:	ff10 7d11 	vmul.f16	d7, d0, d1
 174:	ff10 4dd0 	vmul.f16	q2, q8, q0
 178:	ffb7 600c 	vcvta.s16.f16	d6, d12
 17c:	ffb7 c068 	vcvta.s16.f16	q6, q12
 180:	ffb7 630c 	vcvtm.s16.f16	d6, d12
 184:	ffb7 c368 	vcvtm.s16.f16	q6, q12
 188:	ffb7 610c 	vcvtn.s16.f16	d6, d12
 18c:	ffb7 c168 	vcvtn.s16.f16	q6, q12
 190:	ffb7 620c 	vcvtp.s16.f16	d6, d12
 194:	ffb7 c268 	vcvtp.s16.f16	q6, q12
 198:	ffb7 608c 	vcvta.u16.f16	d6, d12
 19c:	ffb7 c0e8 	vcvta.u16.f16	q6, q12
 1a0:	ffb7 638c 	vcvtm.u16.f16	d6, d12
 1a4:	ffb7 c3e8 	vcvtm.u16.f16	q6, q12
 1a8:	ffb7 618c 	vcvtn.u16.f16	d6, d12
 1ac:	ffb7 c1e8 	vcvtn.u16.f16	q6, q12
 1b0:	ffb7 628c 	vcvtp.u16.f16	d6, d12
 1b4:	ffb7 c2e8 	vcvtp.u16.f16	q6, q12
 1b8:	ffb7 e700 	vcvt.s16.f16	d14, d0
 1bc:	fff7 c740 	vcvt.s16.f16	q14, q0
 1c0:	ffb7 e780 	vcvt.u16.f16	d14, d0
 1c4:	fff7 c7c0 	vcvt.u16.f16	q14, q0
 1c8:	ffb7 e600 	vcvt.f16.s16	d14, d0
 1cc:	fff7 c640 	vcvt.f16.s16	q14, q0
 1d0:	ffb7 e680 	vcvt.f16.u16	d14, d0
 1d4:	fff7 c6c0 	vcvt.f16.u16	q14, q0
 1d8:	efbd ed10 	vcvt.s16.f16	d14, d0, #3
 1dc:	effd cd50 	vcvt.s16.f16	q14, q0, #3
 1e0:	ffbd ed10 	vcvt.u16.f16	d14, d0, #3
 1e4:	fffd cd50 	vcvt.u16.f16	q14, q0, #3
 1e8:	efbd ec10 	vcvt.f16.s16	d14, d0, #3
 1ec:	effd cc50 	vcvt.f16.s16	q14, q0, #3
 1f0:	ffbd ec10 	vcvt.f16.u16	d14, d0, #3
 1f4:	fffd cc50 	vcvt.f16.u16	q14, q0, #3
 1f8:	ffb5 e502 	vceq.f16	d14, d2, #0
 1fc:	fff5 c544 	vceq.f16	q14, q2, #0
 200:	ffb5 e482 	vcge.f16	d14, d2, #0
 204:	fff5 c4c4 	vcge.f16	q14, q2, #0
 208:	ffb5 e402 	vcgt.f16	d14, d2, #0
 20c:	fff5 c444 	vcgt.f16	q14, q2, #0
 210:	ffb5 e582 	vcle.f16	d14, d2, #0
 214:	fff5 c5c4 	vcle.f16	q14, q2, #0
 218:	ffb5 e602 	vclt.f16	d14, d2, #0
 21c:	fff5 c644 	vclt.f16	q14, q2, #0
 220:	ef90 7941 	vmul.f16	d7, d0, d1\[0\]
 224:	ef98 4966 	vmul.f16	d4, d8, d6\[2\]
 228:	ff90 49c8 	vmul.f16	q2, q8, d0\[1\]
 22c:	ff90 49ef 	vmul.f16	q2, q8, d7\[3\]
 230:	ef94 2141 	vmla.f16	d2, d4, d1\[0\]
 234:	ff98 4141 	vmla.f16	q2, q4, d1\[0\]
 238:	ef94 2541 	vmls.f16	d2, d4, d1\[0\]
 23c:	ff98 4541 	vmls.f16	q2, q4, d1\[0\]
 240:	ef98 116f 	vmla.f16	d1, d8, d7\[3\]
 244:	ff90 21ef 	vmla.f16	q1, q8, d7\[3\]
 248:	ef98 156f 	vmls.f16	d1, d8, d7\[3\]
 24c:	ff90 25ef 	vmls.f16	q1, q8, d7\[3\]
