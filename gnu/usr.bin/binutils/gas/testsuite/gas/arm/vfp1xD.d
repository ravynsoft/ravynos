#objdump: -dr --prefix-addresses --show-raw-insn
#name: VFP Single-precision instructions
#as: -mfpu=vfpxd

# Test the ARM VFP Single Precision instructions

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <[^>]*> eef1fa10 	(vmrs	APSR_nzcv, fpscr|fmstat)
0+004 <[^>]*> eeb40ac0 	(vcmpe\.f32|fcmpes)	s0, s0
0+008 <[^>]*> eeb50ac0 	(vcmpe\.f32	s0, #0.0|fcmpezs	s0)
0+00c <[^>]*> eeb40a40 	(vcmp\.f32|fcmps)	s0, s0
0+010 <[^>]*> eeb50a40 	(vcmp\.f32	s0, #0.0|fcmpzs	s0)
0+014 <[^>]*> eeb00ac0 	(vabs\.f32|fabss)	s0, s0
0+018 <[^>]*> eeb00a40 	(vmov\.f32|fcpys)	s0, s0
0+01c <[^>]*> eeb10a40 	(vneg\.f32|fnegs)	s0, s0
0+020 <[^>]*> eeb10ac0 	(vsqrt\.f32|fsqrts)	s0, s0
0+024 <[^>]*> ee300a00 	(vadd\.f32|fadds)	s0, s0, s0
0+028 <[^>]*> ee800a00 	(vdiv\.f32|fdivs)	s0, s0, s0
0+02c <[^>]*> ee000a00 	(vmla\.f32|fmacs)	s0, s0, s0
0+030 <[^>]*> ee100a00 	(vnmls\.f32|fmscs)	s0, s0, s0
0+034 <[^>]*> ee200a00 	(vmul\.f32|fmuls)	s0, s0, s0
0+038 <[^>]*> ee000a40 	(vmls\.f32|fnmacs)	s0, s0, s0
0+03c <[^>]*> ee100a40 	(vnmla\.f32|fnmscs)	s0, s0, s0
0+040 <[^>]*> ee200a40 	(vnmul\.f32|fnmuls)	s0, s0, s0
0+044 <[^>]*> ee300a40 	(vsub\.f32|fsubs)	s0, s0, s0
0+048 <[^>]*> ed900a00 	(vldr|flds)	s0, \[r0\]
0+04c <[^>]*> ed800a00 	(vstr|fsts)	s0, \[r0\]
0+050 <[^>]*> ec900a01 	(vldmia|fldmias)	r0, {s0}
0+054 <[^>]*> ec900a01 	(vldmia|fldmias)	r0, {s0}
0+058 <[^>]*> ecb00a01 	(vldmia|fldmias)	r0!, {s0}
0+05c <[^>]*> ecb00a01 	(vldmia|fldmias)	r0!, {s0}
0+060 <[^>]*> ed300a01 	(vldmdb|fldmdbs)	r0!, {s0}
0+064 <[^>]*> ed300a01 	(vldmdb|fldmdbs)	r0!, {s0}
0+068 <[^>]*> ec900b03 	fldmiax	r0, {d0}(	@ Deprecated|)
0+06c <[^>]*> ec900b03 	fldmiax	r0, {d0}(	@ Deprecated|)
0+070 <[^>]*> ecb00b03 	fldmiax	r0!, {d0}(	@ Deprecated|)
0+074 <[^>]*> ecb00b03 	fldmiax	r0!, {d0}(	@ Deprecated|)
0+078 <[^>]*> ed300b03 	fldmdbx	r0!, {d0}(	@ Deprecated|)
0+07c <[^>]*> ed300b03 	fldmdbx	r0!, {d0}(	@ Deprecated|)
0+080 <[^>]*> ec800a01 	(vstmia|fstmias)	r0, {s0}
0+084 <[^>]*> ec800a01 	(vstmia|fstmias)	r0, {s0}
0+088 <[^>]*> eca00a01 	(vstmia|fstmias)	r0!, {s0}
0+08c <[^>]*> eca00a01 	(vstmia|fstmias)	r0!, {s0}
0+090 <[^>]*> ed200a01 	(vstmdb|fstmdbs)	r0!, {s0}
0+094 <[^>]*> ed200a01 	(vstmdb|fstmdbs)	r0!, {s0}
0+098 <[^>]*> ec800b03 	fstmiax	r0, {d0}(	@ Deprecated|)
0+09c <[^>]*> ec800b03 	fstmiax	r0, {d0}(	@ Deprecated|)
0+0a0 <[^>]*> eca00b03 	fstmiax	r0!, {d0}(	@ Deprecated|)
0+0a4 <[^>]*> eca00b03 	fstmiax	r0!, {d0}(	@ Deprecated|)
0+0a8 <[^>]*> ed200b03 	fstmdbx	r0!, {d0}(	@ Deprecated|)
0+0ac <[^>]*> ed200b03 	fstmdbx	r0!, {d0}(	@ Deprecated|)
0+0b0 <[^>]*> eeb80ac0 	(vcvt\.f32\.s32|fsitos)	s0, s0
0+0b4 <[^>]*> eeb80a40 	(vcvt\.f32\.u32|fuitos)	s0, s0
0+0b8 <[^>]*> eebd0a40 	(vcvtr\.s32\.f32|ftosis)	s0, s0
0+0bc <[^>]*> eebd0ac0 	(vcvt\.s32\.f32|ftosizs)	s0, s0
0+0c0 <[^>]*> eebc0a40 	(vcvtr\.u32\.f32|ftouis)	s0, s0
0+0c4 <[^>]*> eebc0ac0 	(vcvt\.u32\.f32|ftouizs)	s0, s0
0+0c8 <[^>]*> ee100a10 	(vmov|fmrs)	r0, s0
0+0cc <[^>]*> eef00a10 	(vmrs|fmrx)	r0, fpsid
0+0d0 <[^>]*> eef10a10 	(vmrs|fmrx)	r0, fpscr
0+0d4 <[^>]*> eef80a10 	(vmrs|fmrx)	r0, fpexc
0+0d8 <[^>]*> ee000a10 	(vmov|fmsr)	s0, r0
0+0dc <[^>]*> eee00a10 	(vmsr|fmxr)	fpsid, r0
0+0e0 <[^>]*> eee10a10 	(vmsr|fmxr)	fpscr, r0
0+0e4 <[^>]*> eee80a10 	(vmsr|fmxr)	fpexc, r0
0+0e8 <[^>]*> eef50a40 	(vcmp\.f32	s1, #0.0|fcmpzs	s1)
0+0ec <[^>]*> eeb51a40 	(vcmp\.f32	s2, #0.0|fcmpzs	s2)
0+0f0 <[^>]*> eef5fa40 	(vcmp\.f32	s31, #0.0|fcmpzs	s31)
0+0f4 <[^>]*> eeb40a60 	(vcmp\.f32|fcmps)	s0, s1
0+0f8 <[^>]*> eeb40a41 	(vcmp\.f32|fcmps)	s0, s2
0+0fc <[^>]*> eeb40a6f 	(vcmp\.f32|fcmps)	s0, s31
0+100 <[^>]*> eef40a40 	(vcmp\.f32|fcmps)	s1, s0
0+104 <[^>]*> eeb41a40 	(vcmp\.f32|fcmps)	s2, s0
0+108 <[^>]*> eef4fa40 	(vcmp\.f32|fcmps)	s31, s0
0+10c <[^>]*> eef4aa46 	(vcmp\.f32|fcmps)	s21, s12
0+110 <[^>]*> eeb10a60 	(vneg\.f32|fnegs)	s0, s1
0+114 <[^>]*> eeb10a41 	(vneg\.f32|fnegs)	s0, s2
0+118 <[^>]*> eeb10a6f 	(vneg\.f32|fnegs)	s0, s31
0+11c <[^>]*> eef10a40 	(vneg\.f32|fnegs)	s1, s0
0+120 <[^>]*> eeb11a40 	(vneg\.f32|fnegs)	s2, s0
0+124 <[^>]*> eef1fa40 	(vneg\.f32|fnegs)	s31, s0
0+128 <[^>]*> eeb16a6a 	(vneg\.f32|fnegs)	s12, s21
0+12c <[^>]*> ee300a20 	(vadd\.f32|fadds)	s0, s0, s1
0+130 <[^>]*> ee300a01 	(vadd\.f32|fadds)	s0, s0, s2
0+134 <[^>]*> ee300a2f 	(vadd\.f32|fadds)	s0, s0, s31
0+138 <[^>]*> ee300a80 	(vadd\.f32|fadds)	s0, s1, s0
0+13c <[^>]*> ee310a00 	(vadd\.f32|fadds)	s0, s2, s0
0+140 <[^>]*> ee3f0a80 	(vadd\.f32|fadds)	s0, s31, s0
0+144 <[^>]*> ee700a00 	(vadd\.f32|fadds)	s1, s0, s0
0+148 <[^>]*> ee301a00 	(vadd\.f32|fadds)	s2, s0, s0
0+14c <[^>]*> ee70fa00 	(vadd\.f32|fadds)	s31, s0, s0
0+150 <[^>]*> ee3a6aa2 	(vadd\.f32|fadds)	s12, s21, s5
0+154 <[^>]*> eeb80ae0 	(vcvt\.f32\.s32|fsitos)	s0, s1
0+158 <[^>]*> eeb80ac1 	(vcvt\.f32\.s32|fsitos)	s0, s2
0+15c <[^>]*> eeb80aef 	(vcvt\.f32\.s32|fsitos)	s0, s31
0+160 <[^>]*> eef80ac0 	(vcvt\.f32\.s32|fsitos)	s1, s0
0+164 <[^>]*> eeb81ac0 	(vcvt\.f32\.s32|fsitos)	s2, s0
0+168 <[^>]*> eef8fac0 	(vcvt\.f32\.s32|fsitos)	s31, s0
0+16c <[^>]*> eebd0a60 	(vcvtr\.s32\.f32|ftosis)	s0, s1
0+170 <[^>]*> eebd0a41 	(vcvtr\.s32\.f32|ftosis)	s0, s2
0+174 <[^>]*> eebd0a6f 	(vcvtr\.s32\.f32|ftosis)	s0, s31
0+178 <[^>]*> eefd0a40 	(vcvtr\.s32\.f32|ftosis)	s1, s0
0+17c <[^>]*> eebd1a40 	(vcvtr\.s32\.f32|ftosis)	s2, s0
0+180 <[^>]*> eefdfa40 	(vcvtr\.s32\.f32|ftosis)	s31, s0
0+184 <[^>]*> ee001a10 	(vmov|fmsr)	s0, r1
0+188 <[^>]*> ee007a10 	(vmov|fmsr)	s0, r7
0+18c <[^>]*> ee00ea10 	(vmov|fmsr)	s0, lr
0+190 <[^>]*> ee000a90 	(vmov|fmsr)	s1, r0
0+194 <[^>]*> ee010a10 	(vmov|fmsr)	s2, r0
0+198 <[^>]*> ee0f0a90 	(vmov|fmsr)	s31, r0
0+19c <[^>]*> ee0a7a90 	(vmov|fmsr)	s21, r7
0+1a0 <[^>]*> eee01a10 	(vmsr|fmxr)	fpsid, r1
0+1a4 <[^>]*> eee0ea10 	(vmsr|fmxr)	fpsid, lr
0+1a8 <[^>]*> ee100a90 	(vmov|fmrs)	r0, s1
0+1ac <[^>]*> ee110a10 	(vmov|fmrs)	r0, s2
0+1b0 <[^>]*> ee1f0a90 	(vmov|fmrs)	r0, s31
0+1b4 <[^>]*> ee101a10 	(vmov|fmrs)	r1, s0
0+1b8 <[^>]*> ee107a10 	(vmov|fmrs)	r7, s0
0+1bc <[^>]*> ee10ea10 	(vmov|fmrs)	lr, s0
0+1c0 <[^>]*> ee159a90 	(vmov|fmrs)	r9, s11
0+1c4 <[^>]*> eef01a10 	(vmrs|fmrx)	r1, fpsid
0+1c8 <[^>]*> eef0ea10 	(vmrs|fmrx)	lr, fpsid
0+1cc <[^>]*> ed910a00 	(vldr|flds)	s0, \[r1\]
0+1d0 <[^>]*> ed9e0a00 	(vldr|flds)	s0, \[lr\]
0+1d4 <[^>]*> ed900a00 	(vldr|flds)	s0, \[r0\]
0+1d8 <[^>]*> ed900aff 	(vldr|flds)	s0, \[r0, #1020\].*
0+1dc <[^>]*> ed100aff 	(vldr|flds)	s0, \[r0, #-1020\].*
0+1e0 <[^>]*> edd00a00 	(vldr|flds)	s1, \[r0\]
0+1e4 <[^>]*> ed901a00 	(vldr|flds)	s2, \[r0\]
0+1e8 <[^>]*> edd0fa00 	(vldr|flds)	s31, \[r0\]
0+1ec <[^>]*> edccaac9 	(vstr|fsts)	s21, \[ip, #804\].*
0+1f0 <[^>]*> ecd00a01 	(vldmia|fldmias)	r0, {s1}
0+1f4 <[^>]*> ec901a01 	(vldmia|fldmias)	r0, {s2}
0+1f8 <[^>]*> ecd0fa01 	(vldmia|fldmias)	r0, {s31}
0+1fc <[^>]*> ec900a02 	(vldmia|fldmias)	r0, {s0-s1}
0+200 <[^>]*> ec900a03 	(vldmia|fldmias)	r0, {s0-s2}
0+204 <[^>]*> ec900a20 	(vldmia|fldmias)	r0, {s0-s31}
0+208 <[^>]*> ecd00a1f 	(vldmia|fldmias)	r0, {s1-s31}
0+20c <[^>]*> ec901a1e 	(vldmia|fldmias)	r0, {s2-s31}
0+210 <[^>]*> ec90fa02 	(vldmia|fldmias)	r0, {s30-s31}
0+214 <[^>]*> ec910a01 	(vldmia|fldmias)	r1, {s0}
0+218 <[^>]*> ec9e0a01 	(vldmia|fldmias)	lr, {s0}
0+21c <[^>]*> ec801b03 	fstmiax	r0, {d1}(	@ Deprecated|)
0+220 <[^>]*> ec802b03 	fstmiax	r0, {d2}(	@ Deprecated|)
0+224 <[^>]*> ec80fb03 	fstmiax	r0, {d15}(	@ Deprecated|)
0+228 <[^>]*> ec800b05 	fstmiax	r0, {d0-d1}(	@ Deprecated|)
0+22c <[^>]*> ec800b07 	fstmiax	r0, {d0-d2}(	@ Deprecated|)
0+230 <[^>]*> ec800b21 	fstmiax	r0, {d0-d15}(	@ Deprecated|)
0+234 <[^>]*> ec801b1f 	fstmiax	r0, {d1-d15}(	@ Deprecated|)
0+238 <[^>]*> ec802b1d 	fstmiax	r0, {d2-d15}(	@ Deprecated|)
0+23c <[^>]*> ec80eb05 	fstmiax	r0, {d14-d15}(	@ Deprecated|)
0+240 <[^>]*> ec810b03 	fstmiax	r1, {d0}(	@ Deprecated|)
0+244 <[^>]*> ec8e0b03 	fstmiax	lr, {d0}(	@ Deprecated|)
0+248 <[^>]*> eeb50a40 	(vcmp\.f32	s0, #0.0|fcmpzs	s0)
0+24c <[^>]*> eef50a40 	(vcmp\.f32	s1, #0.0|fcmpzs	s1)
0+250 <[^>]*> eeb51a40 	(vcmp\.f32	s2, #0.0|fcmpzs	s2)
0+254 <[^>]*> eef51a40 	(vcmp\.f32	s3, #0.0|fcmpzs	s3)
0+258 <[^>]*> eeb52a40 	(vcmp\.f32	s4, #0.0|fcmpzs	s4)
0+25c <[^>]*> eef52a40 	(vcmp\.f32	s5, #0.0|fcmpzs	s5)
0+260 <[^>]*> eeb53a40 	(vcmp\.f32	s6, #0.0|fcmpzs	s6)
0+264 <[^>]*> eef53a40 	(vcmp\.f32	s7, #0.0|fcmpzs	s7)
0+268 <[^>]*> eeb54a40 	(vcmp\.f32	s8, #0.0|fcmpzs	s8)
0+26c <[^>]*> eef54a40 	(vcmp\.f32	s9, #0.0|fcmpzs	s9)
0+270 <[^>]*> eeb55a40 	(vcmp\.f32	s10, #0.0|fcmpzs	s10)
0+274 <[^>]*> eef55a40 	(vcmp\.f32	s11, #0.0|fcmpzs	s11)
0+278 <[^>]*> eeb56a40 	(vcmp\.f32	s12, #0.0|fcmpzs	s12)
0+27c <[^>]*> eef56a40 	(vcmp\.f32	s13, #0.0|fcmpzs	s13)
0+280 <[^>]*> eeb57a40 	(vcmp\.f32	s14, #0.0|fcmpzs	s14)
0+284 <[^>]*> eef57a40 	(vcmp\.f32	s15, #0.0|fcmpzs	s15)
0+288 <[^>]*> eeb58a40 	(vcmp\.f32	s16, #0.0|fcmpzs	s16)
0+28c <[^>]*> eef58a40 	(vcmp\.f32	s17, #0.0|fcmpzs	s17)
0+290 <[^>]*> eeb59a40 	(vcmp\.f32	s18, #0.0|fcmpzs	s18)
0+294 <[^>]*> eef59a40 	(vcmp\.f32	s19, #0.0|fcmpzs	s19)
0+298 <[^>]*> eeb5aa40 	(vcmp\.f32	s20, #0.0|fcmpzs	s20)
0+29c <[^>]*> eef5aa40 	(vcmp\.f32	s21, #0.0|fcmpzs	s21)
0+2a0 <[^>]*> eeb5ba40 	(vcmp\.f32	s22, #0.0|fcmpzs	s22)
0+2a4 <[^>]*> eef5ba40 	(vcmp\.f32	s23, #0.0|fcmpzs	s23)
0+2a8 <[^>]*> eeb5ca40 	(vcmp\.f32	s24, #0.0|fcmpzs	s24)
0+2ac <[^>]*> eef5ca40 	(vcmp\.f32	s25, #0.0|fcmpzs	s25)
0+2b0 <[^>]*> eeb5da40 	(vcmp\.f32	s26, #0.0|fcmpzs	s26)
0+2b4 <[^>]*> eef5da40 	(vcmp\.f32	s27, #0.0|fcmpzs	s27)
0+2b8 <[^>]*> eeb5ea40 	(vcmp\.f32	s28, #0.0|fcmpzs	s28)
0+2bc <[^>]*> eef5ea40 	(vcmp\.f32	s29, #0.0|fcmpzs	s29)
0+2c0 <[^>]*> eeb5fa40 	(vcmp\.f32	s30, #0.0|fcmpzs	s30)
0+2c4 <[^>]*> eef5fa40 	(vcmp\.f32	s31, #0.0|fcmpzs	s31)
0+2c8 <[^>]*> 0ef1fa10 	(vmrseq	APSR_nzcv, fpscr|fmstateq)
0+2cc <[^>]*> 0ef41ae3 	(vcmpeeq\.f32|fcmpeseq)	s3, s7
0+2d0 <[^>]*> 0ef52ac0 	(vcmpeeq\.f32	s5, #0.0|fcmpezseq	s5)
0+2d4 <[^>]*> 0ef40a41 	(vcmpeq\.f32|fcmpseq)	s1, s2
0+2d8 <[^>]*> 0ef50a40 	(vcmpeq\.f32	s1, #0.0|fcmpzseq	s1)
0+2dc <[^>]*> 0ef00ae1 	(vabseq\.f32|fabsseq)	s1, s3
0+2e0 <[^>]*> 0ef0fa69 	(vmoveq\.f32|fcpyseq)	s31, s19
0+2e4 <[^>]*> 0eb1aa44 	(vnegeq\.f32|fnegseq)	s20, s8
0+2e8 <[^>]*> 0ef12ae3 	(vsqrteq\.f32|fsqrtseq)	s5, s7
0+2ec <[^>]*> 0e323a82 	(vaddeq\.f32|faddseq)	s6, s5, s4
0+2f0 <[^>]*> 0ec11a20 	(vdiveq\.f32|fdivseq)	s3, s2, s1
0+2f4 <[^>]*> 0e4ffa2e 	(vmlaeq\.f32|fmacseq)	s31, s30, s29
0+2f8 <[^>]*> 0e1dea8d 	(vnmlseq\.f32|fmscseq)	s28, s27, s26
0+2fc <[^>]*> 0e6cca2b 	(vmuleq\.f32|fmulseq)	s25, s24, s23
0+300 <[^>]*> 0e0abaca 	(vmlseq\.f32|fnmacseq)	s22, s21, s20
0+304 <[^>]*> 0e599a68 	(vnmlaeq\.f32|fnmscseq)	s19, s18, s17
0+308 <[^>]*> 0e278ac7 	(vnmuleq\.f32|fnmulseq)	s16, s15, s14
0+30c <[^>]*> 0e766a65 	(vsubeq\.f32|fsubseq)	s13, s12, s11
0+310 <[^>]*> 0d985a00 	(vldreq|fldseq)	s10, \[r8\]
0+314 <[^>]*> 0dc74a00 	(vstreq|fstseq)	s9, \[r7\]
0+318 <[^>]*> 0c914a01 	(vldmiaeq|fldmiaseq)	r1, {s8}
0+31c <[^>]*> 0cd23a01 	(vldmiaeq|fldmiaseq)	r2, {s7}
0+320 <[^>]*> 0cb33a01 	(vldmiaeq|fldmiaseq)	r3!, {s6}
0+324 <[^>]*> 0cf42a01 	(vldmiaeq|fldmiaseq)	r4!, {s5}
0+328 <[^>]*> 0d352a01 	(vldmdbeq|fldmdbseq)	r5!, {s4}
0+32c <[^>]*> 0d761a01 	(vldmdbeq|fldmdbseq)	r6!, {s3}
0+330 <[^>]*> 0c971b03 	fldmiaxeq	r7, {d1}(	@ Deprecated|)
0+334 <[^>]*> 0c982b03 	fldmiaxeq	r8, {d2}(	@ Deprecated|)
0+338 <[^>]*> 0cb93b03 	fldmiaxeq	r9!, {d3}(	@ Deprecated|)
0+33c <[^>]*> 0cba4b03 	fldmiaxeq	sl!, {d4}(	@ Deprecated|)
0+340 <[^>]*> 0d3b5b03 	fldmdbxeq	fp!, {d5}(	@ Deprecated|)
0+344 <[^>]*> 0d3c6b03 	fldmdbxeq	ip!, {d6}(	@ Deprecated|)
0+348 <[^>]*> 0c8d1a01 	(vstmiaeq|fstmiaseq)	sp, {s2}
0+34c <[^>]*> 0cce0a01 	(vstmiaeq|fstmiaseq)	lr, {s1}
0+350 <[^>]*> 0ce1fa01 	(vstmiaeq|fstmiaseq)	r1!, {s31}
0+354 <[^>]*> 0ca2fa01 	(vstmiaeq|fstmiaseq)	r2!, {s30}
0+358 <[^>]*> 0d63ea01 	(vstmdbeq|fstmdbseq)	r3!, {s29}
0+35c <[^>]*> 0d24ea01 	(vstmdbeq|fstmdbseq)	r4!, {s28}
0+360 <[^>]*> 0c857b03 	fstmiaxeq	r5, {d7}(	@ Deprecated|)
0+364 <[^>]*> 0c868b03 	fstmiaxeq	r6, {d8}(	@ Deprecated|)
0+368 <[^>]*> 0ca79b03 	fstmiaxeq	r7!, {d9}(	@ Deprecated|)
0+36c <[^>]*> 0ca8ab03 	fstmiaxeq	r8!, {d10}(	@ Deprecated|)
0+370 <[^>]*> 0d29bb03 	fstmdbxeq	r9!, {d11}(	@ Deprecated|)
0+374 <[^>]*> 0d2acb03 	fstmdbxeq	sl!, {d12}(	@ Deprecated|)
0+378 <[^>]*> 0ef8dac3 	(vcvteq\.f32\.s32|fsitoseq)	s27, s6
0+37c <[^>]*> 0efdca62 	(vcvtreq\.s32\.f32|ftosiseq)	s25, s5
0+380 <[^>]*> 0efdbac2 	(vcvteq\.s32\.f32|ftosizseq)	s23, s4
0+384 <[^>]*> 0efcaa61 	(vcvtreq\.u32\.f32|ftouiseq)	s21, s3
0+388 <[^>]*> 0efc9ac1 	(vcvteq\.u32\.f32|ftouizseq)	s19, s2
0+38c <[^>]*> 0ef88a60 	(vcvteq\.f32\.u32|fuitoseq)	s17, s1
0+390 <[^>]*> 0e11ba90 	(vmoveq|fmrseq)	fp, s3
0+394 <[^>]*> 0ef09a10 	(vmrseq|fmrxeq)	r9, fpsid
0+398 <[^>]*> 0e019a90 	(vmoveq|fmsreq)	s3, r9
0+39c <[^>]*> 0ee08a10 	(vmsreq|fmxreq)	fpsid, r8
0+3a0 <[^>]*> eef90a10 	(vmrs|fmrx)	r0, fpinst	@ Impl def
0+3a4 <[^>]*> eefa0a10 	(vmrs|fmrx)	r0, fpinst2	@ Impl def
0+3a8 <[^>]*> eef70a10 	(vmrs|fmrx)	r0, mvfr0
0+3ac <[^>]*> eef60a10 	(vmrs|fmrx)	r0, mvfr1
0+3b0 <[^>]*> eefc0a10 	(vmrs|fmrx)	r0, (vpr|<impl def 0xc>)
0+3b4 <[^>]*> eee90a10 	(vmsr|fmxr)	fpinst, r0	@ Impl def
0+3b8 <[^>]*> eeea0a10 	(vmsr|fmxr)	fpinst2, r0	@ Impl def
0+3bc <[^>]*> eee70a10 	(vmsr|fmxr)	mvfr0, r0
0+3c0 <[^>]*> eee60a10 	(vmsr|fmxr)	mvfr1, r0
0+3c4 <[^>]*> eeec0a10 	(vmsr|fmxr)	(vpr|<impl def 0xc>), r0
0+3c8 <[^>]*> eef10a10 	vmrs	r0, fpscr
0+3cc <[^>]*> eef11a10 	vmrs	r1, fpscr
0+3d0 <[^>]*> eef12a10 	vmrs	r2, fpscr
0+3d4 <[^>]*> eef13a10 	vmrs	r3, fpscr
0+3d8 <[^>]*> eef14a10 	vmrs	r4, fpscr
0+3dc <[^>]*> eef15a10 	vmrs	r5, fpscr
0+3e0 <[^>]*> eef16a10 	vmrs	r6, fpscr
0+3e4 <[^>]*> eef17a10 	vmrs	r7, fpscr
0+3e8 <[^>]*> eef18a10 	vmrs	r8, fpscr
0+3ec <[^>]*> eef19a10 	vmrs	r9, fpscr
0+3f0 <[^>]*> eef1aa10 	vmrs	sl, fpscr
0+3f4 <[^>]*> eef1ba10 	vmrs	fp, fpscr
0+3f8 <[^>]*> eef1ca10 	vmrs	ip, fpscr
0+3fc <[^>]*> eef1ea10 	vmrs	lr, fpscr
0+400 <[^>]*> eef1fa10 	vmrs	APSR_nzcv, fpscr
0+404 <[^>]*> eee10a10 	vmsr	fpscr, r0
0+408 <[^>]*> eee11a10 	vmsr	fpscr, r1
0+40c <[^>]*> eee12a10 	vmsr	fpscr, r2
0+410 <[^>]*> eee13a10 	vmsr	fpscr, r3
0+414 <[^>]*> eee14a10 	vmsr	fpscr, r4
0+418 <[^>]*> eee15a10 	vmsr	fpscr, r5
0+41c <[^>]*> eee16a10 	vmsr	fpscr, r6
0+420 <[^>]*> eee17a10 	vmsr	fpscr, r7
0+424 <[^>]*> eee18a10 	vmsr	fpscr, r8
0+428 <[^>]*> eee19a10 	vmsr	fpscr, r9
0+42c <[^>]*> eee1aa10 	vmsr	fpscr, sl
0+430 <[^>]*> eee1ba10 	vmsr	fpscr, fp
0+434 <[^>]*> eee1ca10 	vmsr	fpscr, ip
0+438 <[^>]*> eee1ea10 	vmsr	fpscr, lr
0+43c <[^>]*> eee01a10 	vmsr	fpsid, r1
0+440 <[^>]*> eee82a10 	vmsr	fpexc, r2
0+444 <[^>]*> eee93a10 	vmsr	fpinst, r3	@ Impl def
0+448 <[^>]*> eeea4a10 	vmsr	fpinst2, r4	@ Impl def
0+44c <[^>]*> eeef5a10 	vmsr	(c15|<impl def 0xf>|fpcxt_s), r5
0+450 <[^>]*> eef03a10 	vmrs	r3, fpsid
0+454 <[^>]*> eef64a10 	vmrs	r4, mvfr1
0+458 <[^>]*> eef75a10 	vmrs	r5, mvfr0
0+45c <[^>]*> eef86a10 	vmrs	r6, fpexc
0+460 <[^>]*> eef97a10 	vmrs	r7, fpinst	@ Impl def
0+464 <[^>]*> eefa8a10 	vmrs	r8, fpinst2	@ Impl def
0+468 <[^>]*> eeff9a10 	vmrs	r9, (c15|<impl def 0xf>|fpcxt_s)
0+46c <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
0+470 <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
0+474 <[^>]*> e1a00000 	nop			@ \(mov r0, r0\)
