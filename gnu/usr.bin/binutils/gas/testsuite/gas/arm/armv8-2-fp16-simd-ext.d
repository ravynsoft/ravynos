#name: ARM v8.2 FP16 support on SIMD
#source: armv8-2-fp16-simd.s
#objdump: -d
#as: -march=armv8.2-a+fp16
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:

00000000 <func>:
   0:	f3342d0e 	vabd.f16	d2, d4, d14
   4:	f3384d6c 	vabd.f16	q2, q4, q14
   8:	f2142f0e 	vmax.f16	d2, d4, d14
   c:	f2184f6c 	vmax.f16	q2, q4, q14
  10:	f2342f0e 	vmin.f16	d2, d4, d14
  14:	f2384f6c 	vmin.f16	q2, q4, q14
  18:	f3300dec 	vabd.f16	q0, q8, q14
  1c:	f2100fec 	vmax.f16	q0, q8, q14
  20:	f2300fec 	vmin.f16	q0, q8, q14
  24:	f3331d0f 	vabd.f16	d1, d3, d15
  28:	f3310d08 	vabd.f16	d0, d1, d8
  2c:	f3b50708 	vabs.f16	d0, d8
  30:	f3b50760 	vabs.f16	q0, q8
  34:	f3b50788 	vneg.f16	d0, d8
  38:	f3b507e0 	vneg.f16	q0, q8
  3c:	f3b5474c 	vabs.f16	q2, q6
  40:	f3b547cc 	vneg.f16	q2, q6
  44:	f3b57703 	vabs.f16	d7, d3
  48:	f3b59781 	vneg.f16	d9, d1
  4c:	f3142e1e 	vacge.f16	d2, d4, d14
  50:	f3184e7c 	vacge.f16	q2, q4, q14
  54:	f3342e1e 	vacgt.f16	d2, d4, d14
  58:	f3384e7c 	vacgt.f16	q2, q4, q14
  5c:	f33e2e14 	vacgt.f16	d2, d14, d4
  60:	f33c4ed8 	vacgt.f16	q2, q14, q4
  64:	f31e2e14 	vacge.f16	d2, d14, d4
  68:	f31c4ed8 	vacge.f16	q2, q14, q4
  6c:	f2142e0e 	vceq.f16	d2, d4, d14
  70:	f2184e6c 	vceq.f16	q2, q4, q14
  74:	f3142e0e 	vcge.f16	d2, d4, d14
  78:	f3184e6c 	vcge.f16	q2, q4, q14
  7c:	f3342e0e 	vcgt.f16	d2, d4, d14
  80:	f3384e6c 	vcgt.f16	q2, q4, q14
  84:	f31e2e04 	vcge.f16	d2, d14, d4
  88:	f31c4ec8 	vcge.f16	q2, q14, q4
  8c:	f33e2e04 	vcgt.f16	d2, d14, d4
  90:	f33c4ec8 	vcgt.f16	q2, q14, q4
  94:	f3100efc 	vacge.f16	q0, q8, q14
  98:	f3300efc 	vacgt.f16	q0, q8, q14
  9c:	f33c0ef0 	vacgt.f16	q0, q14, q8
  a0:	f31c0ef0 	vacge.f16	q0, q14, q8
  a4:	f2100eec 	vceq.f16	q0, q8, q14
  a8:	f3100eec 	vcge.f16	q0, q8, q14
  ac:	f3300eec 	vcgt.f16	q0, q8, q14
  b0:	f31c0ee0 	vcge.f16	q0, q14, q8
  b4:	f33c0ee0 	vcgt.f16	q0, q14, q8
  b8:	f2142d0e 	vadd.f16	d2, d4, d14
  bc:	f2184d6c 	vadd.f16	q2, q4, q14
  c0:	f2342d0e 	vsub.f16	d2, d4, d14
  c4:	f2384d6c 	vsub.f16	q2, q4, q14
  c8:	f2100dec 	vadd.f16	q0, q8, q14
  cc:	f2300dec 	vsub.f16	q0, q8, q14
  d0:	f3142f1e 	vmaxnm.f16	d2, d4, d14
  d4:	f3184f7c 	vmaxnm.f16	q2, q4, q14
  d8:	f3342f1e 	vminnm.f16	d2, d4, d14
  dc:	f3384f7c 	vminnm.f16	q2, q4, q14
  e0:	f2142c1e 	vfma.f16	d2, d4, d14
  e4:	f2184c7c 	vfma.f16	q2, q4, q14
  e8:	f2342c1e 	vfms.f16	d2, d4, d14
  ec:	f2384c7c 	vfms.f16	q2, q4, q14
  f0:	f2142d1e 	vmla.f16	d2, d4, d14
  f4:	f2184d7c 	vmla.f16	q2, q4, q14
  f8:	f2342d1e 	vmls.f16	d2, d4, d14
  fc:	f2384d7c 	vmls.f16	q2, q4, q14
 100:	f3b6458e 	vrintz.f16	d4, d14
 104:	f3b685ec 	vrintz.f16	q4, q14
 108:	f3b6448e 	vrintx.f16	d4, d14
 10c:	f3b684ec 	vrintx.f16	q4, q14
 110:	f3b6450e 	vrinta.f16	d4, d14
 114:	f3b6856c 	vrinta.f16	q4, q14
 118:	f3b6440e 	vrintn.f16	d4, d14
 11c:	f3b6846c 	vrintn.f16	q4, q14
 120:	f3b6478e 	vrintp.f16	d4, d14
 124:	f3b687ec 	vrintp.f16	q4, q14
 128:	f3b6468e 	vrintm.f16	d4, d14
 12c:	f3b686ec 	vrintm.f16	q4, q14
 130:	f3184d0e 	vpadd.f16	d4, d8, d14
 134:	f3b74508 	vrecpe.f16	d4, d8
 138:	f3b78560 	vrecpe.f16	q4, q8
 13c:	f3b74588 	vrsqrte.f16	d4, d8
 140:	f3b785e0 	vrsqrte.f16	q4, q8
 144:	f3b70564 	vrecpe.f16	q0, q10
 148:	f3b705e4 	vrsqrte.f16	q0, q10
 14c:	f21a8f1c 	vrecps.f16	d8, d10, d12
 150:	f2540ff8 	vrecps.f16	q8, q10, q12
 154:	f23a8f1c 	vrsqrts.f16	d8, d10, d12
 158:	f2740ff8 	vrsqrts.f16	q8, q10, q12
 15c:	f2104f58 	vrecps.f16	q2, q0, q4
 160:	f2304f58 	vrsqrts.f16	q2, q0, q4
 164:	f3184f0e 	vpmax.f16	d4, d8, d14
 168:	f338af02 	vpmin.f16	d10, d8, d2
 16c:	f3184d1e 	vmul.f16	d4, d8, d14
 170:	f3107d11 	vmul.f16	d7, d0, d1
 174:	f3104dd0 	vmul.f16	q2, q8, q0
 178:	f3b7600c 	vcvta.s16.f16	d6, d12
 17c:	f3b7c068 	vcvta.s16.f16	q6, q12
 180:	f3b7630c 	vcvtm.s16.f16	d6, d12
 184:	f3b7c368 	vcvtm.s16.f16	q6, q12
 188:	f3b7610c 	vcvtn.s16.f16	d6, d12
 18c:	f3b7c168 	vcvtn.s16.f16	q6, q12
 190:	f3b7620c 	vcvtp.s16.f16	d6, d12
 194:	f3b7c268 	vcvtp.s16.f16	q6, q12
 198:	f3b7608c 	vcvta.u16.f16	d6, d12
 19c:	f3b7c0e8 	vcvta.u16.f16	q6, q12
 1a0:	f3b7638c 	vcvtm.u16.f16	d6, d12
 1a4:	f3b7c3e8 	vcvtm.u16.f16	q6, q12
 1a8:	f3b7618c 	vcvtn.u16.f16	d6, d12
 1ac:	f3b7c1e8 	vcvtn.u16.f16	q6, q12
 1b0:	f3b7628c 	vcvtp.u16.f16	d6, d12
 1b4:	f3b7c2e8 	vcvtp.u16.f16	q6, q12
 1b8:	f3b7e700 	vcvt.s16.f16	d14, d0
 1bc:	f3f7c740 	vcvt.s16.f16	q14, q0
 1c0:	f3b7e780 	vcvt.u16.f16	d14, d0
 1c4:	f3f7c7c0 	vcvt.u16.f16	q14, q0
 1c8:	f3b7e600 	vcvt.f16.s16	d14, d0
 1cc:	f3f7c640 	vcvt.f16.s16	q14, q0
 1d0:	f3b7e680 	vcvt.f16.u16	d14, d0
 1d4:	f3f7c6c0 	vcvt.f16.u16	q14, q0
 1d8:	f2bded10 	vcvt.s16.f16	d14, d0, #3
 1dc:	f2fdcd50 	vcvt.s16.f16	q14, q0, #3
 1e0:	f3bded10 	vcvt.u16.f16	d14, d0, #3
 1e4:	f3fdcd50 	vcvt.u16.f16	q14, q0, #3
 1e8:	f2bdec10 	vcvt.f16.s16	d14, d0, #3
 1ec:	f2fdcc50 	vcvt.f16.s16	q14, q0, #3
 1f0:	f3bdec10 	vcvt.f16.u16	d14, d0, #3
 1f4:	f3fdcc50 	vcvt.f16.u16	q14, q0, #3
 1f8:	f3b5e502 	vceq.f16	d14, d2, #0
 1fc:	f3f5c544 	vceq.f16	q14, q2, #0
 200:	f3b5e482 	vcge.f16	d14, d2, #0
 204:	f3f5c4c4 	vcge.f16	q14, q2, #0
 208:	f3b5e402 	vcgt.f16	d14, d2, #0
 20c:	f3f5c444 	vcgt.f16	q14, q2, #0
 210:	f3b5e582 	vcle.f16	d14, d2, #0
 214:	f3f5c5c4 	vcle.f16	q14, q2, #0
 218:	f3b5e602 	vclt.f16	d14, d2, #0
 21c:	f3f5c644 	vclt.f16	q14, q2, #0
 220:	f2907941 	vmul.f16	d7, d0, d1\[0\]
 224:	f2984966 	vmul.f16	d4, d8, d6\[2\]
 228:	f39049c8 	vmul.f16	q2, q8, d0\[1\]
 22c:	f39049ef 	vmul.f16	q2, q8, d7\[3\]
 230:	f2942141 	vmla.f16	d2, d4, d1\[0\]
 234:	f3984141 	vmla.f16	q2, q4, d1\[0\]
 238:	f2942541 	vmls.f16	d2, d4, d1\[0\]
 23c:	f3984541 	vmls.f16	q2, q4, d1\[0\]
 240:	f298116f 	vmla.f16	d1, d8, d7\[3\]
 244:	f39021ef 	vmla.f16	q1, q8, d7\[3\]
 248:	f298156f 	vmls.f16	d1, d8, d7\[3\]
 24c:	f39025ef 	vmls.f16	q1, q8, d7\[3\]
