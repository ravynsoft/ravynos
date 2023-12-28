#objdump: -dr --prefix-addresses --show-raw-insn
#name: PC-relative LDR from global

.*: +file format .*arm.*

Disassembly of section .text:
0+00 <[^>]*> e59f0010 ?	ldr	r0, \[pc, #16\]	@ 0+18 <[^>]*>
0+04 <[^>]*> e1df00fc ?	ldrsh	r0, \[pc, #12\]	@ 0+18 <[^>]*>
0+08 <[^>]*> ed9f0a02 ?	vldr	s0, \[pc, #8\]	@ 0+18 <[^>]*>
0+0c <[^>]*> 4802      ?	ldr	r0, \[pc, #8\]	@ \(0+18 <[^>]*>\)
0+0e <[^>]*> 4802      ?	ldr	r0, \[pc, #8\]	@ \(0+18 <[^>]*>\)
0+10 <[^>]*> ed9f 0a01 ?	vldr	s0, \[pc, #4\]	@ 0+18 <[^>]*>
0+14 <[^>]*> f8df 0000 ?	ldr\.w	r0, \[pc\]	@ 0+18 <[^>]*>
#...
