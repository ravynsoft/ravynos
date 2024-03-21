#name: Valid v8-r+simdv3
#source: armv8-ar+simd.s
#as: -march=armv8-r
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f3000f10 	vmaxnm.f32	d0, d0, d0
0[0-9a-f]+ <[^>]+> f3400fb0 	vmaxnm.f32	d16, d16, d16
0[0-9a-f]+ <[^>]+> f30fff1f 	vmaxnm.f32	d15, d15, d15
0[0-9a-f]+ <[^>]+> f34fffbf 	vmaxnm.f32	d31, d31, d31
0[0-9a-f]+ <[^>]+> f3000f50 	vmaxnm.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f3400ff0 	vmaxnm.f32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f30eef5e 	vmaxnm.f32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f34eeffe 	vmaxnm.f32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f3200f10 	vminnm.f32	d0, d0, d0
0[0-9a-f]+ <[^>]+> f3600fb0 	vminnm.f32	d16, d16, d16
0[0-9a-f]+ <[^>]+> f32fff1f 	vminnm.f32	d15, d15, d15
0[0-9a-f]+ <[^>]+> f36fffbf 	vminnm.f32	d31, d31, d31
0[0-9a-f]+ <[^>]+> f3200f50 	vminnm.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> f3600ff0 	vminnm.f32	q8, q8, q8
0[0-9a-f]+ <[^>]+> f32eef5e 	vminnm.f32	q7, q7, q7
0[0-9a-f]+ <[^>]+> f36eeffe 	vminnm.f32	q15, q15, q15
0[0-9a-f]+ <[^>]+> f3bb0000 	vcvta.s32.f32	d0, d0
0[0-9a-f]+ <[^>]+> f3fb0120 	vcvtn.s32.f32	d16, d16
0[0-9a-f]+ <[^>]+> f3bbf28f 	vcvtp.u32.f32	d15, d15
0[0-9a-f]+ <[^>]+> f3fbf3af 	vcvtm.u32.f32	d31, d31
0[0-9a-f]+ <[^>]+> f3bb0040 	vcvta.s32.f32	q0, q0
0[0-9a-f]+ <[^>]+> f3fb0160 	vcvtn.s32.f32	q8, q8
0[0-9a-f]+ <[^>]+> f3bbe2ce 	vcvtp.u32.f32	q7, q7
0[0-9a-f]+ <[^>]+> f3fbe3ee 	vcvtm.u32.f32	q15, q15
0[0-9a-f]+ <[^>]+> f3ba0500 	vrinta.f32	d0, d0
0[0-9a-f]+ <[^>]+> f3fa0420 	vrintn.f32	d16, d16
0[0-9a-f]+ <[^>]+> f3baf68f 	vrintm.f32	d15, d15
0[0-9a-f]+ <[^>]+> f3faf7af 	vrintp.f32	d31, d31
0[0-9a-f]+ <[^>]+> f3ba04af 	vrintx.f32	d0, d31
0[0-9a-f]+ <[^>]+> f3fa058f 	vrintz.f32	d16, d15
0[0-9a-f]+ <[^>]+> f3ba0540 	vrinta.f32	q0, q0
0[0-9a-f]+ <[^>]+> f3fa0460 	vrintn.f32	q8, q8
0[0-9a-f]+ <[^>]+> f3bae6ce 	vrintm.f32	q7, q7
0[0-9a-f]+ <[^>]+> f3fae7ee 	vrintp.f32	q15, q15
0[0-9a-f]+ <[^>]+> f3ba04ee 	vrintx.f32	q0, q15
0[0-9a-f]+ <[^>]+> f3fa05ce 	vrintz.f32	q8, q7
0[0-9a-f]+ <[^>]+> ff00 0f10 	vmaxnm.f32	d0, d0, d0
0[0-9a-f]+ <[^>]+> ff40 0fb0 	vmaxnm.f32	d16, d16, d16
0[0-9a-f]+ <[^>]+> ff0f ff1f 	vmaxnm.f32	d15, d15, d15
0[0-9a-f]+ <[^>]+> ff4f ffbf 	vmaxnm.f32	d31, d31, d31
0[0-9a-f]+ <[^>]+> ff00 0f50 	vmaxnm.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ff40 0ff0 	vmaxnm.f32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ff0e ef5e 	vmaxnm.f32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ff4e effe 	vmaxnm.f32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ff20 0f10 	vminnm.f32	d0, d0, d0
0[0-9a-f]+ <[^>]+> ff60 0fb0 	vminnm.f32	d16, d16, d16
0[0-9a-f]+ <[^>]+> ff2f ff1f 	vminnm.f32	d15, d15, d15
0[0-9a-f]+ <[^>]+> ff6f ffbf 	vminnm.f32	d31, d31, d31
0[0-9a-f]+ <[^>]+> ff20 0f50 	vminnm.f32	q0, q0, q0
0[0-9a-f]+ <[^>]+> ff60 0ff0 	vminnm.f32	q8, q8, q8
0[0-9a-f]+ <[^>]+> ff2e ef5e 	vminnm.f32	q7, q7, q7
0[0-9a-f]+ <[^>]+> ff6e effe 	vminnm.f32	q15, q15, q15
0[0-9a-f]+ <[^>]+> ffbb 0000 	vcvta.s32.f32	d0, d0
0[0-9a-f]+ <[^>]+> fffb 0120 	vcvtn.s32.f32	d16, d16
0[0-9a-f]+ <[^>]+> ffbb f28f 	vcvtp.u32.f32	d15, d15
0[0-9a-f]+ <[^>]+> fffb f3af 	vcvtm.u32.f32	d31, d31
0[0-9a-f]+ <[^>]+> ffbb 0040 	vcvta.s32.f32	q0, q0
0[0-9a-f]+ <[^>]+> fffb 0160 	vcvtn.s32.f32	q8, q8
0[0-9a-f]+ <[^>]+> ffbb e2ce 	vcvtp.u32.f32	q7, q7
0[0-9a-f]+ <[^>]+> fffb e3ee 	vcvtm.u32.f32	q15, q15
0[0-9a-f]+ <[^>]+> ffba 0500 	vrinta.f32	d0, d0
0[0-9a-f]+ <[^>]+> fffa 0420 	vrintn.f32	d16, d16
0[0-9a-f]+ <[^>]+> ffba f68f 	vrintm.f32	d15, d15
0[0-9a-f]+ <[^>]+> fffa f7af 	vrintp.f32	d31, d31
0[0-9a-f]+ <[^>]+> ffba 04af 	vrintx.f32	d0, d31
0[0-9a-f]+ <[^>]+> fffa 058f 	vrintz.f32	d16, d15
0[0-9a-f]+ <[^>]+> ffba 0540 	vrinta.f32	q0, q0
0[0-9a-f]+ <[^>]+> fffa 0460 	vrintn.f32	q8, q8
0[0-9a-f]+ <[^>]+> ffba e6ce 	vrintm.f32	q7, q7
0[0-9a-f]+ <[^>]+> fffa e7ee 	vrintp.f32	q15, q15
0[0-9a-f]+ <[^>]+> ffba 04ee 	vrintx.f32	q0, q15
0[0-9a-f]+ <[^>]+> fffa 05ce 	vrintz.f32	q8, q7
