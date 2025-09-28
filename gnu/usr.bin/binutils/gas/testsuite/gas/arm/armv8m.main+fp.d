#name: Valid armv8-m.main+fp
#as: -march=armv8-m.main+fp
#source: fpv5-sp-d16.s
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-wince-*

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> fe00 0a00 	vseleq.f32	s0, s0, s0
0[0-9a-f]+ <[^>]+> fe50 0aa0 	vselvs.f32	s1, s1, s1
0[0-9a-f]+ <[^>]+> fe2f fa0f 	vselge.f32	s30, s30, s30
0[0-9a-f]+ <[^>]+> fe7f faaf 	vselgt.f32	s31, s31, s31
0[0-9a-f]+ <[^>]+> fe80 0a00 	vmaxnm.f32	s0, s0, s0
0[0-9a-f]+ <[^>]+> fec0 0aa0 	vmaxnm.f32	s1, s1, s1
0[0-9a-f]+ <[^>]+> fe8f fa0f 	vmaxnm.f32	s30, s30, s30
0[0-9a-f]+ <[^>]+> fecf faaf 	vmaxnm.f32	s31, s31, s31
0[0-9a-f]+ <[^>]+> fe80 0a40 	vminnm.f32	s0, s0, s0
0[0-9a-f]+ <[^>]+> fec0 0ae0 	vminnm.f32	s1, s1, s1
0[0-9a-f]+ <[^>]+> fe8f fa4f 	vminnm.f32	s30, s30, s30
0[0-9a-f]+ <[^>]+> fecf faef 	vminnm.f32	s31, s31, s31
0[0-9a-f]+ <[^>]+> febc 0ac0 	vcvta.s32.f32	s0, s0
0[0-9a-f]+ <[^>]+> fefd 0ae0 	vcvtn.s32.f32	s1, s1
0[0-9a-f]+ <[^>]+> febe fa4f 	vcvtp.u32.f32	s30, s30
0[0-9a-f]+ <[^>]+> feff fa6f 	vcvtm.u32.f32	s31, s31
0[0-9a-f]+ <[^>]+> eeb6 0ac0 	vrintz.f32	s0, s0
0[0-9a-f]+ <[^>]+> eef7 0a60 	vrintx.f32	s1, s1
0[0-9a-f]+ <[^>]+> eeb6 fa4f 	vrintr.f32	s30, s30
0[0-9a-f]+ <[^>]+> feb8 0a40 	vrinta.f32	s0, s0
0[0-9a-f]+ <[^>]+> fef9 0a60 	vrintn.f32	s1, s1
0[0-9a-f]+ <[^>]+> feba fa4f 	vrintp.f32	s30, s30
0[0-9a-f]+ <[^>]+> fefb fa6f 	vrintm.f32	s31, s31
