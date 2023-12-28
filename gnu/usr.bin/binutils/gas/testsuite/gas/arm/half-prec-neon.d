# objdump: -dr --prefix-addresses --show-raw-insn
#name: Half-precision neon instructions
#as: -mfpu=neon-fp16

.*: +file format .*arm.*

.*
0+0 <[^>]*> f3b60602 	vcvt\.f16\.f32	d0, q1
0+4 <[^>]*> f3b6a706 	vcvt\.f32\.f16	q5, d6
