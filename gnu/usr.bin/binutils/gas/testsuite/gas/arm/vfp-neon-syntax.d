# name: VFP Neon-style syntax, ARM mode
# as: -mfpu=vfp3 -I$srcdir/$subdir
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> eeb00a60 	(vmov\.f32|fcpys)	s0, s1
0[0-9a-f]+ <[^>]+> eeb00b41 	(vmov\.f64|fcpyd)	d0, d1
0[0-9a-f]+ <[^>]+> eeb50a00 	(vmov\.f32|fconsts)	s0, #80.*
0[0-9a-f]+ <[^>]+> eeb70b00 	(vmov\.f64|fconstd)	d0, #112.*
0[0-9a-f]+ <[^>]+> ee100a90 	(vmov|fmrs)	r0, s1
0[0-9a-f]+ <[^>]+> ee001a10 	(vmov|fmsr)	s0, r1
0[0-9a-f]+ <[^>]+> ec510a11 	(vmov	r0, r1, s2, s3|fmrrs	r0, r1, {s2, s3})
0[0-9a-f]+ <[^>]+> ec442a10 	(vmov	s0, s1, r2, r4|fmsrr	{s0, s1}, r2, r4)
0[0-9a-f]+ <[^>]+> 0eb00a60 	(vmoveq\.f32|fcpyseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb00b41 	(vmoveq\.f64|fcpydeq)	d0, d1
0[0-9a-f]+ <[^>]+> 0eb50a00 	(vmoveq\.f32|fconstseq)	s0, #80.*
0[0-9a-f]+ <[^>]+> 0eb70b00 	(vmoveq\.f64|fconstdeq)	d0, #112.*
0[0-9a-f]+ <[^>]+> 0e100a90 	(vmoveq|fmrseq)	r0, s1
0[0-9a-f]+ <[^>]+> 0e001a10 	(vmoveq|fmsreq)	s0, r1
0[0-9a-f]+ <[^>]+> 0c510a11 	(vmoveq	r0, r1, s2, s3|fmrrseq	r0, r1, {s2, s3})
0[0-9a-f]+ <[^>]+> 0c442a10 	(vmoveq	s0, s1, r2, r4|fmsrreq	{s0, s1}, r2, r4)
0[0-9a-f]+ <[^>]+> eeb10ae0 	(vsqrt\.f32|fsqrts)	s0, s1
0[0-9a-f]+ <[^>]+> eeb10bc1 	(vsqrt\.f64|fsqrtd)	d0, d1
0[0-9a-f]+ <[^>]+> 0eb10ae0 	(vsqrteq.f32|fsqrtseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb10bc1 	(vsqrteq.f64|fsqrtdeq)	d0, d1
0[0-9a-f]+ <[^>]+> eeb00ae0 	(vabs\.f32|fabss)	s0, s1
0[0-9a-f]+ <[^>]+> eeb00bc1 	(vabs\.f64|fabsd)	d0, d1
0[0-9a-f]+ <[^>]+> 0eb00ae0 	(vabseq\.f32|fabsseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb00bc1 	(vabseq\.f64|fabsdeq)	d0, d1
0[0-9a-f]+ <[^>]+> eeb10a60 	(vneg\.f32|fnegs)	s0, s1
0[0-9a-f]+ <[^>]+> eeb10b41 	(vneg\.f64|fnegd)	d0, d1
0[0-9a-f]+ <[^>]+> 0eb10a60 	(vnegeq\.f32|fnegseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb10b41 	(vnegeq\.f64|fnegdeq)	d0, d1
0[0-9a-f]+ <[^>]+> eeb40a60 	(vcmp\.f32|fcmps)	s0, s1
0[0-9a-f]+ <[^>]+> eeb40b41 	(vcmp\.f64|fcmpd)	d0, d1
0[0-9a-f]+ <[^>]+> 0eb40a60 	(vcmpeq\.f32|fcmpseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb40b41 	(vcmpeq\.f64|fcmpdeq)	d0, d1
0[0-9a-f]+ <[^>]+> eeb40ae0 	(vcmpe\.f32|fcmpes)	s0, s1
0[0-9a-f]+ <[^>]+> eeb40bc1 	(vcmpe\.f64|fcmped)	d0, d1
0[0-9a-f]+ <[^>]+> 0eb40ae0 	(vcmpeeq\.f32|fcmpeseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb40bc1 	(vcmpeeq\.f64|fcmpedeq)	d0, d1
0[0-9a-f]+ <[^>]+> ee200ac1 	(vnmul\.f32|fnmuls)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee210b42 	(vnmul\.f64|fnmuld)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e200ac1 	(vnmuleq\.f32|fnmulseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e210b42 	(vnmuleq\.f64|fnmuldeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee100ac1 	(vnmla\.f32|fnmacs)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee110b42 	(vnmla\.f64|fnmacd)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e100ac1 	(vnmlaeq\.f32|fnmacseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e110b42 	(vnmlaeq\.f64|fnmacdeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee100a81 	(vnmls\.f32|fnmscs)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee110b02 	(vnmls\.f64|fnmscd)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e100a81 	(vnmlseq\.f32|fnmscseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e110b02 	(vnmlseq\.f64|fnmscdeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee200a81 	(vmul\.f32|fmuls)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee210b02 	(vmul\.f64|fmuld)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e200a81 	(vmuleq\.f32|fmulseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e210b02 	(vmuleq\.f64|fmuldeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee000a81 	(vmla\.f32|fmacs)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee010b02 	(vmla\.f64|fmacd)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e000a81 	(vmlaeq\.f32|fmacseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e010b02 	(vmlaeq\.f64|fmacdeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee000ac1 	(vmls\.f32|fmscs)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee010b42 	(vmls\.f64|fmscd)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e000ac1 	(vmlseq\.f32|fmscseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e010b42 	(vmlseq\.f64|fmscdeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee300a81 	(vadd\.f32|fadds)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee310b02 	(vadd\.f64|faddd)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e300a81 	(vaddeq\.f32|faddseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e310b02 	(vaddeq\.f64|fadddeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee300ac1 	(vsub\.f32|fsubs)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee310b42 	(vsub\.f64|fsubd)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e300ac1 	(vsubeq\.f32|fsubseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e310b42 	(vsubeq\.f64|fsubdeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> ee800a81 	(vdiv\.f32|fdivs)	s0, s1, s2
0[0-9a-f]+ <[^>]+> ee810b02 	(vdiv\.f64|fdivd)	d0, d1, d2
0[0-9a-f]+ <[^>]+> 0e800a81 	(vdiveq\.f32|fdivseq)	s0, s1, s2
0[0-9a-f]+ <[^>]+> 0e810b02 	(vdiveq\.f64|fdivdeq)	d0, d1, d2
0[0-9a-f]+ <[^>]+> eeb50a40 	(vcmp\.f32	s0, #0.0|fcmpzs	s0)
0[0-9a-f]+ <[^>]+> eeb50b40 	(vcmp\.f64	d0, #0.0|fcmpzd	d0)
0[0-9a-f]+ <[^>]+> 0eb50a40 	(vcmpeq\.f32	s0, #0.0|fcmpzseq	s0)
0[0-9a-f]+ <[^>]+> 0eb50b40 	(vcmpeq\.f64	d0, #0.0|fcmpzdeq	d0)
0[0-9a-f]+ <[^>]+> eeb50ac0 	(vcmpe\.f32	s0, #0.0|fcmpezs	s0)
0[0-9a-f]+ <[^>]+> eeb50bc0 	(vcmpe\.f64	d0, #0.0|fcmpezd	d0)
0[0-9a-f]+ <[^>]+> 0eb50ac0 	(vcmpeeq\.f32	s0, #0.0|fcmpezseq	s0)
0[0-9a-f]+ <[^>]+> 0eb50bc0 	(vcmpeeq\.f64	d0, #0.0|fcmpezdeq	d0)
0[0-9a-f]+ <[^>]+> eebd0ae0 	(vcvt\.s32\.f32|ftosizs)	s0, s1
0[0-9a-f]+ <[^>]+> eebc0ae0 	(vcvt\.u32\.f32|ftouizs)	s0, s1
0[0-9a-f]+ <[^>]+> eebd0bc1 	(vcvt\.s32\.f64|ftosizd)	s0, d1
0[0-9a-f]+ <[^>]+> eebc0bc1 	(vcvt\.u32\.f64|ftouizd)	s0, d1
0[0-9a-f]+ <[^>]+> 0ebd0ae0 	(vcvteq\.s32\.f32|ftosizseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0ebc0ae0 	(vcvteq\.u32\.f32|ftouizseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0ebd0bc1 	(vcvteq\.s32\.f64|ftosizdeq)	s0, d1
0[0-9a-f]+ <[^>]+> 0ebc0bc1 	(vcvteq\.u32\.f64|ftouizdeq)	s0, d1
0[0-9a-f]+ <[^>]+> eebd0ae0 	(vcvt\.s32\.f32|ftosis)	s0, s1
0[0-9a-f]+ <[^>]+> eebc0ae0 	(vcvt\.u32\.f32|ftouis)	s0, s1
0[0-9a-f]+ <[^>]+> eeb80ae0 	(vcvt\.f32\.s32|fsitos)	s0, s1
0[0-9a-f]+ <[^>]+> eeb80a60 	(vcvt\.f32\.u32|fuitos)	s0, s1
0[0-9a-f]+ <[^>]+> eeb70bc1 	(vcvt\.f32\.f64|fcvtsd)	s0, d1
0[0-9a-f]+ <[^>]+> eeb70ae0 	(vcvt\.f64\.f32|fcvtds)	d0, s1
0[0-9a-f]+ <[^>]+> eebd0bc1 	(vcvt\.s32\.f64|ftosid)	s0, d1
0[0-9a-f]+ <[^>]+> eebc0bc1 	(vcvt\.u32\.f64|ftouid)	s0, d1
0[0-9a-f]+ <[^>]+> eeb80be0 	(vcvt\.f64\.s32|fsitod)	d0, s1
0[0-9a-f]+ <[^>]+> eeb80b60 	(vcvt\.f64\.u32|fuitod)	d0, s1
0[0-9a-f]+ <[^>]+> 0ebd0ae0 	(vcvteq\.s32\.f32|ftosiseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0ebc0ae0 	(vcvteq\.u32\.f32|ftouiseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb80ae0 	(vcvteq\.f32\.s32|fsitoseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb80a60 	(vcvteq\.f32\.u32|fuitoseq)	s0, s1
0[0-9a-f]+ <[^>]+> 0eb70bc1 	(vcvteq\.f32\.f64|fcvtsdeq)	s0, d1
0[0-9a-f]+ <[^>]+> 0eb70ae0 	(vcvteq\.f64\.f32|fcvtdseq)	d0, s1
0[0-9a-f]+ <[^>]+> 0ebd0bc1 	(vcvteq\.s32\.f64|ftosideq)	s0, d1
0[0-9a-f]+ <[^>]+> 0ebc0bc1 	(vcvteq\.u32\.f64|ftouideq)	s0, d1
0[0-9a-f]+ <[^>]+> 0eb80be0 	(vcvteq\.f64\.s32|fsitodeq)	d0, s1
0[0-9a-f]+ <[^>]+> 0eb80b60 	(vcvteq\.f64\.u32|fuitodeq)	d0, s1
0[0-9a-f]+ <[^>]+> eebe0aef 	(vcvt\.s32\.f32	s0, s0, #1|ftosls	s0, #1)
0[0-9a-f]+ <[^>]+> eebf0aef 	(vcvt\.u32\.f32	s0, s0, #1|ftouls	s0, #1)
0[0-9a-f]+ <[^>]+> eeba0aef 	(vcvt\.f32\.s32	s0, s0, #1|fsltos	s0, #1)
0[0-9a-f]+ <[^>]+> eebb0aef 	(vcvt\.f32\.u32	s0, s0, #1|fultos	s0, #1)
0[0-9a-f]+ <[^>]+> eebe0bef 	(vcvt\.s32\.f64	d0, d0, #1|ftosld	d0, #1)
0[0-9a-f]+ <[^>]+> eebf0bef 	(vcvt\.u32\.f64	d0, d0, #1|ftould	d0, #1)
0[0-9a-f]+ <[^>]+> eeba0bef 	(vcvt\.f64\.s32	d0, d0, #1|fsltod	d0, #1)
0[0-9a-f]+ <[^>]+> eebb0bef 	(vcvt\.f64\.u32	d0, d0, #1|fultod	d0, #1)
0[0-9a-f]+ <[^>]+> eeba0a67 	(vcvt\.f32\.s16	s0, s0, #1|fshtos	s0, #1)
0[0-9a-f]+ <[^>]+> eebb0a67 	(vcvt\.f32\.u16	s0, s0, #1|fuhtos	s0, #1)
0[0-9a-f]+ <[^>]+> eeba0b67 	(vcvt\.f64\.s16	d0, d0, #1|fshtod	d0, #1)
0[0-9a-f]+ <[^>]+> eebb0b67 	(vcvt\.f64\.u16	d0, d0, #1|fuhtod	d0, #1)
0[0-9a-f]+ <[^>]+> eebe0a67 	(vcvt\.s16\.f32	s0, s0, #1|ftoshs	s0, #1)
0[0-9a-f]+ <[^>]+> eebf0a67 	(vcvt\.u16\.f32	s0, s0, #1|ftouhs	s0, #1)
0[0-9a-f]+ <[^>]+> eebe0b67 	(vcvt\.s16\.f64	d0, d0, #1|ftoshd	d0, #1)
0[0-9a-f]+ <[^>]+> eebf0b67 	(vcvt\.u16\.f64	d0, d0, #1|ftouhd	d0, #1)
0[0-9a-f]+ <[^>]+> 0ebe0aef 	(vcvteq\.s32\.f32	s0, s0, #1|ftoslseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0ebf0aef 	(vcvteq\.u32\.f32	s0, s0, #1|ftoulseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0eba0aef 	(vcvteq\.f32\.s32	s0, s0, #1|fsltoseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0ebb0aef 	(vcvteq\.f32\.u32	s0, s0, #1|fultoseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0ebe0bef 	(vcvteq\.s32\.f64	d0, d0, #1|ftosldeq	d0, #1)
0[0-9a-f]+ <[^>]+> 0ebf0bef 	(vcvteq\.u32\.f64	d0, d0, #1|ftouldeq	d0, #1)
0[0-9a-f]+ <[^>]+> 0eba0bef 	(vcvteq\.f64\.s32	d0, d0, #1|fsltodeq	d0, #1)
0[0-9a-f]+ <[^>]+> 0ebb0bef 	(vcvteq\.f64\.u32	d0, d0, #1|fultodeq	d0, #1)
0[0-9a-f]+ <[^>]+> 0eba0a67 	(vcvteq\.f32\.s16	s0, s0, #1|fshtoseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0ebb0a67 	(vcvteq\.f32\.u16	s0, s0, #1|fuhtoseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0eba0b67 	(vcvteq\.f64\.s16	d0, d0, #1|fshtodeq	d0, #1)
0[0-9a-f]+ <[^>]+> 0ebb0b67 	(vcvteq\.f64\.u16	d0, d0, #1|fuhtodeq	d0, #1)
0[0-9a-f]+ <[^>]+> 0ebe0a67 	(vcvteq\.s16\.f32	s0, s0, #1|ftoshseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0ebf0a67 	(vcvteq\.u16\.f32	s0, s0, #1|ftouhseq	s0, #1)
0[0-9a-f]+ <[^>]+> 0ebe0b67 	(vcvteq\.s16\.f64	d0, d0, #1|ftoshdeq	d0, #1)
0[0-9a-f]+ <[^>]+> 0ebf0b67 	(vcvteq\.u16\.f64	d0, d0, #1|ftouhdeq	d0, #1)
0[0-9a-f]+ <[^>]+> ecd01a04 	(vldmia|fldmias)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> ecd01a04 	(vldmia|fldmias)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> ecf01a04 	(vldmia|fldmias)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> ed701a04 	(vldmdb|fldmdbs)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> ec903b08 	vldmia	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> ec903b08 	vldmia	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> ecb03b08 	vldmia	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> ed303b08 	vldmdb	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> 0cd01a04 	(vldmiaeq|fldmiaseq)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> 0cd01a04 	(vldmiaeq|fldmiaseq)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> 0cf01a04 	(vldmiaeq|fldmiaseq)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> 0d701a04 	(vldmdbeq|fldmdbseq)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> 0c903b08 	vldmiaeq	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> 0c903b08 	vldmiaeq	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> 0cb03b08 	vldmiaeq	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> 0d303b08 	vldmdbeq	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> ecc01a04 	(vstmia|fstmias)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> ecc01a04 	(vstmia|fstmias)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> ece01a04 	(vstmia|fstmias)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> ed601a04 	(vstmdb|fstmdbs)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> ec803b08 	vstmia	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> ec803b08 	vstmia	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> eca03b08 	vstmia	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> ed203b08 	vstmdb	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> 0cc01a04 	(vstmiaeq|fstmiaseq)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> 0cc01a04 	(vstmiaeq|fstmiaseq)	r0, {s3-s6}
0[0-9a-f]+ <[^>]+> 0ce01a04 	(vstmiaeq|fstmiaseq)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> 0d601a04 	(vstmdbeq|fstmdbseq)	r0!, {s3-s6}
0[0-9a-f]+ <[^>]+> 0c803b08 	vstmiaeq	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> 0c803b08 	vstmiaeq	r0, {d3-d6}
0[0-9a-f]+ <[^>]+> 0ca03b08 	vstmiaeq	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> 0d203b08 	vstmdbeq	r0!, {d3-d6}
0[0-9a-f]+ <[^>]+> ed900a01 	(vldr|flds)	s0, \[r0, #4\]
0[0-9a-f]+ <[^>]+> ed900b01 	(vldr|vldr)	d0, \[r0, #4\]
0[0-9a-f]+ <[^>]+> 0d900a01 	(vldreq|fldseq)	s0, \[r0, #4\]
0[0-9a-f]+ <[^>]+> 0d900b01 	vldreq	d0, \[r0, #4\]
0[0-9a-f]+ <[^>]+> ed800a01 	(vstr|fsts)	s0, \[r0, #4\]
0[0-9a-f]+ <[^>]+> ed800b01 	vstr	d0, \[r0, #4\]
0[0-9a-f]+ <[^>]+> 0d800a01 	(vstreq|fstseq)	s0, \[r0, #4\]
0[0-9a-f]+ <[^>]+> 0d800b01 	vstreq	d0, \[r0, #4\]
