#name: VCMP immediate without prefix
#as:
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> eeb5 0a40 	vcmp.f32	s0, #0.0
