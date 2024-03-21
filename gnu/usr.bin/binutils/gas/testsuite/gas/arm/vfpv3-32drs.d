# name: VFPv3 extra D registers
# as: -mfpu=vfp3
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section \.text:
0[0-9a-f]+ <[^>]+> eeb03b66 	(vmov\.f64|fcpyd)	d3, d22
0[0-9a-f]+ <[^>]+> eef06b43 	(vmov\.f64|fcpyd)	d22, d3
0[0-9a-f]+ <[^>]+> eef76acb 	(vcvt\.f64\.f32|fcvtds)	d22, s22
0[0-9a-f]+ <[^>]+> eeb7bbe6 	(vcvt\.f32\.f64|fcvtsd)	s22, d22
0[0-9a-f]+ <[^>]+> ee254b90 	vmov\.32	d21\[1\], r4
0[0-9a-f]+ <[^>]+> ee0b5b90 	vmov\.32	d27\[0\], r5
0[0-9a-f]+ <[^>]+> ee376b90 	vmov\.32	r6, d23\[1\]
0[0-9a-f]+ <[^>]+> ee197b90 	vmov\.32	r7, d25\[0\]
0[0-9a-f]+ <[^>]+> eef86bcb 	(vcvt\.f64\.s32|fsitod)	d22, s22
0[0-9a-f]+ <[^>]+> eef85b6a 	(vcvt\.f64\.u32|fuitod)	d21, s21
0[0-9a-f]+ <[^>]+> eebdab64 	(vcvtr\.s32\.f64|ftosid)	s20, d20
0[0-9a-f]+ <[^>]+> eebdabe4 	(vcvt\.s32\.f64|ftosizd)	s20, d20
0[0-9a-f]+ <[^>]+> eefc9b63 	(vcvtr\.u32\.f64|ftouid)	s19, d19
0[0-9a-f]+ <[^>]+> eefc9be3 	(vcvt\.u32\.f64|ftouizd)	s19, d19
0[0-9a-f]+ <[^>]+> edda3b01 	vldr	d19, \[sl, #4\]
0[0-9a-f]+ <[^>]+> edca5b01 	vstr	d21, \[sl, #4\]
0[0-9a-f]+ <[^>]+> ecba5b04 	vldmia	sl!, {d5-d6}
0[0-9a-f]+ <[^>]+> ecfa2b06 	vldmia	sl!, {d18-d20}
0[0-9a-f]+ <[^>]+> ecba5b05 	fldmiax	sl!, {d5-d6}(	@ Deprecated|)
0[0-9a-f]+ <[^>]+> ecfa2b07 	fldmiax	sl!, {d18-d20}(	@ Deprecated|)
0[0-9a-f]+ <[^>]+> ed7a2b05 	fldmdbx	sl!, {d18-d19}(	@ Deprecated|)
0[0-9a-f]+ <[^>]+> ecc94b0a 	vstmia	r9, {d20-d24}
0[0-9a-f]+ <[^>]+> eeb03bc5 	(vabs\.f64|fabsd)	d3, d5
0[0-9a-f]+ <[^>]+> eeb0cbe2 	(vabs\.f64|fabsd)	d12, d18
0[0-9a-f]+ <[^>]+> eef02be3 	(vabs\.f64|fabsd)	d18, d19
0[0-9a-f]+ <[^>]+> eeb13b45 	(vneg\.f64|fnegd)	d3, d5
0[0-9a-f]+ <[^>]+> eeb1cb62 	(vneg\.f64|fnegd)	d12, d18
0[0-9a-f]+ <[^>]+> eef12b63 	(vneg\.f64|fnegd)	d18, d19
0[0-9a-f]+ <[^>]+> eeb13bc5 	(vsqrt\.f64|fsqrtd)	d3, d5
0[0-9a-f]+ <[^>]+> eeb1cbe2 	(vsqrt\.f64|fsqrtd)	d12, d18
0[0-9a-f]+ <[^>]+> eef12be3 	(vsqrt\.f64|fsqrtd)	d18, d19
0[0-9a-f]+ <[^>]+> ee353b06 	(vadd\.f64|faddd)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee32cb84 	(vadd\.f64|faddd)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee732ba4 	(vadd\.f64|faddd)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee353b46 	(vsub\.f64|fsubd)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee32cbc4 	(vsub\.f64|fsubd)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee732be4 	(vsub\.f64|fsubd)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee253b06 	(vmul\.f64|fmuld)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee22cb84 	(vmul\.f64|fmuld)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee632ba4 	(vmul\.f64|fmuld)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee853b06 	(vdiv\.f64|fdivd)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee82cb84 	(vdiv\.f64|fdivd)	d12, d18, d4
0[0-9a-f]+ <[^>]+> eec32ba4 	(vdiv\.f64|fdivd)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee053b06 	(vmla\.f64|fmacd)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee02cb84 	(vmla\.f64|fmacd)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee432ba4 	(vmla\.f64|fmacd)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee153b06 	(vnmls\.f64|fmscd)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee12cb84 	(vnmls\.f64|fmscd)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee532ba4 	(vnmls\.f64|fmscd)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee253b46 	(vnmul\.f64|fnmuld)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee22cbc4 	(vnmul\.f64|fnmuld)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee632be4 	(vnmul\.f64|fnmuld)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee053b46 	(vmls\.f64|fnmacd)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee02cbc4 	(vmls\.f64|fnmacd)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee432be4 	(vmls\.f64|fnmacd)	d18, d19, d20
0[0-9a-f]+ <[^>]+> ee153b46 	(vnmla\.f64|fnmscd)	d3, d5, d6
0[0-9a-f]+ <[^>]+> ee12cbc4 	(vnmla\.f64|fnmscd)	d12, d18, d4
0[0-9a-f]+ <[^>]+> ee532be4 	(vnmla\.f64|fnmscd)	d18, d19, d20
0[0-9a-f]+ <[^>]+> eeb43b62 	(vcmp\.f64|fcmpd)	d3, d18
0[0-9a-f]+ <[^>]+> eef42b43 	(vcmp\.f64|fcmpd)	d18, d3
0[0-9a-f]+ <[^>]+> eef53b40 	(vcmp\.f64	d19, #0.0|fcmpzd	d19)
0[0-9a-f]+ <[^>]+> eeb43be2 	(vcmpe\.f64|fcmped)	d3, d18
0[0-9a-f]+ <[^>]+> eef42bc3 	(vcmpe\.f64|fcmped)	d18, d3
0[0-9a-f]+ <[^>]+> eef53bc0 	(vcmpe\.f64	d19, #0.0|fcmpezd	d19)
0[0-9a-f]+ <[^>]+> ec443b3f 	vmov	d31, r3, r4
0[0-9a-f]+ <[^>]+> ec565b3e 	vmov	r5, r6, d30
