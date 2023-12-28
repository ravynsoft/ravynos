# name: Single precision instructions for 'armv8.1-m.main'
# source: vfp1xD_t2.s
# as: -march=armv8.1-m.main+fp
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> eef1 fa10 	(vmrs	APSR_nzcv, fpscr|fmstat)
0+004 <[^>]*> eeb4 0ac0 	(vcmpe\.f32|fcmpes)	s0, s0
0+008 <[^>]*> eeb5 0ac0 	(vcmpe\.f32	s0, #0.0|fcmpezs	s0)
0+00c <[^>]*> eeb4 0a40 	(vcmp\.f32|fcmps)	s0, s0
0+010 <[^>]*> eeb5 0a40 	(vcmp\.f32	s0, #0.0|fcmpzs	s0)
0+014 <[^>]*> eeb0 0ac0 	(vabs\.f32|fabss)	s0, s0
0+018 <[^>]*> eeb0 0a40 	(vmov\.f32|fcpys)	s0, s0
0+01c <[^>]*> eeb1 0a40 	(vneg\.f32|fnegs)	s0, s0
0+020 <[^>]*> eeb1 0ac0 	(vsqrt\.f32|fsqrts)	s0, s0
0+024 <[^>]*> ee30 0a00 	(vadd\.f32|fadds)	s0, s0, s0
0+028 <[^>]*> ee80 0a00 	(vdiv\.f32|fdivs)	s0, s0, s0
0+02c <[^>]*> ee00 0a00 	(vmla\.f32|fmacs)	s0, s0, s0
0+030 <[^>]*> ee10 0a00 	(vnmls\.f32|fmscs)	s0, s0, s0
0+034 <[^>]*> ee20 0a00 	(vmul\.f32|fmuls)	s0, s0, s0
0+038 <[^>]*> ee00 0a40 	(vmls\.f32|fnmacs)	s0, s0, s0
0+03c <[^>]*> ee10 0a40 	(vnmla\.f32|fnmscs)	s0, s0, s0
0+040 <[^>]*> ee20 0a40 	(vnmul\.f32|fnmuls)	s0, s0, s0
0+044 <[^>]*> ee30 0a40 	(vsub\.f32|fsubs)	s0, s0, s0
0+048 <[^>]*> ed90 0a00 	(vldr|flds)	s0, \[r0\]
0+04c <[^>]*> ed80 0a00 	(vstr|fsts)	s0, \[r0\]
0+050 <[^>]*> ec90 0a01 	(vldmia|fldmias)	r0, {s0}
0+054 <[^>]*> ec90 0a01 	(vldmia|fldmias)	r0, {s0}
0+058 <[^>]*> ecb0 0a01 	(vldmia|fldmias)	r0!, {s0}
0+05c <[^>]*> ecb0 0a01 	(vldmia|fldmias)	r0!, {s0}
0+060 <[^>]*> ed30 0a01 	(vldmdb|fldmdbs)	r0!, {s0}
0+064 <[^>]*> ed30 0a01 	(vldmdb|fldmdbs)	r0!, {s0}
0+068 <[^>]*> ec90 0b03 	fldmiax	r0, {d0}(	@ Deprecated|)
0+06c <[^>]*> ec90 0b03 	fldmiax	r0, {d0}(	@ Deprecated|)
0+070 <[^>]*> ecb0 0b03 	fldmiax	r0!, {d0}(	@ Deprecated|)
0+074 <[^>]*> ecb0 0b03 	fldmiax	r0!, {d0}(	@ Deprecated|)
0+078 <[^>]*> ed30 0b03 	fldmdbx	r0!, {d0}(	@ Deprecated|)
0+07c <[^>]*> ed30 0b03 	fldmdbx	r0!, {d0}(	@ Deprecated|)
0+080 <[^>]*> ec80 0a01 	(vstmia|fstmias)	r0, {s0}
0+084 <[^>]*> ec80 0a01 	(vstmia|fstmias)	r0, {s0}
0+088 <[^>]*> eca0 0a01 	(vstmia|fstmias)	r0!, {s0}
0+08c <[^>]*> eca0 0a01 	(vstmia|fstmias)	r0!, {s0}
0+090 <[^>]*> ed20 0a01 	(vstmdb|fstmdbs)	r0!, {s0}
0+094 <[^>]*> ed20 0a01 	(vstmdb|fstmdbs)	r0!, {s0}
0+098 <[^>]*> ec80 0b03 	fstmiax	r0, {d0}(	@ Deprecated|)
0+09c <[^>]*> ec80 0b03 	fstmiax	r0, {d0}(	@ Deprecated|)
0+0a0 <[^>]*> eca0 0b03 	fstmiax	r0!, {d0}(	@ Deprecated|)
0+0a4 <[^>]*> eca0 0b03 	fstmiax	r0!, {d0}(	@ Deprecated|)
0+0a8 <[^>]*> ed20 0b03 	fstmdbx	r0!, {d0}(	@ Deprecated|)
0+0ac <[^>]*> ed20 0b03 	fstmdbx	r0!, {d0}(	@ Deprecated|)
0+0b0 <[^>]*> eeb8 0ac0 	(vcvt\.f32\.s32|fsitos)	s0, s0
0+0b4 <[^>]*> eeb8 0a40 	(vcvt\.f32\.u32|fuitos)	s0, s0
0+0b8 <[^>]*> eebd 0a40 	(vcvtr\.s32\.f32|ftosis)	s0, s0
0+0bc <[^>]*> eebd 0ac0 	(vcvt\.s32\.f32|ftosizs)	s0, s0
0+0c0 <[^>]*> eebc 0a40 	(vcvtr\.u32\.f32|ftouis)	s0, s0
0+0c4 <[^>]*> eebc 0ac0 	(vcvt\.u32\.f32|ftouizs)	s0, s0
0+0c8 <[^>]*> ee10 0a10 	(vmov|fmrs)	r0, s0
0+0cc <[^>]*> eef0 0a10 	(vmrs|fmrx)	r0, fpsid
0+0d0 <[^>]*> eef1 0a10 	(vmrs|fmrx)	r0, fpscr
0+0d4 <[^>]*> eef8 0a10 	(vmrs|fmrx)	r0, fpexc
0+0d8 <[^>]*> ee00 0a10 	(vmov|fmsr)	s0, r0
0+0dc <[^>]*> eee0 0a10 	(vmsr|fmxr)	fpsid, r0
0+0e0 <[^>]*> eee1 0a10 	(vmsr|fmxr)	fpscr, r0
0+0e4 <[^>]*> eee8 0a10 	(vmsr|fmxr)	fpexc, r0
0+0e8 <[^>]*> eef5 0a40 	(vcmp\.f32	s1, #0.0|fcmpzs	s1)
0+0ec <[^>]*> eeb5 1a40 	(vcmp\.f32	s2, #0.0|fcmpzs	s2)
0+0f0 <[^>]*> eef5 fa40 	(vcmp\.f32	s31, #0.0|fcmpzs	s31)
0+0f4 <[^>]*> eeb4 0a60 	(vcmp\.f32|fcmps)	s0, s1
0+0f8 <[^>]*> eeb4 0a41 	(vcmp\.f32|fcmps)	s0, s2
0+0fc <[^>]*> eeb4 0a6f 	(vcmp\.f32|fcmps)	s0, s31
0+100 <[^>]*> eef4 0a40 	(vcmp\.f32|fcmps)	s1, s0
0+104 <[^>]*> eeb4 1a40 	(vcmp\.f32|fcmps)	s2, s0
0+108 <[^>]*> eef4 fa40 	(vcmp\.f32|fcmps)	s31, s0
0+10c <[^>]*> eef4 aa46 	(vcmp\.f32|fcmps)	s21, s12
0+110 <[^>]*> eeb1 0a60 	(vneg\.f32|fnegs)	s0, s1
0+114 <[^>]*> eeb1 0a41 	(vneg\.f32|fnegs)	s0, s2
0+118 <[^>]*> eeb1 0a6f 	(vneg\.f32|fnegs)	s0, s31
0+11c <[^>]*> eef1 0a40 	(vneg\.f32|fnegs)	s1, s0
0+120 <[^>]*> eeb1 1a40 	(vneg\.f32|fnegs)	s2, s0
0+124 <[^>]*> eef1 fa40 	(vneg\.f32|fnegs)	s31, s0
0+128 <[^>]*> eeb1 6a6a 	(vneg\.f32|fnegs)	s12, s21
0+12c <[^>]*> ee30 0a20 	(vadd\.f32|fadds)	s0, s0, s1
0+130 <[^>]*> ee30 0a01 	(vadd\.f32|fadds)	s0, s0, s2
0+134 <[^>]*> ee30 0a2f 	(vadd\.f32|fadds)	s0, s0, s31
0+138 <[^>]*> ee30 0a80 	(vadd\.f32|fadds)	s0, s1, s0
0+13c <[^>]*> ee31 0a00 	(vadd\.f32|fadds)	s0, s2, s0
0+140 <[^>]*> ee3f 0a80 	(vadd\.f32|fadds)	s0, s31, s0
0+144 <[^>]*> ee70 0a00 	(vadd\.f32|fadds)	s1, s0, s0
0+148 <[^>]*> ee30 1a00 	(vadd\.f32|fadds)	s2, s0, s0
0+14c <[^>]*> ee70 fa00 	(vadd\.f32|fadds)	s31, s0, s0
0+150 <[^>]*> ee3a 6aa2 	(vadd\.f32|fadds)	s12, s21, s5
0+154 <[^>]*> eeb8 0ae0 	(vcvt\.f32\.s32|fsitos)	s0, s1
0+158 <[^>]*> eeb8 0ac1 	(vcvt\.f32\.s32|fsitos)	s0, s2
0+15c <[^>]*> eeb8 0aef 	(vcvt\.f32\.s32|fsitos)	s0, s31
0+160 <[^>]*> eef8 0ac0 	(vcvt\.f32\.s32|fsitos)	s1, s0
0+164 <[^>]*> eeb8 1ac0 	(vcvt\.f32\.s32|fsitos)	s2, s0
0+168 <[^>]*> eef8 fac0 	(vcvt\.f32\.s32|fsitos)	s31, s0
0+16c <[^>]*> eebd 0a60 	(vcvtr\.s32\.f32|ftosis)	s0, s1
0+170 <[^>]*> eebd 0a41 	(vcvtr\.s32\.f32|ftosis)	s0, s2
0+174 <[^>]*> eebd 0a6f 	(vcvtr\.s32\.f32|ftosis)	s0, s31
0+178 <[^>]*> eefd 0a40 	(vcvtr\.s32\.f32|ftosis)	s1, s0
0+17c <[^>]*> eebd 1a40 	(vcvtr\.s32\.f32|ftosis)	s2, s0
0+180 <[^>]*> eefd fa40 	(vcvtr\.s32\.f32|ftosis)	s31, s0
0+184 <[^>]*> ee00 1a10 	(vmov|fmsr)	s0, r1
0+188 <[^>]*> ee00 7a10 	(vmov|fmsr)	s0, r7
0+18c <[^>]*> ee00 ea10 	(vmov|fmsr)	s0, lr
0+190 <[^>]*> ee00 0a90 	(vmov|fmsr)	s1, r0
0+194 <[^>]*> ee01 0a10 	(vmov|fmsr)	s2, r0
0+198 <[^>]*> ee0f 0a90 	(vmov|fmsr)	s31, r0
0+19c <[^>]*> ee0a 7a90 	(vmov|fmsr)	s21, r7
0+1a0 <[^>]*> eee0 1a10 	(vmsr|fmxr)	fpsid, r1
0+1a4 <[^>]*> eee0 ea10 	(vmsr|fmxr)	fpsid, lr
0+1a8 <[^>]*> ee10 0a90 	(vmov|fmrs)	r0, s1
0+1ac <[^>]*> ee11 0a10 	(vmov|fmrs)	r0, s2
0+1b0 <[^>]*> ee1f 0a90 	(vmov|fmrs)	r0, s31
0+1b4 <[^>]*> ee10 1a10 	(vmov|fmrs)	r1, s0
0+1b8 <[^>]*> ee10 7a10 	(vmov|fmrs)	r7, s0
0+1bc <[^>]*> ee10 ea10 	(vmov|fmrs)	lr, s0
0+1c0 <[^>]*> ee15 9a90 	(vmov|fmrs)	r9, s11
0+1c4 <[^>]*> eef0 1a10 	(vmrs|fmrx)	r1, fpsid
0+1c8 <[^>]*> eef0 ea10 	(vmrs|fmrx)	lr, fpsid
0+1cc <[^>]*> ed91 0a00 	(vldr|flds)	s0, \[r1\]
0+1d0 <[^>]*> ed9e 0a00 	(vldr|flds)	s0, \[lr\]
0+1d4 <[^>]*> ed90 0a00 	(vldr|flds)	s0, \[r0\]
0+1d8 <[^>]*> ed90 0aff 	(vldr|flds)	s0, \[r0, #1020\].*
0+1dc <[^>]*> ed10 0aff 	(vldr|flds)	s0, \[r0, #-1020\].*
0+1e0 <[^>]*> edd0 0a00 	(vldr|flds)	s1, \[r0\]
0+1e4 <[^>]*> ed90 1a00 	(vldr|flds)	s2, \[r0\]
0+1e8 <[^>]*> edd0 fa00 	(vldr|flds)	s31, \[r0\]
0+1ec <[^>]*> edcc aac9 	(vstr|fsts)	s21, \[ip, #804\].*
0+1f0 <[^>]*> ecd0 0a01 	(vldmia|fldmias)	r0, {s1}
0+1f4 <[^>]*> ec90 1a01 	(vldmia|fldmias)	r0, {s2}
0+1f8 <[^>]*> ecd0 fa01 	(vldmia|fldmias)	r0, {s31}
0+1fc <[^>]*> ec90 0a02 	(vldmia|fldmias)	r0, {s0-s1}
0+200 <[^>]*> ec90 0a03 	(vldmia|fldmias)	r0, {s0-s2}
0+204 <[^>]*> ec90 0a20 	(vldmia|fldmias)	r0, {s0-s31}
0+208 <[^>]*> ecd0 0a1f 	(vldmia|fldmias)	r0, {s1-s31}
0+20c <[^>]*> ec90 1a1e 	(vldmia|fldmias)	r0, {s2-s31}
0+210 <[^>]*> ec90 fa02 	(vldmia|fldmias)	r0, {s30-s31}
0+214 <[^>]*> ec91 0a01 	(vldmia|fldmias)	r1, {s0}
0+218 <[^>]*> ec9e 0a01 	(vldmia|fldmias)	lr, {s0}
0+21c <[^>]*> ec80 1b03 	fstmiax	r0, {d1}(	@ Deprecated|)
0+220 <[^>]*> ec80 2b03 	fstmiax	r0, {d2}(	@ Deprecated|)
0+224 <[^>]*> ec80 fb03 	fstmiax	r0, {d15}(	@ Deprecated|)
0+228 <[^>]*> ec80 0b05 	fstmiax	r0, {d0-d1}(	@ Deprecated|)
0+22c <[^>]*> ec80 0b07 	fstmiax	r0, {d0-d2}(	@ Deprecated|)
0+230 <[^>]*> ec80 0b21 	fstmiax	r0, {d0-d15}(	@ Deprecated|)
0+234 <[^>]*> ec80 1b1f 	fstmiax	r0, {d1-d15}(	@ Deprecated|)
0+238 <[^>]*> ec80 2b1d 	fstmiax	r0, {d2-d15}(	@ Deprecated|)
0+23c <[^>]*> ec80 eb05 	fstmiax	r0, {d14-d15}(	@ Deprecated|)
0+240 <[^>]*> ec81 0b03 	fstmiax	r1, {d0}(	@ Deprecated|)
0+244 <[^>]*> ec8e 0b03 	fstmiax	lr, {d0}(	@ Deprecated|)
0+248 <[^>]*> eeb5 0a40 	(vcmp\.f32	s0, #0.0|fcmpzs	s0)
0+24c <[^>]*> eef5 0a40 	(vcmp\.f32	s1, #0.0|fcmpzs	s1)
0+250 <[^>]*> eeb5 1a40 	(vcmp\.f32	s2, #0.0|fcmpzs	s2)
0+254 <[^>]*> eef5 1a40 	(vcmp\.f32	s3, #0.0|fcmpzs	s3)
0+258 <[^>]*> eeb5 2a40 	(vcmp\.f32	s4, #0.0|fcmpzs	s4)
0+25c <[^>]*> eef5 2a40 	(vcmp\.f32	s5, #0.0|fcmpzs	s5)
0+260 <[^>]*> eeb5 3a40 	(vcmp\.f32	s6, #0.0|fcmpzs	s6)
0+264 <[^>]*> eef5 3a40 	(vcmp\.f32	s7, #0.0|fcmpzs	s7)
0+268 <[^>]*> eeb5 4a40 	(vcmp\.f32	s8, #0.0|fcmpzs	s8)
0+26c <[^>]*> eef5 4a40 	(vcmp\.f32	s9, #0.0|fcmpzs	s9)
0+270 <[^>]*> eeb5 5a40 	(vcmp\.f32	s10, #0.0|fcmpzs	s10)
0+274 <[^>]*> eef5 5a40 	(vcmp\.f32	s11, #0.0|fcmpzs	s11)
0+278 <[^>]*> eeb5 6a40 	(vcmp\.f32	s12, #0.0|fcmpzs	s12)
0+27c <[^>]*> eef5 6a40 	(vcmp\.f32	s13, #0.0|fcmpzs	s13)
0+280 <[^>]*> eeb5 7a40 	(vcmp\.f32	s14, #0.0|fcmpzs	s14)
0+284 <[^>]*> eef5 7a40 	(vcmp\.f32	s15, #0.0|fcmpzs	s15)
0+288 <[^>]*> eeb5 8a40 	(vcmp\.f32	s16, #0.0|fcmpzs	s16)
0+28c <[^>]*> eef5 8a40 	(vcmp\.f32	s17, #0.0|fcmpzs	s17)
0+290 <[^>]*> eeb5 9a40 	(vcmp\.f32	s18, #0.0|fcmpzs	s18)
0+294 <[^>]*> eef5 9a40 	(vcmp\.f32	s19, #0.0|fcmpzs	s19)
0+298 <[^>]*> eeb5 aa40 	(vcmp\.f32	s20, #0.0|fcmpzs	s20)
0+29c <[^>]*> eef5 aa40 	(vcmp\.f32	s21, #0.0|fcmpzs	s21)
0+2a0 <[^>]*> eeb5 ba40 	(vcmp\.f32	s22, #0.0|fcmpzs	s22)
0+2a4 <[^>]*> eef5 ba40 	(vcmp\.f32	s23, #0.0|fcmpzs	s23)
0+2a8 <[^>]*> eeb5 ca40 	(vcmp\.f32	s24, #0.0|fcmpzs	s24)
0+2ac <[^>]*> eef5 ca40 	(vcmp\.f32	s25, #0.0|fcmpzs	s25)
0+2b0 <[^>]*> eeb5 da40 	(vcmp\.f32	s26, #0.0|fcmpzs	s26)
0+2b4 <[^>]*> eef5 da40 	(vcmp\.f32	s27, #0.0|fcmpzs	s27)
0+2b8 <[^>]*> eeb5 ea40 	(vcmp\.f32	s28, #0.0|fcmpzs	s28)
0+2bc <[^>]*> eef5 ea40 	(vcmp\.f32	s29, #0.0|fcmpzs	s29)
0+2c0 <[^>]*> eeb5 fa40 	(vcmp\.f32	s30, #0.0|fcmpzs	s30)
0+2c4 <[^>]*> eef5 fa40 	(vcmp\.f32	s31, #0.0|fcmpzs	s31)
0+2c8 <[^>]*> bf01      	itttt	eq
0+2ca <[^>]*> eef1 fa10 	(vmrseq	APSR_nzcv, fpscr|fmstateq)
0+2ce <[^>]*> eef4 1ae3 	(vcmpeeq\.f32|fcmpeseq)	s3, s7
0+2d2 <[^>]*> eef5 2ac0 	(vcmpeeq\.f32	s5, #0.0|fcmpezseq	s5)
0+2d6 <[^>]*> eef4 0a41 	(vcmpeq\.f32|fcmpseq)	s1, s2
0+2da <[^>]*> bf01      	itttt	eq
0+2dc <[^>]*> eef5 0a40 	(vcmpeq\.f32	s1, #0.0|fcmpzseq	s1)
0+2e0 <[^>]*> eef0 0ae1 	(vabseq\.f32|fabsseq)	s1, s3
0+2e4 <[^>]*> eef0 fa69 	(vmoveq\.f32|fcpyseq)	s31, s19
0+2e8 <[^>]*> eeb1 aa44 	(vnegeq\.f32|fnegseq)	s20, s8
0+2ec <[^>]*> bf01      	itttt	eq
0+2ee <[^>]*> eef1 2ae3 	(vsqrteq\.f32|fsqrtseq)	s5, s7
0+2f2 <[^>]*> ee32 3a82 	(vaddeq\.f32|faddseq)	s6, s5, s4
0+2f6 <[^>]*> eec1 1a20 	(vdiveq\.f32|fdivseq)	s3, s2, s1
0+2fa <[^>]*> ee4f fa2e 	(vmlaeq\.f32|fmacseq)	s31, s30, s29
0+2fe <[^>]*> bf01      	itttt	eq
0+300 <[^>]*> ee1d ea8d 	(vnmlseq\.f32|fmscseq)	s28, s27, s26
0+304 <[^>]*> ee6c ca2b 	(vmuleq\.f32|fmulseq)	s25, s24, s23
0+308 <[^>]*> ee0a baca 	(vmlseq\.f32|fnmacseq)	s22, s21, s20
0+30c <[^>]*> ee59 9a68 	(vnmlaeq\.f32|fnmscseq)	s19, s18, s17
0+310 <[^>]*> bf01      	itttt	eq
0+312 <[^>]*> ee27 8ac7 	(vnmuleq\.f32|fnmulseq)	s16, s15, s14
0+316 <[^>]*> ee76 6a65 	(vsubeq\.f32|fsubseq)	s13, s12, s11
0+31a <[^>]*> ed98 5a00 	(vldreq|fldseq)	s10, \[r8\]
0+31e <[^>]*> edc7 4a00 	(vstreq|fstseq)	s9, \[r7\]
0+322 <[^>]*> bf01      	itttt	eq
0+324 <[^>]*> ec91 4a01 	(vldmiaeq|fldmiaseq)	r1, {s8}
0+328 <[^>]*> ecd2 3a01 	(vldmiaeq|fldmiaseq)	r2, {s7}
0+32c <[^>]*> ecb3 3a01 	(vldmiaeq|fldmiaseq)	r3!, {s6}
0+330 <[^>]*> ecf4 2a01 	(vldmiaeq|fldmiaseq)	r4!, {s5}
0+334 <[^>]*> bf01      	itttt	eq
0+336 <[^>]*> ed35 2a01 	(vldmdbeq|fldmdbseq)	r5!, {s4}
0+33a <[^>]*> ed76 1a01 	(vldmdbeq|fldmdbseq)	r6!, {s3}
0+33e <[^>]*> ec97 1b03 	fldmiaxeq	r7, {d1}(	@ Deprecated|)
0+342 <[^>]*> ec98 2b03 	fldmiaxeq	r8, {d2}(	@ Deprecated|)
0+346 <[^>]*> bf01      	itttt	eq
0+348 <[^>]*> ecb9 3b03 	fldmiaxeq	r9!, {d3}(	@ Deprecated|)
0+34c <[^>]*> ecba 4b03 	fldmiaxeq	sl!, {d4}(	@ Deprecated|)
0+350 <[^>]*> ed3b 5b03 	fldmdbxeq	fp!, {d5}(	@ Deprecated|)
0+354 <[^>]*> ed3c 6b03 	fldmdbxeq	ip!, {d6}(	@ Deprecated|)
0+358 <[^>]*> bf01      	itttt	eq
0+35a <[^>]*> ec8d 1a01 	(vstmiaeq|fstmiaseq)	sp, {s2}
0+35e <[^>]*> ecce 0a01 	(vstmiaeq|fstmiaseq)	lr, {s1}
0+362 <[^>]*> ece1 fa01 	(vstmiaeq|fstmiaseq)	r1!, {s31}
0+366 <[^>]*> eca2 fa01 	(vstmiaeq|fstmiaseq)	r2!, {s30}
0+36a <[^>]*> bf01      	itttt	eq
0+36c <[^>]*> ed63 ea01 	(vstmdbeq|fstmdbseq)	r3!, {s29}
0+370 <[^>]*> ed24 ea01 	(vstmdbeq|fstmdbseq)	r4!, {s28}
0+374 <[^>]*> ec85 7b03 	fstmiaxeq	r5, {d7}(	@ Deprecated|)
0+378 <[^>]*> ec86 8b03 	fstmiaxeq	r6, {d8}(	@ Deprecated|)
0+37c <[^>]*> bf01      	itttt	eq
0+37e <[^>]*> eca7 9b03 	fstmiaxeq	r7!, {d9}(	@ Deprecated|)
0+382 <[^>]*> eca8 ab03 	fstmiaxeq	r8!, {d10}(	@ Deprecated|)
0+386 <[^>]*> ed29 bb03 	fstmdbxeq	r9!, {d11}(	@ Deprecated|)
0+38a <[^>]*> ed2a cb03 	fstmdbxeq	sl!, {d12}(	@ Deprecated|)
0+38e <[^>]*> bf01      	itttt	eq
0+390 <[^>]*> eef8 dac3 	(vcvteq\.f32\.s32|fsitoseq)	s27, s6
0+394 <[^>]*> eefd ca62 	(vcvtreq\.s32\.f32|ftosiseq)	s25, s5
0+398 <[^>]*> eefd bac2 	(vcvteq\.s32\.f32|ftosizseq)	s23, s4
0+39c <[^>]*> eefc aa61 	(vcvtreq\.u32\.f32|ftouiseq)	s21, s3
0+3a0 <[^>]*> bf01      	itttt	eq
0+3a2 <[^>]*> eefc 9ac1 	(vcvteq\.u32\.f32|ftouizseq)	s19, s2
0+3a6 <[^>]*> eef8 8a60 	(vcvteq\.f32\.u32|fuitoseq)	s17, s1
0+3aa <[^>]*> ee11 ba90 	(vmoveq|fmrseq)	fp, s3
0+3ae <[^>]*> eef0 9a10 	(vmrseq|fmrxeq)	r9, fpsid
0+3b2 <[^>]*> bf04      	itt	eq
0+3b4 <[^>]*> ee01 9a90 	(vmoveq|fmsreq)	s3, r9
0+3b8 <[^>]*> eee0 8a10 	(vmsreq|fmxreq)	fpsid, r8
0+3bc <[^>]*> eef9 0a10 	(vmrs|fmrx)	r0, fpinst	@ Impl def
0+3c0 <[^>]*> eefa 0a10 	(vmrs|fmrx)	r0, fpinst2	@ Impl def
0+3c4 <[^>]*> eef7 0a10 	(vmrs|fmrx)	r0, mvfr0
0+3c8 <[^>]*> eef6 0a10 	(vmrs|fmrx)	r0, mvfr1
0+3cc <[^>]*> eefc 0a10 	(vmrs|fmrx)	r0, (<impl def 0xc>|vpr)
0+3d0 <[^>]*> eee9 0a10 	(vmsr|fmxr)	fpinst, r0	@ Impl def
0+3d4 <[^>]*> eeea 0a10 	(vmsr|fmxr)	fpinst2, r0	@ Impl def
0+3d8 <[^>]*> eee7 0a10 	(vmsr|fmxr)	mvfr0, r0
0+3dc <[^>]*> eee6 0a10 	(vmsr|fmxr)	mvfr1, r0
0+3e0 <[^>]*> eeec 0a10 	(vmsr|fmxr)	(<impl def 0xc>|vpr), r0
0+3e4 <[^>]*> bf00      	nop
0+3e6 <[^>]*> bf00      	nop
0+3e8 <[^>]*> bf00      	nop
0+3ea <[^>]*> bf00      	nop
0+3ec <[^>]*> bf00      	nop
0+3ee <[^>]*> bf00      	nop
