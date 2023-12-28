#name: Valid armv8-m.main+fp.dp
#as: -march=armv8-m.main+fp.dp
#source: fpv5-d16.s
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-wince-*

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> fe00 0a00 	vseleq.f32	s0, s0, s0
0[0-9a-f]+ <[^>]+> fe50 0aa0 	vselvs.f32	s1, s1, s1
0[0-9a-f]+ <[^>]+> fe2f fa0f 	vselge.f32	s30, s30, s30
0[0-9a-f]+ <[^>]+> fe7f faaf 	vselgt.f32	s31, s31, s31
0[0-9a-f]+ <[^>]+> fe00 0b00 	vseleq.f64	d0, d0, d0
0[0-9a-f]+ <[^>]+> fe18 8b08 	vselvs.f64	d8, d8, d8
0[0-9a-f]+ <[^>]+> fe2f fb0f 	vselge.f64	d15, d15, d15
0[0-9a-f]+ <[^>]+> fe3a ab0a 	vselgt.f64	d10, d10, d10
0[0-9a-f]+ <[^>]+> fe80 0a00 	vmaxnm.f32	s0, s0, s0
0[0-9a-f]+ <[^>]+> fec0 0aa0 	vmaxnm.f32	s1, s1, s1
0[0-9a-f]+ <[^>]+> fe8f fa0f 	vmaxnm.f32	s30, s30, s30
0[0-9a-f]+ <[^>]+> fecf faaf 	vmaxnm.f32	s31, s31, s31
0[0-9a-f]+ <[^>]+> fe80 0b00 	vmaxnm.f64	d0, d0, d0
0[0-9a-f]+ <[^>]+> fe88 8b08 	vmaxnm.f64	d8, d8, d8
0[0-9a-f]+ <[^>]+> fe8f fb0f 	vmaxnm.f64	d15, d15, d15
0[0-9a-f]+ <[^>]+> fe8a ab0a 	vmaxnm.f64	d10, d10, d10
0[0-9a-f]+ <[^>]+> fe80 0a40 	vminnm.f32	s0, s0, s0
0[0-9a-f]+ <[^>]+> fec0 0ae0 	vminnm.f32	s1, s1, s1
0[0-9a-f]+ <[^>]+> fe8f fa4f 	vminnm.f32	s30, s30, s30
0[0-9a-f]+ <[^>]+> fecf faef 	vminnm.f32	s31, s31, s31
0[0-9a-f]+ <[^>]+> fe80 0b40 	vminnm.f64	d0, d0, d0
0[0-9a-f]+ <[^>]+> fe88 8b48 	vminnm.f64	d8, d8, d8
0[0-9a-f]+ <[^>]+> fe8f fb4f 	vminnm.f64	d15, d15, d15
0[0-9a-f]+ <[^>]+> fe8a ab4a 	vminnm.f64	d10, d10, d10
0[0-9a-f]+ <[^>]+> febc 0ac0 	vcvta.s32.f32	s0, s0
0[0-9a-f]+ <[^>]+> fefd 0ae0 	vcvtn.s32.f32	s1, s1
0[0-9a-f]+ <[^>]+> febe fa4f 	vcvtp.u32.f32	s30, s30
0[0-9a-f]+ <[^>]+> feff fa6f 	vcvtm.u32.f32	s31, s31
0[0-9a-f]+ <[^>]+> febc 0bc0 	vcvta.s32.f64	s0, d0
0[0-9a-f]+ <[^>]+> fefd 0bc8 	vcvtn.s32.f64	s1, d8
0[0-9a-f]+ <[^>]+> febe fb4f 	vcvtp.u32.f64	s30, d15
0[0-9a-f]+ <[^>]+> feff fb4a 	vcvtm.u32.f64	s31, d10
0[0-9a-f]+ <[^>]+> eeb6 0ac0 	vrintz.f32	s0, s0
0[0-9a-f]+ <[^>]+> eef7 0a60 	vrintx.f32	s1, s1
0[0-9a-f]+ <[^>]+> eeb6 fa4f 	vrintr.f32	s30, s30
0[0-9a-f]+ <[^>]+> feb8 0a40 	vrinta.f32	s0, s0
0[0-9a-f]+ <[^>]+> fef9 0a60 	vrintn.f32	s1, s1
0[0-9a-f]+ <[^>]+> feba fa4f 	vrintp.f32	s30, s30
0[0-9a-f]+ <[^>]+> fefb fa6f 	vrintm.f32	s31, s31
0[0-9a-f]+ <[^>]+> eeb6 0bc0 	vrintz.f64	d0, d0
0[0-9a-f]+ <[^>]+> eeb7 1b41 	vrintx.f64	d1, d1
0[0-9a-f]+ <[^>]+> eeb6 ab4a 	vrintr.f64	d10, d10
0[0-9a-f]+ <[^>]+> feb8 0b40 	vrinta.f64	d0, d0
0[0-9a-f]+ <[^>]+> feb9 1b41 	vrintn.f64	d1, d1
0[0-9a-f]+ <[^>]+> feba ab4a 	vrintp.f64	d10, d10
0[0-9a-f]+ <[^>]+> febb ab4a 	vrintm.f64	d10, d10
0[0-9a-f]+ <[^>]+> eeb3 0bc0 	vcvtt.f16.f64	s0, d0
0[0-9a-f]+ <[^>]+> eef3 0b48 	vcvtb.f16.f64	s1, d8
0[0-9a-f]+ <[^>]+> eeb3 fbcf 	vcvtt.f16.f64	s30, d15
0[0-9a-f]+ <[^>]+> eef3 fb4a 	vcvtb.f16.f64	s31, d10
0[0-9a-f]+ <[^>]+> eeb2 0bc0 	vcvtt.f64.f16	d0, s0
0[0-9a-f]+ <[^>]+> eeb2 8b60 	vcvtb.f64.f16	d8, s1
0[0-9a-f]+ <[^>]+> eeb2 fbcf 	vcvtt.f64.f16	d15, s30
0[0-9a-f]+ <[^>]+> eeb2 ab6f 	vcvtb.f64.f16	d10, s31
