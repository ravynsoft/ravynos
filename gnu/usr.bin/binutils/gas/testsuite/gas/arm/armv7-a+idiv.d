#name: Valid v7-A+IDIV
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> e730f211 	udiv	r0, r1, r2
0[0-9a-f]+ <[^>]+> e710f211 	sdiv	r0, r1, r2
0[0-9a-f]+ <[^>]+> fbb1 f0f2 	udiv	r0, r1, r2
0[0-9a-f]+ <[^>]+> fb91 f0f2 	sdiv	r0, r1, r2
