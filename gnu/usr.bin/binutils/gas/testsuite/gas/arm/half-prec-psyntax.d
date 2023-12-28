# objdump: -dr --prefix-addresses --show-raw-insn
#name: Half-precision instructions (programmer's syntax)
#as: -mfpu=neon-fp16

.*: +file format .*arm.*

.*
0+00 <[^>]*> f3b60602 	vcvt\.f16\.f32	d0, q1
0+04 <[^>]*> f3b6a706 	vcvt\.f32\.f16	q5, d6
0+08 <[^>]*> eeb21ae2 	vcvtt\.f32\.f16	s2, s5
0+0c <[^>]*> eeb21a62 	vcvtb\.f32\.f16	s2, s5
0+10 <[^>]*> eeb31ae2 	vcvtt\.f16\.f32	s2, s5
0+14 <[^>]*> eeb31a62 	vcvtb\.f16\.f32	s2, s5
