#name: ARM v8.2 FP16 support on scalar
#source: armv8-2-fp16-scalar.s
#objdump: -d
#as: -march=armv8.2-a+fp16 -mfpu=fp-armv8
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*
Disassembly of section .text:

00000000 <label-0xc>:
   0:	ee001910 	vmov.f16	s0, r1
   4:	ee100990 	vmov.f16	r0, s1
   8:	eeb00900 	vmov.f16	s0, #0	@ 0x40000000  2.0

0000000c <label>:
   c:	00000ffe 	.word	0x00000ffe
  10:	ed5f1906 	vldr.16	s3, \[pc, #-12\]	@ c <label>
  14:	ed1f3902 	vldr.16	s6, \[pc, #-4\]	@ 18 <label\+0xc>
  18:	eddf1902 	vldr.16	s3, \[pc, #4\]	@ 24 <label\+0x18>
  1c:	edd00902 	vldr.16	s1, \[r0, #4\]
  20:	ed101902 	vldr.16	s2, \[r0, #-4\]
  24:	ed803902 	vstr.16	s6, \[r0, #4\]
  28:	ed405902 	vstr.16	s11, \[r0, #-4\]
  2c:	eec6298c 	vdiv.f16	s5, s13, s24
  30:	eee6298c 	vfma.f16	s5, s13, s24
  34:	eee629cc 	vfms.f16	s5, s13, s24
  38:	eed629cc 	vfnma.f16	s5, s13, s24
  3c:	eed6298c 	vfnms.f16	s5, s13, s24
  40:	fec6298c 	vmaxnm.f16	s5, s13, s24
  44:	fec629cc 	vminnm.f16	s5, s13, s24
  48:	ee46298c 	vmla.f16	s5, s13, s24
  4c:	ee4629cc 	vmls.f16	s5, s13, s24
  50:	ee66298c 	vmul.f16	s5, s13, s24
  54:	ee5629cc 	vnmla.f16	s5, s13, s24
  58:	ee56298c 	vnmls.f16	s5, s13, s24
  5c:	ee6629cc 	vnmul.f16	s5, s13, s24
  60:	ee7629cc 	vsub.f16	s5, s13, s24
  64:	eef029c6 	vabs.f16	s5, s12
  68:	ee722986 	vadd.f16	s5, s5, s12
  6c:	eef129c6 	vsqrt.f16	s5, s12
  70:	eef12946 	vneg.f16	s5, s12
  74:	eeb51940 	vcmp.f16	s2, #0.0
  78:	eeb519c0 	vcmpe.f16	s2, #0.0
  7c:	eef42966 	vcmp.f16	s5, s13
  80:	eef429e6 	vcmpe.f16	s5, s13
  84:	fe4629ab 	vseleq.f16	s5, s13, s23
  88:	fe6629ab 	vselge.f16	s5, s13, s23
  8c:	fe5629ab 	vselvs.f16	s5, s13, s23
  90:	eefd19c4 	vcvt.s32.f16	s3, s8
  94:	eefc19c4 	vcvt.u32.f16	s3, s8
  98:	eef819c4 	vcvt.f16.s32	s3, s8
  9c:	eef81944 	vcvt.f16.u32	s3, s8
  a0:	eefa39e1 	vcvt.f16.s32	s7, s7, #29
  a4:	eefb39e1 	vcvt.f16.u32	s7, s7, #29
  a8:	eefe39e1 	vcvt.s32.f16	s7, s7, #29
  ac:	eeff39e1 	vcvt.u32.f16	s7, s7, #29
  b0:	fefc29c5 	vcvta.s32.f16	s5, s10
  b4:	fefc2945 	vcvta.u32.f16	s5, s10
  b8:	feff29c5 	vcvtm.s32.f16	s5, s10
  bc:	feff2945 	vcvtm.u32.f16	s5, s10
  c0:	fefd29c5 	vcvtn.s32.f16	s5, s10
  c4:	fefd2945 	vcvtn.u32.f16	s5, s10
  c8:	fefe29c5 	vcvtp.s32.f16	s5, s10
  cc:	fefe2945 	vcvtp.u32.f16	s5, s10
  d0:	eefc2945 	vcvtr.u32.f16	s5, s10
  d4:	eefd2945 	vcvtr.s32.f16	s5, s10
  d8:	fef81965 	vrinta.f16	s3, s11
  dc:	fefb1965 	vrintm.f16	s3, s11
  e0:	fef91965 	vrintn.f16	s3, s11
  e4:	fefa1965 	vrintp.f16	s3, s11
  e8:	eef61965 	vrintr.f16	s3, s11
  ec:	eef71965 	vrintx.f16	s3, s11
  f0:	eef619e5 	vrintz.f16	s3, s11
  f4:	fef02ae4 	vins.f16	s5, s9
  f8:	fef02a64 	vmovx.f16	s5, s9
